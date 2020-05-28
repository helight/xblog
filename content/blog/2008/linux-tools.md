+++
title = "Linux下UML工具和截图工具的使用"
date = "2008-11-10T13:47:08+02:00"
tags = ["UML", "tools"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "花了大半天的时间终于把HttpServer的需求文档写完了，其中还用到了UML工具和截图工具，作为一般使用这两个工具我个人认为还是很优秀的。现在就介绍一下。"
keywords = ["UML", "dia", "scort"]
+++

花了大半天的时间终于把HttpServer的需求文档写完了，其中还用到了UML工具和截图工具，作为一般使用这两个工具我个人认为还是很优秀的。现在就介绍一下。 

# UML工具：
在百度摆了一下，出现了一个小工具gaphor。可是我安装了后发现，这家伙还的确有点不太好用，我问了小组的同学他们告诉我说可以使用dia，安装后感觉还不错。后来发现它的功能也太多了，不尽可以画UML，还有sisco的图，ER图等，好多，都可以画，还是很强大阿。 

还有就是截图工具，上面这个图片就是用截图工具截取的，还可以吧，这个截图工具足够我们平时使用了，要是截的复杂一点就需要GIMP出马了，但是一般使用是足够了，这个截图工具就是－－－－scrot scrot 是一个使用 imlib2 库截取屏幕和保存图像的的工具，没有GUI界面，一个命令行式的截图软件(Text模式下不适用)。非常方便，自由。 
下面是一些基本用法:（搜自网络） 
scrot [选项] [file] 选项 
* -h, --help--显示帮助并且退出 
* -v, --version--显示版本信息并且退出
* -b, --border--当选择一个窗口时，同时包含窗口边框。
* -c, --count--延时时的显示倒计时 
* -d, --delay NUM--延时 NUM 秒 
* -e, --exec APP--对保存的图像执行程序 APP 
* -q, --quality NUM--图像质量 (1-100) 值大意味着文件大， 压缩率低。 
* -m, --multidisp--对多个显示设备分别截图并且连接在一起。 
* -s, --select--用鼠标交互式的选择一个窗口或者区域。 
* -t, --thumb NUM--同时生成缩略图。 NUM 是缩略图的百分比。 

file 指定截图保存的文件名。 如果 [file] 没有指定，截图就会以当前的日期和时间为文件名保存在当前目录中. 命令行的，酷吧！我一般使用“scort -s”选择区域截图，截图后保存在运行命令的所在目录下。


<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
