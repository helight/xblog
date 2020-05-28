+++
title = "Golang中 method has pointer receiver 异常"
date = "2019-05-18T13:47:08+02:00"
tags = ["golang"]
categories = ["golang"]
banner = "img/banners/golang.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "Golang中 method has pointer receiver 异常"
keywords = ["golang", "method"]
+++

# Golang中 method has pointer receiver 异常

在Golang中第一次使用interface 遇到了一个有意思的问题：

method has pointer receiver

这个问题很普遍，所以在此记录先来。 
先看以下例子：

为了解决这个问题，首先得先了解一下Golang 中 方法的集合的概念，一个struct虽然可以通过值类型和引用类型两种方式定义方法，但是不通的对象类型对应了不同的方法集：

Values | Methods Receivers
-- | --
| T | (t T)
| *T | (t T) and (t *T) 

值类型的对象只有（t T) 结构的方法，虽然值类型的对象也可以调用(t *T) 方法，但这实际上是Golang编译器自动转化成了&t的形式来调用方法，并不是表明值类型的对象拥有该方法。

换一个维度来看上面的表格可能更加直观：

| Methods Receivers | Values |
| --  | -- |
| (t T) | T and *T |
| (t *T) | *T |

这就意味着指针类型的receiver 方法实现接口时，只有指针类型的对象实现了该接口。

对应上面的例子来说，只有&user实现了notifier接口，而user根本没有实现该接口。所以上面代码会报出这样的异常。

notify method has pointer receiver
1
解决这个问题也很容易，直接使用&user去代替user调用方法即可：
``` golang
func main() {
    // Create a value of type User and send a notification.
    u := user{"Bill", "bill@email.com"}
    sendNotification(&u)
    // PASSED THE ADDRESS AND NO MORE ERROR.
```
希望我通过这个异常，更深入的了解了Golang的interface机制。

<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)
</center>