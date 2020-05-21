---
title: "如何 60 秒内进行 Linux 性能分析"
date: 2020-05-11T08:45:20+08:00
tags: ["linux", "Performance"]
categories: ["linux", "kernel"]
banner: "img/banners/linux_performance_ovservability.png"
author: "helight"
authorlink: "http://helight.info"
summary: "在你登陆一台 Linux 服务器之后，因为一个问题要做性能分析时：你会在第一分钟内做哪些检测呢？"
keywords: ["linux","Performance", "top"]
draft: true
---
I’ve been helping to get the word out on service mesh and Envoy Proxy for over two years now. It’s been amazing to see how the communities have grown and more importantly how organizations have begun using it to solve difficult production and operational problems. With both my time at Red Hat, and now Solo.io, I’ve been lucky to work closely with organizations on their service-mesh adoption journey.

In this time, I’ve developed this approach to successfully adopt a service-mesh in production:

1. Become deeply familiar with the data-plane technology of the eventual service mesh
2. Operationalize the data plane with a smaller portion of traffic ideally using a shared gateway at first
3. Choose a subset of applications to enable the mesh (sidecar-based) networking
4. Slowly enable the features of the service mesh that deliver the most value
5. Rinse and repeat steps 2–4

A big reason why I left Red Hat to join Solo.io is our visions about service mesh, including this simple adoption list, aligned very well. The rest of the blog is how we currently help customers adopt service mesh using a gateway and data-plane-first approach.

## Understanding Envoy Proxy
[Envoy](https://www.envoyproxy.io/) has become the foundational data plane for a lot of service-mesh technology out there. Meshes like Istio, Consul Connect, AWS App Mesh, Grey Matter (and others from existing API Management vendors are likely on the way) are all based on Envoy.

[Envoy](https://www.envoyproxy.io/) is an extremely powerful, versatile, and complicated piece of technology. One thing that sticks with me from working on messaging infrastructure in the past is that passing messages (or bytes) from one pipe to another seems easy on the surface, but in practice it’s MUCH harder than you think. It’s incredibly important to understand how Envoy works including its various filters, telemetry it collects, and how it’s configuration APIs work. This understanding is best acquired through experience with operating Envoy in YOUR environment.

Ideally, you start with a single Envoy deployment (logically single) to front your applications.

## Using a gateway built on Envoy as a stepping stone
Using Envoy as a shared gateway is a great place to start when adopting a service mesh. In my book Istio in Action I introduce the Istio [Gateway resource](https://istio.io/docs/tasks/traffic-management/ingress/ingress-control/) and its associated configuration near the beginning of the book, because this is the best way to get started with Istio. The Gateway is built on Envoy and can front your microservices without being forced to build the full mesh (ie, inject sidecars next to all of your applications.

![](imgs/1.png)

Using a gateway to front your applications means you can get both operational experience running Envoy as well as get a “service-mesh lite” experience. When the gateway is in place, you can get some powerful traffic routing control (including percentage based routing, header/method based routing, as well as shadow traffic, etc), TLS termination/passthrough, TCP control, etc.

A simple gateway like the Istio Gateway may be a good way provide basic traffic ingress to your cluster when starting out, but a more-full featured API Gateway built on Envoy might provide more benefits.

## A better gateway built on Envoy
The reality is, when connecting clients outside of the cluster/future service mesh to those services running within the cluster/service mesh, there’s a harsh reality that must be taken into account: existing organizations already have assumptions about how traffic flows and should be secured.
For example, when bringing traffic into a cluster or new service mesh via a gateway, we will need to solve for things like:
1. caching
2. spike arrest/rate limiting
3. end-user/client oauth flows
4. HMAC/message signatures
5. jwt validation (including integrating with existing JWT issuers or identity management)
6. web-application firewalling (WAF)
7. message transformation
8. API orchestration

And many others. In other words, this ingress point needs to be more powerful and capable than the a basic Envoy gateway (ie, Istio’s Gateway). It needs to handle familiar edge functionalities typically found in API Gateways.

![](imgs/2.png)

Gloo from Solo.io is an Envoy Proxy based API gateway that provides the best of both worlds:
1. A stepping stone to a service mesh by simplifying the experience of adopting a single front-gateway as well as
2. The ability to handle familiar API Gateway functionality.

Gloo allows you to combine the features of an API gateway with that of a service mesh. Gloo integrates cleanly with all service-mesh implementations like Istio, Consul, AWS App Mesh, and Linkerd. We’ve had a lot of customer success taking this simple, iterative approach, while coaching teams to operationalize Envoy.

![](imgs/3.png)

Gloo is different from other API Gateways built on Envoy because it’s built by a team with vast experience operationalizing Envoy, a scalable and flexible control plane, with a security-first mindset, as well as a Kubernetes-native and non-Kubernetes deployments.

## Wrapping up
If you’re on your journey to a service mesh, keep in mind this simple, tried and true approach to adoption. Envoy is the de-facto service mesh data plane (except for Linkerd — at least at this point) and building your strategy around Envoy is an important first step. If you’re exploring API management or gateway L7 networking technology not built on Envoy, you may wish to have a second look especially if you’re looking for an easy on-ramp to a service mesh.