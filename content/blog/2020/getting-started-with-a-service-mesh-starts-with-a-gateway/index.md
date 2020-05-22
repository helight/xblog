---
title: "从使用微服务网关为起点入门ServiceMesh"
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

本文是在看了国外 Solo 公司 CTO 的博客之后整理的，本来也是想按原文翻译，但是考虑到我自己在公司实践的思路，还是想把他的思路和我自己的思路做一些结合。所以本文中有部分内容是来自这位高手的思考，也有有我在公司实践中的思考。

作者是从 Red Hat 跳到 Solo 公司的，这家公司现在主要产品就是基于 Envoy 和 Istio 的网络治理工具的研发，包括了微服务网关和服务网格上的产品。

他也是很早之前就在思考服务网格虽然很好（至少是思路和理念非常好），而且现在结合 K8S 也有了很好的实现和落地方式。但是人们对这个东西的认知和信心还是不足的，我看到的主要表现在这样一些方面：
1. Istio 等相关服务网格的产品还不够成熟，目前我重点关注 Istio 社区，也是社区 Member。目前看来 Istio 的代码都还在大幅度的变化，很多设计在不同的大版本中是不断调整的，这一点就不多说了。刚好这几天也发布了 1.6，不过我感觉 1.6 应该会好很多。
2. 大家直观的感觉是加了 sidecar 之后，网络调用多了一跳，性能会变差，甚至变得很差。
3. 大家对把服务网格作为基础设施这种想法还是比较抗拒，认为妨碍了他定位问题和提升应用性能。
4. 大家对于微服务框架开发的热情超过服务网格，很多人认为服务网格是一直中微服务架构实现，所以自然的把微服务框架作为了服务网格的竞争对手。对于开发同学来说开发微服务框架的成就感要不直接使用服务网格这样的基础设施强很多。

以上这几个点就不展开讲了，大家应该能理解我的意思。这也是我这几年研究和落地服务网格中的一些难点，我认为这些难点随着时间的发展将不会成为问题，尤其我认为服务网格未来一定会作为操作系统一样的基础设施为大家提供服务。

但是在目前这个阶段我们应该怎么办呢？技术不成熟，大家不太接受。我将奈何？

Solo CTO 的文章让我更坚定了我们目前的思路和做法。下面几点是他总结的，如何在公司中逐步落地服务网格的方法。

这一次，我开发了这种方法，以便在生产环境中很好的采用服务网格：
1. 深入了解最终的服务网格数据面技术。
2. 采用小部分流量使用数据面理想的情况下，首先共享服务网关。
3. 选择一部分应用，启用边车模式的网络。
4. 慢慢启用服务网格中最有价值的功能。
5. 重复第 2 步 到 第 4 步。

目前我们基本也是这个思路，重点在使用 Envoy 作为微服务网关，开发集成了公司内部的相关基础组件和业务组件，作为我们微服务平台的核心组件。并且在使用中也是按照多集群部署服务网关，然后在一些可控业务上尝试启用服务网格能力。这个能力我们做成了 web 页面一个按钮就可以启用，在实现上其实就是注入一段代码在业务 pod 中自动注入 istio-proxy。再配合页面化的配置管理来控制策略实施，或通过页面进行服务状态信息可视化等。

## 再来介绍一下什么是 Envoy Proxy
[Envoy](https://www.envoyproxy.io/) 已经变成大多数服务网格技术中基础的数据面了。像 Istio，Consul Connect，AWS App Mesh，Grey Matter（和其它已经存在的 API 管理提供商也在逐步采用）都是基于 Envoy 的。

也许大家认为作为一个协议代理或者服务网关在技术实现上没有什么难度，但是作为通用的服务代理组件，在实际实现中却是绝对没有大家想的那样简单。要有可以用户自己来扩展的机制，遥测信息收集和监控实现，要可以有静态和动态的配置的方式，对可配置内容的设计抽象，兼顾性能问题等等。如果可以建议大家在实际环境中来真正安装部署运行测试体验一下。

总体来说 Envoy 目前是一个非常功能强大，支持多种使用方式和实现复杂的技术组件。在服务网关，服务网格技术中目前是最佳的选择。

## 从基于 Envoy 构建一个服务网关开始
Using Envoy as a shared gateway is a great place to start when adopting a service mesh. In my book Istio in Action I introduce the Istio [Gateway resource](https://istio.io/docs/tasks/traffic-management/ingress/ingress-control/) and its associated configuration near the beginning of the book, because this is the best way to get started with Istio. The Gateway is built on Envoy and can front your microservices without being forced to build the full mesh (ie, inject sidecars next to all of your applications.

![](imgs/1.png)

Using a gateway to front your applications means you can get both operational experience running Envoy as well as get a “service-mesh lite” experience. When the gateway is in place, you can get some powerful traffic routing control (including percentage based routing, header/method based routing, as well as shadow traffic, etc), TLS termination/passthrough, TCP control, etc.

A simple gateway like the Istio Gateway may be a good way provide basic traffic ingress to your cluster when starting out, but a more-full featured API Gateway built on Envoy might provide more benefits.

## 基于 Envoy 构建更好的服务网关
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

## 总结
If you’re on your journey to a service mesh, keep in mind this simple, tried and true approach to adoption. Envoy is the de-facto service mesh data plane (except for Linkerd — at least at this point) and building your strategy around Envoy is an important first step. If you’re exploring API management or gateway L7 networking technology not built on Envoy, you may wish to have a second look especially if you’re looking for an easy on-ramp to a service mesh.