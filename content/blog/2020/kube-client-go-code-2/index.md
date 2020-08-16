---
title: "k8s代码走读---client-go 编程之 informers"
date: 2020-08-14T08:45:20+08:00
tags: ["k8s", "microservices"]
categories: ["k8s", "microservices"]
banner: "img/banners/kubernetes.jpeg"
author: "helight"
authorlink: "http://helight.info"
summary: ""
keywords: ["k8s","microservices"]
draft: false
---

## 前言
根据我们社区制定的计划，本周是开始走读 client-go 中的 informers 模块了，但是无奈这周时间是相当的不充裕，公司内的事情也突然多了几当子要紧急赶工的事情，另外就是准备 GIAC 和和社区的同仁们组织云原生社区深圳站的交流，接着 GIAC 的风也顺便把这些天南地北的同仁们聚到了一起。所以这部分代码的走读计划还是有所延误了，但是还是要走起。

## informers 机制介绍
Informer(就是SharedInformer)是client-go的重要组成部分，在了解client-go之前，了解一下Informer的实现是很有必要的，下面引用了官方的图，可以看到Informer在client-go中的位置。
![](imgs/informer.jpeg)
主要使用到 Informer和workqueue两个核心组件。Controller可以有一个或多个informer来跟踪某一个resource。Informter跟API server保持通讯获取资源的最新状态并更新到本地的cache中，一旦跟踪的资源有变化，informer就会调用callback。把关心的变更的Object放到workqueue里面。然后woker执行真正的业务逻辑，计算和比较workerqueue里items的当前状态和期望状态的差别，然后通过client-go向API server发送请求，直到驱动这个集群向用户要求的状态演化。

