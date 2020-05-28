+++
title = "Http协议分析"
date = "2009-02-17T13:47:08+02:00"
tags = ["linux", "开源", "http"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "最简单的例子，就是你的浏览器与网页服务器之间使用的应用层协议。虽然官方文档说HTTP协议可以建立在任何可靠传输的协议之上，但是就我们所见到的，HTTP还是建立在TCP之上的。"
keywords = ["开源", "linux", "http"]
+++

# 1.1Http协议分析 什么是HTTP协议？
最简单的例子，就是你的浏览器与网页服务器之间使用的应用层协议。虽然官方文档说HTTP协议可以建立在任何可靠传输的协议之上，但是就我们所见到的，HTTP还是建立在TCP之上的。 httpd最简单的response是返回静态的HTML页面。在该设计中不只要实现对静态网页的响应，还需要实现CGI特性。

首先要分析HTPP协议。关于HTTP协议的详细文档，可以参看RFC文档rfc2616：http://www.w3.org/Protocols /rfc2616/rfc2616.html 。还有一个简单HTTP协议：http://jmarshall.com/easy/http/ 。一下的分析和设计的依据都来自于此。

## 1.1.1HTTP协议结构
HTTP协议无论是请求报文(request message)还是回应报文(response message)都分为四部分：
* 报文头 (initial line )
* 0个或多个header line
* 空行(作为header lines的结束)
* 可选body HTTP协议是基于行的协议，每一行以\r\n作为分隔符。

报文头通常表明报文的类型(例如请求类型)，报文头只占一行；header line 附带一些特殊信息，每一个header line占一行，其格式为name:value，即以分号作为分隔；空行也就是一个\r\n；可选body通常 包含数据，例如服务器返回的某个静态HTML文件的内容。举个例子，以下是一个很常见的请求报文，你可以截获浏览器发送的数据包而获得：
``` sh
GET / HTTP/1.1
Host: 127.0.0.1:8080
User-Agent: Mozilla/5.0 (X11; U; Linux i686; en-US; rv:1.9.0.3) Gecko/2008092816 Icew easel/3.0.3 (Debian-3.0.3-3)
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
Accept-Language: en-us,en;q=0.5
Accept-Encoding: gzip,deflate
Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7
Keep-Alive: 300
Connection: keep-alive
```
第1行就是initial line，2-9行是header lines，10行是一个header line的结束符，没有显示出来。 以下是一个回应报文：
``` sh
HTTP/1.1 200 OK
Server: xiyouhttpd/0.1.0
Content-Type: text/html
Content-Length: 72

index.html
```
第6行就是可选的body，这里是index.html这个文件的内容。

## 1.1.2HTTP request method 首先重点分析请求报文。
先看initial line，该行包含几个字段，每个字段用空格分开，例如以上的GET /index.html HTTP/1.1就可以分为三部分：GET、/index.html、HTTP/1.1。其中第一个字段GET就是所谓的request method。

它表明请求类型，HTTP有很多method，例如：GET、POST、HEAD等。 就针对于该系统的目标而言，只需要实现对GET、HEAD等几个常用的请求做响应即可。

GET是最普遍的method，表示请求一个资源。什么是资源？诸如HTML网页、图片、声音文件等都是资源。顺便提一句，HTTP协议中为每一个资源设置一个唯一的标识符，就是所谓的URI(更宽泛的URL)。

HEAD与GET一样，不过它不请求资源内容，而是请求资源信息，例如文件长度等信息。 initial line后面的内容： 对应于GET和HEAD两个method，紧接着的字段就是资源名，其实从这里可以看出，也就是文件名(相对于你服务器的资源目录)，例 如这里的/index.html；最后一个字段表明HTTP协议版本号。目前我们只需要支持HTTP1.1和1.0，没有多大的技术差别。 然后是header line。我们并不需要关注每一个header line。我只列出常用和必须的几个header line : - Host : 对于HTTP1.1而言，请求报文中必须包含此header，如果没有包含，服务器需要返回bad request错误信息。 - Date : 用于回应报文，用于客户端缓存数据用。 - Content-Type : 用于回应报文，表示回应资源的文件类型，以MIME形式给出。什么是MIME？它们都有自己的格式，例如： text/html, image/jpg, image/gif等。 - Content-Length : 用于回应报文，表示回应资源的文件长度。

body域很简单，你只需要将一个文件全部读入内存，然后附加到回应报文段后发送即可，即使是二进制数据。 - 回应报文 上面已经给出了一个回应报文的例子，以其为例说明。首先是initial line，第一个字段表明HTTP协议版本，可以直接以请求报文为准(即请求报文版本是多少这里就是多少)；第二个字段是一个status code，也就是回应状态，相当于请求结果，请求结果被HTTP官方事先定义，例如200表示成功、404表示资源不存在等；最后一个字段为status code的可读字符串，你随便给吧。 回应报文中最好跟上Content-Type、Content-Length等header。


<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
