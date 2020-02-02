+++
title = "Open Source"
description = "helight"
type = "about"
date = "2019-02-28"
+++

## Open Source---我自己设立的一些小项目。。。
### openflow
项目简介：可视化分布式流程控制系统，数据流和业务流控制系统。 

项目地址：[https://github.com/helight/oneflow](https://github.com/helight/oneflow)

### xrpc
项目简介：rpc server  implemented in c++ json, libev, easy to use 

项目地址：[https://github.com/helight/xrpc](https://github.com/helight/xrpc)  

### 小丁音乐
项目简介：Music player for linux, Has the basic music player function，just for fun. 

项目地址：[https://github.com/helight/xdmusic](https://github.com/helight/xdmusic)  

### Xcut
项目简介：xcut is a C Unit Test framework maintained by HelightXu. xcut is another branch of lcut and more feature then lcut! 

项目地址：[https://github.com/helight/xcut/](https://github.com/helight/xcut/)  

### Linux嵌入式轻量级http服务器
项目简介：编写一个可以在linux嵌入式下轻量级的http服务器软件，能够支持简单的静态页面的传输和cgi技术.目标是可以移植任意操作系统，但是主要针对于嵌入式linux。主要针对于嵌入式系统的web管理的实现。 

项目地址：[https://github.com/helight/xhttpd](https://github.com/helight/xhttpd ) 

### Xnotebook文本资料管理软件
项目简介：Xnotebook是一款用gtk＋开发的文本资料管理软件，它主要用于个人文本资料的分类管理。 

项目地址：[https://github.com/helight/xnotebook](https://github.com/helight/xnotebook)  
### XGCom串口调试工具
项目简介：一个Linux下的图形化的串口调试工具.帮助开发者调试串口程序。 

项目地址：[https://github.com/helight/xgcom](https://github.com/helight/xgcom)

## ARM Program---学习ARM时写的程序或文档。。。
### Linux ARM开发文档
项目简介：编写一个关于linux在arm上比较实用的开发文档，当前使用开发芯片有pxa270和S3C2410. 

项目地址：[查看项目当前版本](/coding/arm-linux-compile.pdf)  
学习编写嵌入式linux下的一些硬件驱动的开发
自己编写的网卡驱动，spi驱动，GPIO驱动，按键驱动等。。。

## School & Lab Project---Project in school。。。
### 数字微波通信设备网络管理系统
这是我研究生阶段的第一个横向项目。在该项目中主要完成了硬件电路板的软测，在ZLG移植UCOSII的基础之上完成了LPC2214上的移植，并将原有的51c程序移植到LPC2214的UCOSII上，由于系统是双CPU双OS的，所以在前期工作完成后，后期我又转向UClinux系统的移植和其中串口程序的编写，并且编写的虚拟网卡驱动，完成网卡数据到串口的转发。最后做了对整个系统的测试工作。感谢在此期间给予我帮助的所有老师。 项目状态：已经完成，已交付企业投入市场，目前正在进行下一个版本的开发。。。

### 内核编程---学习内核写的小程序。。。
2.6.22下基于Netfilter的网络监听程序
在2.6.22中skbuff发生了变化，使得我以前的防火墙程序在新内核中无法使用了，主要是可以当作一个网络数据监视，当然还是不完善的。目前只能监听数据报的源ip和目的ip，还有tcp报的原端口和目的端口。 今天搞了一下，终于又可以了，

下面是程序： [sniffnet.c](/coding/sniffnet.c)

## Tools---收集或是自己做的一些小工具。。。
### C 函数库
是学习c编程的一个小助手，我将它收录到了这里。 

[C 函数库](/tools/clib/clib.html)