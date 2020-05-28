+++
title = "内核补丁制作与提交"
date = "2010-07-18T13:47:08+02:00"
tags = ["git", "kernel", "开源"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "内核补丁制作与提交"
keywords = ["git", "kernel", "开源", "linux"]
+++

# 内核补丁制作与提交
作者：<a href="mailto:zhwenxu@gmail.com">许振文</a>

## 内核源码下载
内核代码的管理是采用的git来管理的，所以要下载内核最新的代码的话就得使用git工具来下载了。git工具的使用我在上一篇文章中已经写了个大概，所以具体可以参考其中的使用说明。

一般测试可以下载部分或是全部的源代码，我们一般会下载Linus Torvalds的git树下的源码。URL如下：
``` sh
http://git.kernel.org/?p=linux/kernel/git/torvalds/linux-2.6.git;a=summary
description		Linus' kernel tree
owner			Linus Torvalds
last change		Sun, 19 Apr 2009 17:58:20 +0000
URL		git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6.git
		http://www.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6.git
``` 
使用git工具下载内核源代码时，可以使用git://... 的URL和http://... 的URL，两者只是服务端口不一样而已，git的服务端口是9418。其它两者具体没有什么太大区别，主要有些公司或是个人网络可能会封闭一些不太用的端口。下面是使用git下载源代码的过程。

使用的命令是：git-clone URL （在要下载内核源码的目录下执行）
``` sh
helight@Zhwen:linux$git-clone git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6.git
Initialized empty Git repository in /home/helight/linux-2.6/.git/
remote: Counting objects: 1171309, done.
remote: Compressing objects: 100% (189000/189000), done.
Receiving objects:   0% (732/1171309), 275.99 KiB | 8 KiB/s   
...
``` 
## 源代码的更新
当然新下载的源代码当然就没必要更新了，但是我们也不需要每天都去下载新的源代码，因为源代码的服务器在国外，国内用户下载速度比较慢，git工具中给我们提供了增量更新源代码的方式，这中方式是是下载并且更新和上次不一样的源代码内容，而不是从新下载所有的源代码。一下是更新过程：

使用的命令是：git-pull （在内核源代码根目录下直接执行）
``` sh
helight@Zhwen:linux-2.6$ git-pull 
remote: Counting objects: 522, done.
remote: Compressing objects: 100% (68/68), done.
remote: Total 325 (delta 260), reused 320 (delta 255)
Receiving objects: 100% (325/325), 47.61 KiB | 10 KiB/s, done.
Resolving deltas: 100% (260/260), completed with 90 local objects.
From git://git.kernel.org/pub/scm/linux/kernel/git/torvalds/linux-2.6
   aefe647..d91dfbb  master     -> origin/master
Updating aefe647..d91dfbb
Fast forward
 .gitignore                          |    1 +
 Documentation/kbuild/makefiles.txt  |   10 ++
 Documentation/kernel-parameters.txt |   38 +++++++
 Documentation/lguest/.gitignore     |    1 +
 Documentation/lguest/lguest.txt     |   11 +-
 MAINTAINERS                         |    5 +
 Makefile                            |    2 +-
 arch/x86/include/asm/lguest_hcall.h |    2 +-
 arch/x86/lguest/boot.c              |   16 ++--
...
 50 files changed, 602 insertions(+), 388 deletions(-)
 create mode 100644 Documentation/lguest/.gitignore
helight@Zhwen:linux-2.6$ 
``` 
这里要注意的是：每次要更新都要在master分支上进行更新，以保证更新不会发生混乱。在master分支上更新之后再创建新的分支。

## 代码分支的管理：

### 分支查看：git-branch （在源代码根目录下直接执行）
``` sh
helight@Zhwen:linux-2.6$ git-branch 
* master
helight@Zhwen:linux-2.6$ 
``` 
刚刚下载的内核代码树只有一个分支就是“master”，这个分支我们一般不做修改，我们做的所有修改都是在自己新建的分支上，然后再针对于“master”分支作补丁。所以这个分支一般情况下最好不要修改。

### 建立新分支：git-branch helight（ 在源代码根目录下直接执行）
``` sh
helight@Zhwen:linux-2.6$ git-branch helight
helight@Zhwen:linux-2.6$ git-branch 
  helight
* master
helight@Zhwen:linux-2.6$
``` 
这里可以看出在master这个分支的前面有一个“＊”号，这表示但前我们工作在master这个分支上，就是说我们这个时候对内核代码做的所有修改都记录保存在这个分支的数据库中。但是前面已经说过了这个分支上最好不要作修改。

### 分支删除：git-branch -D xxx （在源代码根目录下直接执行）
``` sh
helight@Zhwen:linux-2.6$ git-branch xxx
helight@Zhwen:linux-2.6$ git-branch 
  helight
* master
  xxx
helight@Zhwen:linux-2.6$ git-branch -D xxx
Deleted branch xux.
helight@Zhwen:linux-2.6$ git-branch 
  helight
* master
helight@Zhwen:linux-2.6$ 
``` 
	这个主要是用于删除不需要的分支。</br></br>

### 分支切换：git-checkout helight （在源代码根目录下直接执行）
``` sh
helight@Zhwen:linux-2.6$ git-checkout helight 
Switched to branch "helight"
helight@Zhwen:linux-2.6$ git-branch 
* helight
  master
helight@Zhwen:linux-2.6$ 
``` 
	切换分支后可以看到在你切换后的分支标签前面就会出现“＊”号。

## 修改分支：	
这样你就可以在这个新建的分支上面修改内核源码了。

修改源码…….修改完成。

在修改完内核源代码之后就要线提交到本分支，然后在针对于master分支做补丁。

## 提交修改到本分支：git-commit -m " The reason you commit" ./*
``` sh
helight@Zhwen:linux-2.6$ vim samples/tracepoints/tracepoint-sample.c 
helight@Zhwen:linux-2.6$ git-commit -m "clean up code style on samples/tracepoints/tracepoint-sample.c" ./*
Created commit 048f96f: clean up code style on samples/tracepoints/tracepoint-sample.c
 1 files changed, 1 insertions(+), 2 deletions(-)
helight@Zhwen:linux-2.6$ 
``` 
提交之后可以看出其改变的文件个数和行术等信息。接下来就是制作补丁了。
	
## 制作补丁：git-format-patch master
``` sh
helight@Zhwen:linux-2.6$ git-format-patch master 
0001-clean-up-code-style-on-samples-tracepoints-tracepoin.patch
helight@Zhwen:linux-2.6$ cat 0001-clean-up-code-style-on-samples-tracepoints-tracepoin.patch 
From 048f96f08d2e1d3bb776560a7d1b34cf82e5ea1a Mon Sep 17 00:00:00 2001
From: Zhenwen Xu <helight.xu@gmail.com>
Date: Mon, 20 Apr 2009 16:07:00 +0800
Subject: [PATCH] clean up code style on samples/tracepoints/tracepoint-sample.c

---
 samples/tracepoints/tracepoint-sample.c |    3 +--
 1 files changed, 1 insertions(+), 2 deletions(-)

diff --git a/samples/tracepoints/tracepoint-sample.c b/samples/tracepoints/tracepoint-sample.c
index 9cf80a1..7a3865c 100644
--- a/samples/tracepoints/tracepoint-sample.c
+++ b/samples/tracepoints/tracepoint-sample.c
@@ -35,8 +35,7 @@ static struct file_operations mark_ops = {
 static int __init sample_init(void)
 {
 	printk(KERN_ALERT "sample init\n");
-	pentry_sample = proc_create("tracepoint-sample", 0444, NULL,
-		&mark_ops);
+	pentry_sample = proc_create("tracepoint-sample", 0444, NULL, &mark_ops);
 	if (!pentry_sample)
 		return -EPERM;
 	return 0;
-- 
1.5.6.5

helight@Zhwen:linux-2.6$ 
``` 
这个工具制作的补丁中一般不加“ Signed-off-by”这个标签，这个标签是标识这个补丁是誰发的或是誰转发的。这个不加也可能是我的git配置的问题。所以一般我是手动添加的，如果谁知道怎么可以自动添加请告诉我一下。

而且补丁自动生成的前面4行也做好删除。修改后的补丁如下：

``` sh
helight@Zhwen:linux-2.6$ vim 0001-clean-up-code-style-on-samples-tracepoints-tracepoin.patch 
helight@Zhwen:linux-2.6$ cat 0001-clean-up-code-style-on-samples-tracepoints-tracepoin.patch 
Signed-off-by: Zhenwen Xu <helight.xu@gmail.com>
---
 samples/tracepoints/tracepoint-sample.c |    3 +--
 1 files changed, 1 insertions(+), 2 deletions(-)

diff --git a/samples/tracepoints/tracepoint-sample.c b/samples/tracepoints/tracepoint-sample.c
index 9cf80a1..7a3865c 100644
--- a/samples/tracepoints/tracepoint-sample.c
+++ b/samples/tracepoints/tracepoint-sample.c
@@ -35,8 +35,7 @@ static struct file_operations mark_ops = {
 static int __init sample_init(void)
 {
 	printk(KERN_ALERT "sample init\n");
-	pentry_sample = proc_create("tracepoint-sample", 0444, NULL,
-		&mark_ops);
+	pentry_sample = proc_create("tracepoint-sample", 0444, NULL, &mark_ops);
 	if (!pentry_sample)
 		return -EPERM;
 	return 0;
-- 
1.5.6.5

helight@Zhwen:linux-2.6$ 
``` 
## 补丁检测：./scripts/checkpatch.pl xxx.patch
	这个时候补丁就制作成了，但是还要使用内核提供的补丁检测工具检测一下补丁的正确性，应为我们发布的补丁不能把很明显错误带进去。
``` sh
helight@Zhwen:linux-2.6$ ./scripts/checkpatch.pl 0001-clean-up-code-style-on-samples-tracepoints-tracepoin.patch 
total: 0 errors, 0 warnings, 9 lines checked

0001-clean-up-code-style-on-samples-tracepoints-tracepoin.patch has no obvious style problems and is ready for submission.
helight@Zhwen:linux-2.6$ 
``` 
如果你认为你做的补丁有意义而且是正确的（这里只是为了演示，这个补丁并没有多大的实际意义，只是用于演示使用），而且也通过了这一步，那么你就可以发布你的补丁了。

## 发布补丁：
补丁的发布一般是以邮件的形式来发布，而首先你就要知道你这个补丁应该发给谁，有那些人你必须要发。这里首先你要向内核邮件列表发一份：linux-kernel@vger.kernel.org， 其次还要给你所修改的这个文件所在的内核子邮件列表和维护者发一份，这些信息可以在内核源代码目录下的“MAINTAINERS”文件中查到，使用VIM打开发该文件后，进行关键词的搜索就可以了，搜到后会有相应的内核子邮件列表和维护者的邮件地址。然后就可以发送了。发送补丁一般只需要把补丁内容加入到邮件正文发送即可。

## 特别注意：
提交时邮件的题目为：[PATCH] The reason you make this patch

提交时必须注明修改原因和修改文件。这些一般写在邮件正文的一开始。

补丁一般不要以附件发送，而是直接写到正文中。在修改原因等写完之后就附加补丁内容。在VIM中可以使用:r xxx.patch将补丁添加进邮件正文。（这里是说使用mutt来发送邮件）


<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
