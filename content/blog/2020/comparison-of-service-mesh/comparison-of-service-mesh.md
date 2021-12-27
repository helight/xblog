+++
title = "【译】Kubernetes 服务网格: Istio, Linkerd 和 Consul 大比较"
date = "2020-02-02T13:47:08+02:00"
tags = ["istio", "k8s"]
categories = ["sevicemesh", "microservices"]
banner = "img/banners/istio.png"
draft = false
author = "helight"
authorlink = "http://helight.cn"
keywords = ["istio","linkerd", "consul"]
+++

云原生应用通常是由一组运行在容器中的分布式微服务架构起来的。目前越来越多的容器应用都是基于 Kubernetes 的，Kubernetes 已经成为了容器编排的事实标准。
<!--more-->
大多数采用了微服务架构的公司对微服务架构的影响是没有完全理解的，其中之一就是为服务的延展。就像城市周围的郊区一样，部署的小服务数量在呈现几何增长。在我看来这也是微服务带来的灾难性问题之一。
![](imgs/consul-linkerd-istio.png)
微服务的大量增加对如何统一标准化管理服务带来了非常大的挑战，比如多个服务/版本之间的路由、验证和授权、加密，以及在Kubernetes集群内的负载均衡等。

服务网格就是来帮助解决这些问题的，甚至可以有更多功能。就像容器把应用程序从操作系统上抽象出来，服务网格的目标就是把如何处理进程间通信再抽象出来。

## What is Service Mesh 什么是服务网格

虽然服务网格技术出现在Kubernetes之前，但是激增的构建在Kubernetes之上的微服务促使了人们对服务网格解决方案的越来越有兴趣。

理解微服务最关键的一点就是要理解微服务都是严重依赖网络的。

服务网格管理服务间的网络流量。其它管理方式需要大量人工、容易出错的工作，并且在长远支持来说这种工作是一种难以忍受的负担，而服务网格提供了一种非常优雅和可扩展的方式来管理。

一般情况下，服务网格层是在Kubernetes设施之上的，让服务间的网络通信安全可靠。

可以把服务网格想成一个路由和追踪服务，用来处理邮件中要运送的包裹：保存路由规则路线，并且动态的指挥流量和包裹的路径，以此来加速交付和保证接受。

服务网格可以通过可观测性，网络和安全策略来分隔应用程序的业务逻辑。通过服务网格可以链接微服务，让微服务更安全，并且可以更好的监控微服务。

1. 连接：通过服务网格服务可以发现其它服务，并且可以相互通信。它可以智能的路由控制使服务间的流量和API调用。同时也可以支持高级的不部署方式，比如蓝绿发布，金丝雀或者滚动升级，甚至更多。
2. 安全：服务网格可以让服务间进行安全通信。可以执行安全策略来允许或者拒绝通信，比如，可以配置一个策略来拒绝来自部署环境中的客户服务访问生产服务。
3. 监控：服务网格可以让分布式的微服务系统具有可观测性。服务网格通常集成开箱即用的监控和追踪工具（比如 Kubernetes 中的 Prometheus 和 Jaeger），这些工具可以发现并可视化服务、流量、API延迟和跟踪之间的依赖关系。

针对由分布式微服务组成的复杂云原生应用程序，这些关键功能为它的整个网络行为提供了操作控制和可观察性。

当你处理大型网站或者超大规模微服务系统的时候服务网格就很关键了（比如Netflix，Amazon）。但是如我们下面看到的，在你的系统还在发展的时候，现在已经有很多像框架一样的技术来支持未来的大规模应用了。


## Service Mesh Options for Kubernetes:Kubernetes 下的服务网格选择

在 Kubernetes 生态系统中有 3 个领先的服务网格竞争者。这 3 个解决方案都是开源的。每个解决方案都有它的优势和不足，对于一个要开发和管理越来越多微服务的 DevOps 团队来说，采用任何一个都可以让自己的团队有更好的发展。

### Consul Connect
![](imgs/consul.png)

Consul 是一个全功能的服务管理框架，在1.2版本中的连接能力加强让它有了服务发现能力，这也让它成为了一个完整的服务网格方案。Consul 是 HashiCorp 公司技术设施管理产品套件的一部分；它开始是为了管理运行在 Nomad 上的服务，后台发展到支持多数据中心和包括 Kubernetes 的容器管理平台。

Consul Connect 以 DaemonSet 的方式在每个节点上安装 agent，它来和 Envoy sidecar 通信，让 sidecar处理路由并且转发流量。

