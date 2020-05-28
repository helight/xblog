+++
title = "嵌入式开发中NFS调试环境搭建-V0.2"
date = "2010-01-18T13:47:08+02:00"
tags = ["nfs", "开源"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "NFS(Network File System, 网络文件系统)可以通过NFS把远程主机的目录挂载到本机,使得访问远程主机的目录就像访问本地目录一样方便快捷。"
keywords = ["nfs", "文件系统", "开源", "linux"]
+++

# 嵌入式开发中NFS调试环境搭建-V0.2
作者：<a href="mailto:zhwenxu@gmail.com">许振文</a>

# 开发环境：S3C4510B+uCLinux－2.4.x+ubuntu7.10

建立的目的：可以直接在开发板上挂载开发主机上的文件系统，减少程序的烧写次数，提高程序开发速度。

NFS(Network File System, 网络文件系统)可以通过NFS把远程主机的目录挂载到本机,使得访问远程主机的目录就像访问本地目录一样方便快捷。
NFS一般是实现linux系统之间实现共享.当然和unix之间也应该可以使用它来实现共享。但如果需要在linux和windows系统之间共享, 就得使用samba了!，
NFS是一个RPC服务程序,所以在使用它之前, 先要映射好端口——通过portmap设定. 比如: 某个NFS client发起NFS服务请求时, 它需要先得到一个端口(port).所以它先通过portmap得到port number.所以在启动NFS之前, 需要启动portmap服务！

# 安装NFS服务程序：

Ubuntu上默认是没有安装NFS服务器的，首先要安装NFS服务程序：
``` sh
sudo apt-get install nfs-kernel-server
```
(安装nfs-kernel-server时，apt会自动安装nfs-common和portmap）
这样，宿主机就相当于NFS Server。

与NFS相关的几个文件, 命令
1、/etc/exports 对共享目录的管理都是在这个文件中实现的
2、/sbin/exportfs 维护NFS的资源共享.通过它可以使修改后的/etc/exports中的的共享目录生效关于这个命令的使用方法如下：

 exportfs [-aruv]

 -a ：全部mount或者unmount /etc/exports中的内容

 -r ：重新mount /etc/exports中分享出来的目录

 -u ：umount 目录

 -v ：在 export 的?r候，将详细的信息输出到屏幕上。

3、/usr/sbin/showmount 用在 NFS Server 端。主要用查看 RPC共享的连接

4、/var/lib/nfs/xtab NFS的记录文档:通过它可以查看有哪些Client 连接到NFS主机的记录.

下面这几个文件并不直接负责NFS, 实际上它们负责所有的RPC

5、/etc/default/portmap 实际上, portmap负责映射所有的RPC服务端口

6、/etc/hosts.deny 设定拒绝portmap服务的主机

7、/etc/hosts.allow 设定允许portmap服务的主机

# 添加共享目录：

1.修改/etc/exports。/etc/exports是nfs服务器的核心配置文件。在/etc/exports中添加一个共享目录。

/var/nfs/ *(rw,sync)
/var/nfs/是要共享的文件夹，*是表示所有用户都可以挂载这个共享文件夹。这里也可以替换成ip地址，网段（192.168.1.0/24）
或是主机名。(rw,sync)表示以读写方式挂载，并且远程主机同步，sync是NFS的默认选项。关于括号内的参数还有以下几种：
rw：可读写的权限；
ro：只读的权限；
no_root_squash：登入到NFS主机的用户如果是ROOT用户，他就拥有ROOT的权限，此参数很不安全，建议不要使用。
root_squash：all_squash：不管登陆NFS主机的用户是什么都会被重新设定为nobody。
anonuid：将登入NFS主机的用户都设定成指定的user id,此ID必须存在于/etc/passwd中。
anongid：同 anonuid ，但是?成 group ID 就是了！
sync：资料同步写入存储器中。
async：资料会先暂时存放在内存中，不会直接写入硬盘。insecure 允许从这台机器过来的非授权访问。

2 使用命令sudo exportfs -r 更新

3.重新启动portmap服务和nfs-kernel-server服务

命令分别为：
``` sh
/etc/init.d/portmap start

/etc/init.d/nfs-kernel-server restart
```
# uclinux端的配置：

在uclinux端在还需作一些配置才可以使用mount来挂载远程主机的NFS共享目录.配置修改如下：

1)配置内核

选中ramdisk驱动，加入ext2文件系统
``` sh
Networking options --> (缺省)

Network device support --> (缺省)

File systems --> Network File Systems --> NFS file system support

Provide NFSv3 client support
```
2)用户程序配置(ramdisk中)
``` sh
Network Applications --> portmap

BusyBox --> mount(mount nfs support)
```
# 测试使用NFS：

测试NFS启动客户端uclinux输入命令：
``` sh
mount -t nfs 192.168.1.242:/var/nfs /mnt -o nolock
```
可以使用ls /mnt查看挂载过来的文件。在uclinux下挂载远程主机的共享文件主要是为了实现远程调试。在远程主机上进行交叉编译
之后，在uclinux下直接运行编译好的程序。

# 特别说明：

该配置可适用于几乎所有的嵌入式开发，这里只是以Uclinux为例来说明！！！




<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
