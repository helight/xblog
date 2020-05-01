---
title: "如何构建一个控制面来管理 Envoy 管理集群网络流量"
date: 2020-04-04T22:45:20+08:00
tags: ["envoy", "mesh"]
categories: ["sevicemesh", "microservices"]
banner: "img/banners/linux_ebpf_support.png"
author: "helight"
authorlink: "http://helight.info"
summary: ""
keywords: ["kernel","ebpf", "go", "trace"]
draft: true
---

## 指导在服务边缘构建控制面来管理 Envoy Proxy，让它作为服务网关或者在服务网格中使用
Envoy has become a popular networking component as of late. Matt Klein wrote a blog a couple years back talking about Envoy’s dynamic configuration API and how it has been part of the reason the adoption curve for Envoy has been up and to the right. He called the blog the “universal data plane API”. With so many other projects adopting Envoy as a central component to their offering, it would not be a stretch to say “Envoy has become the universal data plane in cloud-native architectures” for application/L7 networking solutions, not just establishing a standardized API.

![](imgs/envoy.png)

Moreover, because of Envoy’s universal data plane API, we’ve seen a multitude of implementations of a management layer to configure and drive Envoy-based infrastructure. We’re going to take a deep dive into what it takes to build a control plane for Envoy so you can use this information to evaluate what type of infrastructure will fit your organization and usecases best. Because this is a broad topic, we’ll tackle it in a multi-part series published over the next coming days. Follow along (@christianposta, @soloio_inc) for the next entries.

There were some great talks at EnvoyCon/KubeCon where some organizations shared their experiences adopting Envoy including how they built their own control planes. Some of the reasons folks chose to build their own control plane:

1. Had existing solutions built on different data planes with pre-existing control planes and needed to retrofit Envoy in
2. Building for infrastructure that doesn’t have any existing opensource or other Envoy control planes (ie, VMs, AWS ECS, etc)
3. Don’t need to use all of Envoy’s features; just a subset
4. Prefer an domain-specific API/object model for Envoy configuration that fits their workflows/worldview better
5. Other control planes weren’t in a mature state when their respective organizations were ready to deploy
![](imgs/control-plane-data-plane.png)

However, just because some early adopters built their own bespoke control planes doesn’t mean YOU should do the same thing now. First, projects building control planes for Envoy have matured quite a bit in the last year and you should explore using those before deciding to re-create yet another control plane. Second, as the folks at Datawire found, and Daniel Bryant recently articulated, building a control plane for Envoy is not for the faint of heart.

I work on a couple open-source projects that have built a control plane for Envoy. For example, Gloo is a function gateway that can act as a very powerful Kubernetes ingress, API Gateway, or function gateway to ease the transition of monoliths to microservices. Gloo has a control-plane for Envoy that we can refer to in this series of posts as an example of how to build a simple abstraction that allows for pluggability and extensibility at the control points you need. Other solid control-plane implementations you can use for reference are Istio and Heptio Contour and we’ll use those as good examples throughout the blog series. If nothing else, you can learn what options exist for an Envoy control plane and use that to guide your implementation if you have to go down that path.
![](imgs/envoyprojects.png)

In this blog series, we’ll take a look at the following areas:

1. Adopting a mechanism to dynamically update Envoy’s routing, service discovery, and other configuration (this part)
2. Identifying what components make up your control plane, including backing stores, service discovery APIs, security components, et. al.
3. Establishing any domain-specific configuration objects and APIs that best fit your usecases and organization
4. Thinking of how best to make your control plane pluggable where you need it
5. Options for deploying your various control-plane components
6. Thinking through a testing harness for your control plane
   
To kick off the series, let’s look at using Envoy’s dynamic configuration APIs to update Envoy at runtime to deal with changes in topology and deployments.

## Dynamically configuring Envoy with its xDS API
One of the main advantages of building on top of Envoy is it’s data plane API. With the data plane API, we can dynamically configure most of Envoy’s important runtime settings. Envoy’s configuration via its xDS APIs is eventually consistent by design – that is – there is no way to affect an “atomic update” to all of the proxies in the cluster. When the control plane has configuration updates, it makes them available to the data plane proxies through the xDS APIs and each proxy will apply these updates independently from each other.