架构图和更多产品信息请看 [Consul.io](https://www.consul.io/docs/internals/architecture.html)。

### Istio
![](imgs/istio.png)

Istio 是一个 Kubernetes 原生的解决方案，由 Lyft 公司创建，有许多主流科技公司已经选择使用 Istio 作为他们的服务网格技术方案。Google， IBM，和 Microsoft 更是把 Istio 作为其默认的服务网格解决方案，并且已经在他们的 Kubernetes 云服务上提供了相应的服务。针对混合环境下的 Istio 完整管理服务将会在 Platform9 上很快可用了，也是基于 Kubernetes 管理的。

Istio 是第一个有开发人员非常想要的附加功能的，比如深度分析功能。

Istio 分离了数据平面和控制平面，数据平面使用 sidecar 作为代理，数据平面从控制平面这里加载规则信息并缓存起来，这样数据平面就不需要对每个请求都去访问控制平面了。控制平面也是以多个 pod 的运行在 Kubernetes 集群内，这样让服务网格任何部分出现单个 pod 故障的时候都有较好的弹性。

允许在服务网格的任何部分出现单个pod故障时具有更好的弹性

架构图和更多产品信息请看 [Istio.io](https://istio.io/docs/concepts/what-is-istio/#architecture)。

### Linkerd
![](imgs/linkerd.png)

Linkerd 可以说说说 Kubernetes 上第二个最流行的服务网格技术，在它重写的 V2 版本中，它的架构已经和 Istio 的非常相似了，它最初聚焦在简单性而不是灵活性。事实上，它是目前只支持 Kubernetes 的一个方案，导致组件较少，这也意味着 Linkerd 总体并不复杂。Linkerd v1.x 还在支持中，也开始支持了除 Kubernetes 外的其它容器平台。在 v2 版本中主要会考虑新特性（比如蓝绿发布）。

Linkerd 是唯一一个在 [CNCF](https://www.cncf.io/) 上的服务网格解决方案，它也是针对 Kubernetes 的。其它服务网格技术都没有独立的基金会支持。

架构图和更多产品信息请看 [Linkerd.io](https://linkerd.io/2/reference/architecture/)。

## 比较 Kubernetes 上的服务网格技术：Istio， Linkerd 和 Consul Connect

1. **Supported Workloads**：它是否支持 VM 和 Kubernetes？
2. **Architecture**：解决方案的架构是否能解决操作的开销，其实也就是是否高可用，系统运维复杂性。
3. **Secure Communication**：所有服务支持双边 TLS（mTLS）加密，还要支持本地证书管理，这样方便更新证书或者注销证书。
4. **Communication Protocols**：通信协议支持情况
5. **Traffic Management**：流量管理策略
6. **Chaos Monkey-style Testing**：流量管理功能可以引入调用延时或者部分请求失败特性，以此来提升系统的可用性和增强系统运维。
7. **Observability**：为了识别和解决问题，就需要分布式的监控和追踪能力。
8. **Multicluster Support**：多集群支持
9. **Installation**：安装方式
10. **Operations Complexity**：安装，配置和运维的难易程度。	

| **Supported Workloads** | Istio | Linkerd v2 | Consul |
| -- | -- | -- | -- |
| Workloads | Kubernetes + VMs | Kubernetes only | Kubernetes + VMs|
| -- | -- | -- | -- |
| **Architecture 架构**| Istio | Linkerd v2 | Consul |
| Single point of failure 单点故障 | No – 每个 pod 上都有 sidecar | No | No. 但增加了管理HA的复杂性，<br>因为必须按指定数量安装Consul服务，<br>而非用本地K8S master |
| Sidecar Proxy | Yes (Envoy) | Yes | Yes (Envoy) |
| Per-node agent | No | No | Yes |
| -- | -- | -- | -- |
| **Secure Communication 安全通信**| Istio | Linkerd v2 | Consul |
| mTLS | Yes | Yes | Yes |
| Certificate Management | Yes | Yes | Yes |
| Authentication and Authorization | Yes | Yes | Yes |
| -- | -- | -- | -- |
| **Communication Protocols 通信协议** | Istio | Linkerd v2 | Consul |
| TCP | Yes | Yes | Yes |
| HTTP/1.x | Yes | Yes | Yes |
| HTTP/2 | Yes | Yes | Yes |
| gRPC | Yes | Yes | Yes |
| -- | -- | -- | -- |
| **Traffic Management 流量管控** | Istio | Linkerd v2 | Consul |
| Blue/Green Deployments 蓝绿发布| Yes | Yes | Yes |
| Circuit Breaking 熔断| Yes | No | Yes |
| Fault Injection 故障注入| Yes | Yes | Yes |
| Rate Limiting 限频| Yes | No | Yes |
| -- | -- | -- | -- |
| **Chaos Monkey-style Testing: 混沌测试**| Istio | Linkerd v2 | Consul |
| Testing | Yes- 可以配置服务延时响应或者按请求百分比返回失败 | Limited | No |
| -- | -- | -- | -- |
| **Observability 可观测性**| Istio | Linkerd v2 | Consul |
| Monitoring 监控| Yes, with Prometheus | Yes, with Prometheus | Yes, with Prometheus |
| Distributed Tracing 分布式追踪| Yes | Some | Yes |
| -- | -- | -- | -- |
| **Multicluster Support 多集群支持** | Istio | Linkerd v2 | Consul |
| Multicluster | Yes | No | Yes |
| -- | -- | -- | -- |
| **Installation 安装支持** | Istio | Linkerd v2 | Consul |
| Deployment | 通过 Helm 和 istioctl 安装 | Helm | Helm |
| -- | -- | -- | -- |
| **Operations Complexity 操作复杂度**| Istio | Linkerd v2 | Consul |
| Complexity | High | Low | Medium |


以上任何一种服务网格技术都可以解决你遇到的基础需求。你要根据你需要的除了基础功能外的附加功能来进行选择。

就目前为止 Istio 是 3 个技术方案中拥有最多的特性和灵活性的一个，但是要记住灵活性就意味着复杂性，所以你的团队要明白这一点，要为此做好准备。

如果只是支持 Kubernetes，那么 Linkerd 或许是最好的选择。如果你想支持多种环境（包括了 Kubernetes 和 VM 环境）但是又不需要 Istio 的复杂性，那么 Consul 可能是最好的选择。

### 在不同服务网格技术间迁移

注意，服务网格技术并不是有侵入性的转变，不像单体应用到微服务或者从 VM 到基于 Kubernetes 应用一样，它们是有侵入性的。因为多数服务网格技术采用的是 sidecar 模式，所以多数服务都不知道它们是以服务网格的方式运行。但是从一个服务网格技术转换到另外一种是很复杂的。尤其是你想把服务网格技术作为你伸缩所有服务的标准方式的时候，这种情况下难度就很大了。

所以明智的挑选服务网格技术非常重要！可以从简单的项目开始测试，以此来看那种解决方案更适合你。

Istio 正迅速的成为 Kubernetes 上的服务网格技术标准。它是最成熟，但是部署最复杂的。

## 目前使用服务网格技术常用案例
从运维的视角来看，服务网格对于管理任何形式的微服务架构系统都是有用的，它可以帮助你对微服务进行管控流量，安全，权限管理和服务观测。

现在一旦你有了 Kubernetes 技术设施 + 微服务架构的体系，就可以参考下面的例子，来看看如何利用服务网格技术，先不管应用的伸缩性哈。

通过熟悉这些知识，就可以开始在系统设计中对服务网格进行标准化，以便为将来的大规模操作放置构建块和关键组件。

1. 在分布式服务中改进可观测性：让你有了服务级别的可视化，追踪和监控能力。服务网格的一些关键能力极大地提高了可视化，同时提高了解决和减少问题的能力。例如，如果系统中的一个服务成为了瓶颈，一般的做法就是重试，但是这会因为瓶颈服务超时而恶化服务能力。有了服务网格，就可以很轻松的断开失败的服务，以此来禁止无用的副本，从而保持 API 的相应。
2. 蓝绿发布：有流量控制的能力。服务网格可以实现蓝绿发布，从而让应用程序的升级无需终端的安全升级。首先，暴露一小部分用户到新的版本，然后验证，最后再继续将其发布到生产中的所有实例
3. 生产环境中的混沌测试：注入延时、故障的能力，可以帮助提升部署的鲁棒性。
4. 对接已有应用程序：如果你正在迁移现有的应用程序到基于 Kubernetes 的微服务上，可以使用服务网格作为`桥接器`而不用重写你的应用。可以把已有的应用程序作为 `services` 注册成为 Istio 的服务，然后开始逐步的迁移到 Kubernetes，而不用改变现在的服务间通信（像 DNS 路由）。这个案例和使用服务发现类似。
5. API 网关：如果你对服务网格很感兴趣，并且打算使用，但是还没有使用 Kubernetes 的应用程序在跑，可以让你的运维团队部署服务网格来度量你的 API 使用，以此来学习使用服务网格。

在最成熟的实现中，服务网格变成了微服务架构的仪表盘。可以用来分析解决问题，应用流量策略，限流，测试新代码。这将是所有服务交互的监控，追踪和控制中心（它们如何连接，执行和做安全保护）。


原文：https://platform9.com/blog/kubernetes-service-mesh-a-comparison-of-istio-linkerd-and-consul/
<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)
</center>
<!--
<table><tbody>
<tr><th></th><th><img src="imgs/istio.png"><p ><strong>Istio</strong></p></th><th><img  src="imgs/linkerd.png"><p ><strong>Linkerd v2</strong></p></th><th><img  src="imgs/consul.png" alt="" widthheight><p ><strong>Consul</strong></p></th></tr>
<tr><td><strong>Supported Workloads</strong></td><td >Does it support both VMs-based applications and Kubernetes?</td></tr>
<tr><td><strong>Workloads</strong></td><td>Kubernetes + VMs</td><td>Kubernetes only</td><td>Kubernetes + VMs</td></tr>
<tr><td><strong>Architecture</strong></td><td >The solution’s architecture has implications on operation overhead.</td></tr>
<tr><td><strong>Single point of failure</strong></td><td>No – uses sidecar per pod</td><td>No</td><td>No. But added complexity managing HA due to having to install the Consul server and its quorum operations, etc., vs. using the native K8s master primitives.</td></tr>
<tr><td><strong>Sidecar Proxy</strong></td><td>Yes (Envoy)</td><td>Yes</td><td>Yes (Envoy)</td></tr>
<tr><td><strong>Per-node agent</strong></td><td>No</td><td>No</td><td>Yes</td></tr>
<tr><td><strong>Secure Communication</strong></td><td >All services support mutual TLS encryption (mTLS), and native certificate management so that you can rotate certificates or revoke them if they are compromised.</td></tr>
<tr><td><strong>mTLS</strong></td><td>Yes</td><td>Yes</td><td>Yes</td></tr>
<tr><td><strong>Certificate Management</strong></td><td>Yes</td><td>Yes</td><td>Yes</td></tr>
<tr><td><strong>Authentication and Authorization</strong></td><td>Yes</td><td>Yes</td><td>Yes</td></tr>
<tr><td ><strong>Communication Protocols</strong></td></tr>
<tr><td><strong>TCP</strong></td><td>Yes</td><td>Yes</td><td>Yes</td></tr>
<tr><td><strong>HTTP/1.x</strong></td><td>Yes</td><td>Yes</td><td>Yes</td></tr>
<tr><td><strong>HTTP/2</strong></td><td>Yes</td><td>Yes</td><td>Yes</td></tr>
<tr><td><strong>gRPC</strong></td><td>Yes</td><td>Yes</td><td>Yes</td></tr>
<tr><td ><strong>Traffic Management</strong></td></tr>
<tr><td><strong>Blue/Green Deployments</strong></td><td>Yes</td><td>Yes</td><td>Yes</td></tr>
<tr><td><strong>Circuit Breaking</strong></td><td>Yes</td><td>No</td><td>Yes</td></tr>
<tr><td><strong>Fault Injection</strong></td><td>Yes</td><td>Yes</td><td>Yes</td></tr>
<tr><td><strong>Rate Limiting</strong></td><td>Yes</td><td>No</td><td>Yes</td></tr>
<tr><td><strong>Chaos Monkey-style Testing</strong></td><td >Traffic management features allow you to introduce delays or failures to some of the requests in order to improve the resiliency of your system and harden your operations</td></tr>
<tr><td><strong>Testing</strong></td><td>Yes- you can configure services to delay or outright fail a certain percentage of requests</td><td>Limited</td><td>No</td></tr>
<tr><td><strong>Observability</strong></td><td >In order to identify and troubleshoot incidents, you need distributed monitoring and tracing.</td></tr>
<tr><td><strong>Monitoring</strong></td><td>Yes, with Prometheus</td><td>Yes, with Prometheus</td><td>Yes, with Prometheus</td></tr>
<tr><td><strong>Distributed Tracing</strong></td><td>Yes</td><td>Some</td><td>Yes</td></tr>
<tr><td ><strong>Multicluster Support</strong></td></tr>
<tr><td><strong>Multicluster</strong></td><td>Yes</td><td>No</td><td>Yes</td></tr>
<tr><td><strong>Installation</strong></td><td ></td></tr>
<tr><td><strong>Deployment</strong></td><td>Install via Helm and Operator</td><td>Helm</td><td>Helm</td></tr>
<tr><td><strong>Operations Complexity</strong></td><td >How difficult is it to install, configure and operate</td></tr>
<tr><td><strong>Complexity</strong></td><td>High</td><td>Low</td><td>Medium</td></tr>
</tbody></table>
-->