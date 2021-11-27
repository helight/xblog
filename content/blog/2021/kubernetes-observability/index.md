---
title: "云原生架构中 Kubernetes 可观测性的挑战"
date: 2021-11-31T08:45:20+08:00
tags: ["Kubernetes", "ebpf"]
categories: ["k8s", "microservices"]
banner: "img/banners/kubernetes.jpeg"
author: "helight"
authorlink: "http://helight.info"
summary: ""
keywords: ["k8s","microservices"]
draft: true
---

## 前言
本文是一篇翻译的文章，主要是学习，[原文地址在这里](https://developers.redhat.com/blog/2020/05/11/top-10-must-know-kubernetes-design-patterns).

这篇文章是对 redhat 写的一本书的概述，提炼了很多核心概念。

这篇文章中介绍了和传统《设计模式》类似的云原生时代的设计模式，《设计模式》在软件开发中意义重大，现在多少软件研发都受到它的影响，而且我之前也在公司内开了这门课程，自己学习的同时，也是想让我们的开发者开发软件更有软件设计思维。

而今这本书的出现是针对目前云原生时代的设计模式，之前的设计模式更多的是对单个模块或是简单系统的，但是云原生时代的开发方式和理念与之前的主机开发模式还是有大的不同的。所以现在开发者在学习了《设计模式》的基础上，还应该学习这些**云上设计模式**。

## 正文
Kubernetes 已经是对工作负载和微服务进行容器编排的实时标准平台，目前 Kubernetes 已经在云原生应用广泛落地。 Kubernetes 的工作负载是高度动态变化的，并且部署在分布式和敏捷的基础架构上的。虽然 Kubernetes 管理云原生应用的好处非常多了，但是 Kubernetes 在云本地应用程序中遇到了一些新的可观测性问题挑战。这篇文章就是想讨论一下目前遇到的这些挑战。

## 可观测性挑战
先看看我们会遇到哪些挑战：
1. **数据孤立仓库** - 传统的监控工具专门收集应用程序和基础架构级别的指标。考虑到云本机应用程序的高度动态、分布式和短暂性，这种度量收集方式会在仓库中创建数据，这些数据需要在服务上下文中缝合在一起，以便使 DevOps 和 SRE 能够调试服务问题（例如，响应时间慢、停机等）。此外，如果 DevOps 工程师或者服务所有者添加了新的观测指标，数据孤立仓库可能会导致交叉引用中断和数据误解，从而导致数据错位、通信速度减慢和分析错误。
2. **数据卷和细粒度组件** - Kubernetes 的部署有多个组件构成，比如 Pod，容器和微服务，他们都运行在分布式的基础设施上。由每个层面产生的海量的各种粒度的数据，如告警，日志和跟踪信息，这些数据随着服务扩展而增加。数据越多就越难梳理出模式和调试问题。随着数据增长，观察和故障排除会变得更加困难。
3. **Kubernetes 抽象** - Kubernetes 抽象让大家难以理解在动态，短时和分布式的基础设施底层发生的事情。要解开这些抽象的概念就像一层层剥开洋葱（在容器，node节点，网络，进程级别，有时甚至要深入到套接字和网络数据包级别），通过逐层分析才能理解这些底层的问题。
 
鉴于Kubernetes微服务部署的复杂性和生成的大量数据，在 Kubernetes 中定位处理一个应用程序的问题是比较困难的。需要一种不同的方法来解决 Kubernetes 可观测性挑战。

## Kubernetes 可观测性的另一种解决方式
Kubernetes 的声明性方式使得正确地进行可观察性非常简单。DevOps，SRE 或者服务拥有者可以声明一种高级语言来构建他们想要的安全和观察系统，并且 Kubernetes 能负责这些构建的实现。可观测性可以被当作一种代码，以便它作为应用程序的一个组成部分连接到应用程序中，然后随应用程序一起部署，以便它可以在任何云、基础设施、网络或应用程序上运行。可观察性可以进一步改进，因为代码可以集成到 CI/CD 链中。这允许开发人员和软件工程师以适当的可观察性级别构建他们的应用程序，以确保应用程序按预期工作。

为了进一步理解这一点，让我们看一个简单的示例，它展示了在 Kubernetes 环境中可观测性作为代码是怎么在的云原生应用程序中工作的。

在线精品店（以前称为 Hipster Shop）是来自谷歌云的 11 层微服务演示应用程序。

![](imgs/Online-Boutique-microservices-architecture.png)
在线精品店微服务架构

其中一个微服务是 ProductCatalogService，其目的是显示最新的分类（分类会随着可买新产品和现有产品库存的变化而变化）。在线精品店应用部署后，我们将监控相关的微服务（包括 ProductCatalogService）的故障、超时和性能缓慢。

使用传统的监控和故障排除方法，开发人员、DevOps 和 SRE 在各自的数据仓库中工作，分别为 ProductCatalogService 捕获数据，然后通过构建内部脚本或利用第三方软件将各自的数据相互关联。开始的时候，在数据量较低的情况下，这是可能的，但这种方法不会随着业务的增长而扩展（因为有更大的产品分类和/或业务逻辑来让更多的客户访问购买）。由于应用程序使用Kubernetes 作为分布式、动态和短时作业的基础结构来运行 ProductCatalogService，Kubernetes 的抽象将使节点、容器、网络、进程和网络数据包级别的数据上下文分析变得困难。所有这些挑战加在一起将导致更长的故障排除时间，并在应用程序崩溃或性能降低时让大家只能瞎猜。

DevOps 或 SRE 可以采用另一种方法，将可观察性作为代码，利用 Kubernetes 原生的抽象及其声明性模型来正确观察 ProductCatalogService。

```yaml
apiVersion: projectcalico.org/v3
kind: PacketCapture
metadata:
name: productcatalogservice-pcap
namespace: onlineboutique
    spec:
        selector: app == "productcatalogservice"
        destination:
            ports:
                - 80
            protocol: TCP
```
The above is an example of observability as code. A job “packetcapture-pcap” is created in the namespace “onlineboutique” that selects and filters the workloads running on Kubernetes labeled as “app == productcatalogservice,” and does the packet capture at port 80 and protocol TCP. Data silo and data granularity problems are eliminated as Packetcapture is part of ProductCatalogService in Online Boutique and is collecting all relevant metadata. This approach has the following benefits:

1. As the application and associated microservices are deployed by Kubernetes on ephemeral and distributed infrastructure, the label in code and the job ensure that all relevant context remains at the fingertips of all team members. This allows for easy filtering and subsequent analysis of traffic payload.
2. This example can be used by different team members for other services or packet capture at a different port, or for a different protocol.
3. In the same way that development teams maintain unit tests to ensure the quality of their code at build time, observability as code (like the one in this example) can be maintained to ensure that various stakeholders (DevOps, SREs, etc.) can easily troubleshoot an application at runtime.

As shown with this example, the observability challenges posed by data silos, data volume and granular components, and Kubernetes abstraction can be addressed by using an observability as code approach, which utilizes Kubernetes’ declarative nature. This leads to faster troubleshooting and shorter time to resolution if your application is experiencing performance, breakdown, or timeout issues.

<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)
</center>