The following are the parts of Envoy’s runtime model we can configure dynamically through xDS:
1. [Listeners Discovery Service API](https://www.envoyproxy.io/docs/envoy/v1.9.0/configuration/listeners/lds#config-listeners-lds) - LDS to publish ports on which to listen for traffic
2. [Endpoints Discovery Service API](https://www.envoyproxy.io/docs/envoy/v1.9.0/api-v2/api/v2/eds.proto#envoy-api-file-envoy-api-v2-eds-proto)- EDS for service discovery,
3. [Routes Discovery Service API](https://www.envoyproxy.io/docs/envoy/v1.9.0/configuration/http_conn_man/rds#config-http-conn-man-rds)- RDS for traffic routing decisions
4. [Clusters Discovery Service](https://www.envoyproxy.io/docs/envoy/v1.9.0/configuration/cluster_manager/cds#config-cluster-manager-cds)- CDS for backend services to which we can route traffic
5. [Secrets Discovery Service](https://www.envoyproxy.io/docs/envoy/v1.9.0/configuration/secret) - SDS for distributing secrets (certificates and keys)
![](imgs/xds-control-plane.png)
The API is defined with proto3 Protocol Buffers and even has a couple reference implementations you can use to bootstrap your own control plane:

1. [go-control-plane](https://github.com/envoyproxy/go-control-plane)
2. [java-control-plane](https://github.com/envoyproxy/java-control-plane)

Although each of these areas (LDS/EDS/RDS/CDS/SDS, together “xDS”) are dynamically configurable, that doesn’t mean you must configure everything dynamically. You can have a combination of parts that are statically defined and some parts that are updated dynamically. For example, to implement a type of service discovery where endpoints are expected to be dynamic but the clusters are well known at deploy time, you can statically define the clusters and use the [Endpoint Discovery Service](https://www.envoyproxy.io/docs/envoy/v1.9.0/api-v2/api/v2/eds.proto#envoy-api-file-envoy-api-v2-eds-proto) from Envoy. If you are not sure exactly which upstream clusters will be used at deploy time you could use the [Cluster Discovery Service](https://www.envoyproxy.io/docs/envoy/v1.9.0/configuration/cluster_manager/cds#config-cluster-manager-cds) to dynamically find those. The point is, you can build a workflow and process that statically configures the parts you need while use dynamic xDS services to discover the pieces you need at runtime. One of the reasons why you see different control-plane implementation is not everyone has a fully dynamic and fungible environment where all of the pieces should be dynamic. Adopt the level of dynamism that’s most appropriate for your system given the existing constraints and available workflows.

In the case of Gloo, we use a control plane based on go-control-plane to implement the xDS APIs to serve Envoy’s dynamic configuration. Istio uses this implementation also as does Heptio Contour. This control plane API leverages [gRPC streaming](https://grpc.io/docs/guides/concepts.html#server-streaming-rpc) calls and stubs out the API so you can fill it with an implementation. Another project, which is unfortunately deprecated but can be used to learn a lot, is Turbine Labs’ Rotor project. This is a highly efficient way to integrate Envoy’s data plane API with the control plane.

gRPC streaming is not the only way to update Envoy’s configuration. In [previous versions of the Envoy xDS API](https://www.envoyproxy.io/docs/envoy/v1.5.0/api-v1/api), polling was the only option to determine whether new configuration was available. Although this was acceptable, and met the criteria for “eventually-consistent” configuration updates, it was less efficient in both network and compute usage. It can also be difficult to properly tune the polling configurations to reduce wasted resources.

Lastly, some Envoy management implementations opt to generate [static Envoy configuration files](https://www.envoyproxy.io/docs/envoy/latest/configuration/overview/v2_overview#static) and periodically replace the configuration files on disk for Envoy and then perform a [hot reload of the Envoy process](https://blog.envoyproxy.io/envoy-hot-restart-1d16b14555b5). In a highly dynamic environment (like Kubernetes, but really any ephemeral-compute based platform) the management of this file generation, delivery, hot-restart, etc can get unwieldy. Envoy was originally operated in an environment that performed updates like this (Lyft, where it was created) but they are incrementally moving toward using the xDS APIs.

## Takeaway
The Gloo team believes using gRPC streaming and the xDS APIs is the ideal way to implement dynamic configuration and control for Envoy. Again, not all of the Envoy configurations should be served dynamically if you don’t need that, however if you’re operating in a highly dynamic environment (e.g., Kubernetes), the option to configure Envoy dynamically is critical. Other environments may not have this need. Either way, gRPC streaming API for the dynamic parts is ideal. Some benefits to this approach:

1. event-driven configuration updates; configuration is pushed to Envoy when it becomes available in the control plane
2. no need to poll for changes
3. no need to hot-reload Envoy
4. no disruption to traffic

## What’s next
In this first part, we established some basic context on how to build a control plane for Envoy by covering the xDS APIs and the different options you have for serving dynamic configuration to Envoy. In the next sections, to be released in a few days, will cover breaking your control plane into deployable components, identifying which pieces you need, what a domain-specific configuration object model could look like, and how to think about pluggability of the control plane. Follow along on twitter (@christianposta, @soloio_inc) or blog (https://blog.christianposta.com https://medium.com/solo-io)

