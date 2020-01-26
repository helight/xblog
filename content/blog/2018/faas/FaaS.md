+++
title = "Function as a Service介绍"
date = "2018-11-18T13:47:08+02:00"
tags = ["serverless", "FaaS"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "本文是在今年梳理的关于FaaS的一个文档，主要目标是梳理FaaS的基本概念，发展势头，应用场景和一些基本的架构设计。"
keywords = ["FaaS","ServerLess", "MVP"]
+++

本文是在今年梳理的关于FaaS的一个文档，主要目标是梳理FaaS的基本概念，发展势头，应用场景和一些基本的架构设计。后来在内部参考FaaS的设计思路，我们实现了一套适用于我们自己业务特点的函数服务，结合了DevOps，目前在内部使用的还算不错。目前可以用在实时排行服务，实时用户触达，实时数据清洗处理等等一些场景，以后也会把实现思路拿出来分享的。

![](20190301/2.PNG)

​	现在的各种技术发展，必然是以服务的形式出现的，所以所有的技术以服务化的方式提供这是必然的。在内部我们建设各种系统和平台，在架构设计思路上也必然遵循这样的规范：分层设计和服务化设计。

![](20190301/3.PNG)
![](20190301/4.PNG)
![](20190301/5.PNG)
![](20190301/6.PNG)
![](20190301/7.PNG)
![](20190301/8.PNG)

Serverless在14年出现，并不断发展，到目前为止，其设计和实现不断完善，而且出现了不少开源实现。目前主力还是AWS，在国外使用的非常广泛。国内相对较少，目前有阿里，腾讯和华为几家云服务厂商提供了这类服务。

![](20190301/9.PNG)
![](20190301/10.PNG)

FaaS的出现也是服务发展的必然趋势，在微服务大行其道的现在，FaaS作为服务函数的出现，在服务细粒度实现和计费上给出了很好的解决方案。目前这块属AWS的lambda做的最好了。

下面可以看看FaaS一些常用的场景，目前这些场景大部分我在内部已经落地使用，效果非常不错，在之前看起来非常复杂的系统，现在有了FaaS，开发效率提升非常明显。另外FaaS在开发MVP中有非常大的优势。

![](20190301/11.PNG)
![](20190301/12.PNG)
![](20190301/13.PNG)
![](20190301/14.PNG)
![](20190301/15.PNG)
![](20190301/16.PNG)
![](20190301/17.PNG)
![](20190301/18.PNG)
![](20190301/19.PNG)
![](20190301/20.PNG)
![](20190301/21.PNG)
![](20190301/22.PNG)
![](20190301/23.PNG)
![](20190301/24.PNG)
![](20190301/25.PNG)
![](20190301/26.PNG)
![](20190301/27.PNG)
![](20190301/28.PNG)
![](20190301/29.PNG)
![](20190301/30.PNG)
![](20190301/31.PNG)
![](20190301/32.PNG)
![](20190301/33.PNG)
![](20190301/34.PNG)
![](20190301/36.PNG)