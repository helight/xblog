---
title: "怎么写 Envoy 的 Filter"
date: 2020-06-07T08:45:20+08:00
tags: ["api", "Envoy", "microservices"]
categories: ["api", "microservices"]
banner: "img/banners/ms.jpg"
author: "helight"
authorlink: "http://helight.cn"
summary: ""
keywords: ["api","gateway", "Envoy"]
draft: true
---

## 前言
Envoy 是一个功能强大服务代理，目前主流的服务网格也在拿它作为数据面，比如 Istio，AWS APP Mesh，ConsulConnect。从一般使用来说还好，直接下镜像就可以使用。但是很多时候需要自定义开发一些功能，这个时候就需要上手改动 Envoy 的代码。不过Envoy 是用 C++ 编写的，上手门槛还是比较高的。所以今天这篇文件来简单介绍一下如何编写一个 Envoy 上的扩展功能。

## Envoy 的 Filter
Envoy 在实现上提供了一种机制：filter，使用这种机制我们开发一个filter，对经过 Envoy 的数据包进行处理。所以下面我们先来介绍一下 Envoy 的 filter。

### Filter 的基本原理
Envoy 暴露了一组 API，我们可以通过这些 API 对 Envoy 进行动态的下发配置信息。