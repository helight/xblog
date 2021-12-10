---
title: "Do you really need a service mesh?"
date: 2021-11-31T08:45:20+08:00
tags: ["sevicemesh", "microservices"]
categories: ["sevicemesh", "microservices"]
banner: "img/banners/istio2.png"
author: "helight"
authorlink: "http://helight.info"
summary: ""
keywords: ["sevicemesh","microservices"]
draft: true
---

## 前言
本文是一篇翻译的文章，主要是学习，[原文地址在这里](https://www.tigera.io/blog/do-you-really-need-a-service-mesh/).


## 正文
部署和管理微服务所面临的挑战导致了服务网格的出现，服务网格是一种用于在应用层添加可观察性、安全性和流量管理功能的工具和技术。虽然服务网格目的在帮助开发人员和 SRE 处理在 Kubernetes 集群内的服务对服务通信相关的场景，但服务网格还增加了操作复杂性，并引入了一个额外的控制平面，供安全团队管理。

## 什么是服务网格？
服务网格是一个软件基础设施层，用于控制和监控微服务应用内服务到服务流量。

服务网格提供了一些中间件和一些组件，它们支持服务到服务的通信，例如动态发现。它提供了服务发现、跨服务流量负载平衡、加密和身份验证相关的安全功能、可观察性跟踪等功能。服务网格架构利用设计模式实现服务之间的通信，而无需微服务重写应用程序。

## 服务网格架构
服务网格工作的一个关键方式是它利用了 sidecar 设计模式。服务通过代理进行通信和处理请求，代理被动态地注入到每个 pod 中。Envoy 因其性能、可扩展性和 API 功能而成为在服务网格中使用最流行的代理之一。

虽然 sidecar 代理的这种设计模式构成了服务网格的数据平面，但大多数服务网格还引入了一个额外的控制平面来配置和操作数据平面。

### 控制平面 vs 数据平面
数据平面是作为代理实现的，如 Envoy，部署为 sidecar。这意味着每个 pod 中都包含该代理的一个实例，用于调解和控制微服务之间的通信，以及观察、收集和报告服务网格流量遥测数据。所有应用层通信都是通过数据平面进行路由的。

控制平面管理和配置代理，以进行流量路由，收集和整合数据平面遥测数据。虽然 Kubernetes 有自己的控制平面，并且用于调度 pod 和处理部署的自动伸缩，但服务网格引入了另一个控制平面来管理这些代理正在执行的操作，以实现管理服务到服务间的通信。

![](imgs/service-mesh-architecture.png)
服务网格架构

## 服务网格特性

服务网格提供了一组功能强大但复杂的功能。最流行的开源服务网格之一 istio 将这些特性分解为了四大块：**连接、安全、控制和观察特性。**

在连接特性（也称为流量管理）中，如果正在对微服务进行版本控制，并且需要能够处理各种故障场景，那么你会在这个特性中获得一些可能需要的高级功能。

接下来的两个特性，安全和控制。是服务网格提供的通过相互 TLS 身份验证来保护流量的设施，并对在服务间通信的流量实施管理策略。

最后，观测性特性涵盖了可观察性功能，包括 sidecar 代理，它能够收集有关网格内服务间如何相互通信的遥测和日志数据。

![](imgs/service-mesh-pillars.png)

这四个特性代表了一个非常强大的功能集；然而，这种能力是有代价的。服务网格引入了一个额外的控制平面，这会增加部署复杂性和显著的操作开销。

## 服务网格的挑战点
服务网格带来的两个主要挑战是**复杂性和性能**。

就复杂性而言，服务网格比较那安装部署和管理。它需要专业技能，包括大多数用户不需要的功能。

因为服务网格引入了延迟，所以它也会产生性能问题。

与服务网格相关的许多挑战都源于这样一个事实问题：**需要配置的内容太多**（上面提到的大多数功能都需要某种形式的配置）。

虽然有许多服务网格实现，但在满足不同组织的需求时，并没有一站式的解决方案。很可能是安全团队需要花费大量的时间来确定哪个服务网格适用于他们的应用程序。

因此，最终选择使用哪个服务网格就需要了解开发领域知识和专业技能。所以这在除了已经在使用的 Kubernetes 之外，这又增加了一层复杂性。

## 采用服务网格的主要原因
通过与 DevOps 团队、平台和服务所有者的沟通，我们发现有三个主要场景是采用服务网格因素：安全/加密、服务级别可观察性和服务级别控制。

Security/encryption – Security for data in transit within a cluster. Sometimes this is driven by industry-specific regulatory concerns, such as PCI compliance or HIPAA. In other cases, it is driven by internal data security requirements. When the security of internet-facing applications is at the core of an organization’s brand and reputation, security becomes extremely important.
Service-level observability – Visibility into how workloads and services are communicating at the application layer. By design, Kubernetes is a multi-tenant environment. As more workloads and services are deployed, it becomes harder to understand how everything is working together, especially if an organization is embracing a microservices-based architecture. Service teams want to understand what their upstream and downstream dependencies are.
Service-level control – Controlling which services can talk to one another. This includes the ability to implement best practices around a zero-trust model to ensure security.
While these are the main drivers for adoption, the complexity involved in achieving them through use of a service mesh can be a deterrent for many organizations and teams.

## 可操作的简化实现
Platform owners, DevOps teams, and SREs have limited resources, so adopting a service mesh is a significant undertaking due to the resources required for configuration and operation.

Calico enables a single-pane-of-glass unified control to address the three most popular service mesh use cases—security, observability, and control—with an operationally simpler approach, while avoiding the complexities associated with deploying a separate, standalone service mesh. Let’s look at the benefits of this approach, and how you can easily achieve full-stack observability and security, deploy highly performant encryption, and tightly integrate with existing security infrastructure like firewalls.

### 安全
Calico offers encryption for data in transit that leverages the latest in crypto technology, using open-source WireGuard. As a result, Calico’s encryption is highly performant while still allowing visibility into all traffic flows.

One of the areas that introduces considerable operational complexity is the certificate management associated with mTLS that is used in most service meshes. WireGuard provides a highly performant alternative that requires zero configuration—even aspects such as key rotation are built into the protocol.

### 可观测性
Calico offers visibility into service-to-service communication in a way that is resource efficient and cost effective. It provides Kubernetes-native visualizations of all the data it collects, in the form of a Dynamic Service Graph. The graph allows the user to visualize communication flows across services and team spaces, to facilitate troubleshooting. This is beneficial to platform operators, service owners, and development teams.

With Calico, Envoy is integrated into the data plane, providing operational simplicity. No matter which data plane you’re using (standard Linux iptables, Windows, or eBPF), Calico provides observability, traffic flow management, and control by deploying a single instance of Envoy as a daemon set on each node of your cluster. Instead of having an Envoy container in every pod as a sidecar, there is one Envoy container per node. This provides performance advantages, as it’s more resource efficient and cost effective.

This visibility can easily be extended to workloads outside the cluster as well.

### 实现控制
Over the years, Calico has become well-known for its capabilities around implementing controls. Now, we’ve extended these capabilities to the full stack, from the network layer up through the application layer. You get the application-layer controls you would get with a service mesh, but are able to combine those with controls you might want to implement at the network or transport layer. You can implement policies you’ve defined by assigning them to tiers that enforce an order of precedent, and each of those tiers can be tied to role-based access control (RBAC).

For example, if you want to have a security team manage certain tiers that might be geared toward PCI compliance, or monitor egress traffic and compare it against known threat feeds, you can easily do so. What’s more, you can put those policies in place without interfering with what you might want to enable your application and development teams to do for east-west traffic and service-to-service communication, which service mesh typically handles.

Calico also offers some powerful capabilities related to egress access controls. The capabilities make it easy to integrate with firewalls or other kinds of controls where you might want to understand the origin of egress traffic, and implement certain controls around that. If you’re working with a SIEM or other log management system or monitoring tool, it’s really helpful to be able to identify the origin of egress traffic, to the point where you have visibility into the specific application or namespace from which egress traffic seen outside the cluster came.

Kubernetes itself presents a lot of challenges, and working through getting Kubernetes up and running keeps security teams quite busy. Calico is able address some of the most popular service mesh use cases, without introducing an additional control plane. This makes it operationally a lot simpler to handle, especially if your team has limited resources.

## 总结
So… Do you really need a service mesh? In my opinion, if security and observability are your primary drivers, the answer is no. Calico provides granular observability and security—not just at the application layer, but across the full stack—while avoiding the operational complexities and overhead often associated with deploying a service mesh.

<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)
</center>