---
title: "envoy filter 开发实践系列 3：编译 envoy 的其它方式"
date: 2020-10-13T08:45:20+08:00
tags: ["Envoy", "microservices"]
categories: ["Envoy", "microservices"]
banner: "img/banners/envoy.png"
author: "helight"
authorlink: "http://helight.cn"
summary: ""
keywords: ["microservices", "Envoy"]
draft: false
---

## 1. 前言
前天早上开会还说这个 envoy 1.16 不知道什么时候发布，我们需要的几个新特性都在这个版本中，今天一看已经发布了，所以今天又测试了一波 1.16 上的例子。

## 2. 使用官方 docker 编译镜像来编译

从 envoy 1.16 开始发现 gcc 7.5 的版本已经无法编译通过了，从其官网推荐来说是要 gcc 9 以上或者要 clang 10 以上。

### 2.1 查看 docker 镜像
可以用于编译的是 `envoyproxy/envoy-build-ubuntu` 这个镜像。

```sh
root@ubuntu:/data/home/ubuntu# docker images
REPOSITORY                      TAG                                        IMAGE ID            CREATED             SIZE
envoyproxy/envoy-dev            latest                                     317be1534a57        4 days ago          129MB
envoyproxy/envoy-build-ubuntu   b480535e8423b5fd7c102fd30c92f4785519e33a   7757d8081892        8 days ago          3.8GB
envoyproxy/envoy-build          latest                                     96175ccf21e5        14 months ago       1.16GB
root@ubuntu:/data/home/ubuntu# 
```
### 2.2 查看 gcc 版本
进入镜像后可以看一下 gcc 的版本号，这个镜像使用的是 9.3 的 gcc。
```sh
root@ubuntu:/data/home/ubuntu# docker run -t -i 7757d8081892 /bin/bash
root@0bef984284ca:/# gcc -v
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-linux-gnu/9/lto-wrapper
OFFLOAD_TARGET_NAMES=nvptx-none:hsa
OFFLOAD_TARGET_DEFAULT=1
Target: x86_64-linux-gnu
Configured with: ../src/configure -v --with-pkgversion='Ubuntu 9.3.0-11ubuntu0~18.04.1' --with-bugurl=file:///usr/share/doc/gcc-9/README.Bugs --enable-languages=c,ada,c++,go,brig,d,fortran,objc,obj-c++,gm2 --prefix=/usr --with-gcc-major-version-only --program-suffix=-9 --program-prefix=x86_64-linux-gnu- --enable-shared --enable-linker-build-id --libexecdir=/usr/lib --without-included-gettext --enable-threads=posix --libdir=/usr/lib --enable-nls --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --with-default-libstdcxx-abi=new --enable-gnu-unique-object --disable-vtable-verify --enable-plugin --enable-default-pie --with-system-zlib --with-target-system-zlib=auto --enable-objc-gc=auto --enable-multiarch --disable-werror --with-arch-32=i686 --with-abi=m64 --with-multilib-list=m32,m64,mx32 --enable-multilib --with-tune=generic --enable-offload-targets=nvptx-none,hsa --without-cuda-driver --enable-checking=release --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu
Thread model: posix
gcc version 9.3.0 (Ubuntu 9.3.0-11ubuntu0~18.04.1) 
root@0bef984284ca:/#
```
### 2.3 运行 docker 镜像
这里要把把源码映射到 docker 内，再到 docker 内的目录内进行编译，如下：
```sh
docker run -v /data/mesh/envoy-filter-example:/envoy-filter-example -t -i 7757d8081892 /bin/bash
root@300e54c54ce4:/# cd envoy-filter-example/
root@300e54c54ce4:/envoy-filter-example# bazel build //:envoy
```
## 3. 使用 clang 编译 envoy

在公司电脑上安装 gcc 9 要依赖外部源而且非常慢，所以就直接安装 clang 了。发现 clang 的用户体验做的非常不错，在网站直接给出各种系统安装的指引，而且安装起来也非常简单。而 gcc 那个千年不变的页面基本只提供源码编译安装

llvm 的官网在这里：[https://apt.llvm.org/](https://apt.llvm.org/)

### 3.1 安装方式 1
使用下面的方式可以安装最新稳定版的 clang。
```sh
root@ubuntu:/data/home/ubuntu# bash -c "$(wget -O - https://apt.llvm.org/llvm.sh)"
```
### 3.2 安装方式 2
根据其官网指引配置相应的源地址，我使用的 ubuntu 18.04，所以源配置如下：
```sh
Bionic (18.04) - Last update : Tue, 13 Oct 2020 20:00:24 UTC / Revision: 20201013091414+1687a8d83b7
# i386 not available
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic main
# 10
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-10 main
# 11
deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-11 main
deb-src http://apt.llvm.org/bionic/ llvm-toolchain-bionic-11 main
```
配置完成之后直接 `apt-get update` 进行信息更新后就可以直接安装了。

```sh
root@ubuntu:/data/home/ubuntu# apt-cache search clang
......
clang-11 - C, C++ and Objective-C compiler
clang-11-doc - C, C++ and Objective-C compiler - Documentation
clang-11-examples - Clang examples
clang-format-11 - Tool to format C/C++/Obj-C code
clang-tidy-11 - clang-based C++ linter tool
clang-tools-11 - clang-based tools for C/C++ developments
clangd-11 - Language server that provides IDE-like features to editors
libclang-11-dev - Clang library - Development package
libclang-common-11-dev - Clang library - Common development package
libclang-cpp11 - C++ interface to the Clang library
libclang-cpp11-dev - C++ interface to the Clang library
libclang1-11 - C interface to the Clang library
liblldb-11 - Next generation, high-performance debugger, library
liblldb-11-dev - Next generation, high-performance debugger, header files
lldb-11 - Next generation, high-performance debugger
python3-clang-11 - Clang Python Bindings
python3-lldb-11 - Next generation, high-performance debugger, python3 lib
root@ubuntu:/data/home/ubuntu# 
```
执行安装：
```sh
root@ubuntu:/data/home/ubuntu# apt-get install clang-11
```
### 3.3 版本设置和环境变量配置
设置版本，我测试过 clang 10 和 clang 11 两个版本编译，都没问题。
```sh
root@ubuntu:/usr/bin# rm /usr/bin/clang
root@ubuntu:/usr/bin# rm /usr/bin/clang++
root@ubuntu:/usr/bin# ln -s ../lib/llvm-11/bin/clang /usr/bin/clang
root@ubuntu:/usr/bin# ln -s ../lib/llvm-11/bin/clang++ /usr/bin/clang++
root@ubuntu:/usr/bin# 
```
配置环境变量到 `~/.bashrc`，从新打开一个终端或者执行一下 `source ~/.bashrc` 就可以用了。
```sh
export CXX=clang++                                                                                  
export CC=clang 
```
使用 `env` 来查看环境信息。
```sh
ubuntu@ubuntu:~$ env
...
LANG=en_US.utf8
CXX=clang++
CC=clang
...
ubuntu@ubuntu:~$ 
```
## 4. 编译 envoy 1.16
接下来就来编译吧，编译过程和之前介绍的大体差不多，但是这里我们是要把 envoy 的版本 check 到 1.16 上去。
```sh
ubuntu@ubuntu:/data/mesh$ git clone https://github.com/envoyproxy/envoy-filter-example
ubuntu@ubuntu:/data/mesh$ cd envoy-filter-example
ubuntu@ubuntu:/data/mesh/envoy-filter-example$ git submodule update --init
ubuntu@ubuntu:/data/mesh/envoy-filter-example/envoy$ git checkout 8fb3cb86082b17144a80402f5367ae65f06083bd
HEAD is now at 8fb3cb86 release: cutting 1.16.0 (#13438)
ubuntu@ubuntu:/data/mesh/envoy-filter-example/envoy$ cd ../
ubuntu@ubuntu:/data/mesh/envoy-filter-example$ bazel build //:envoy
```

## 5. 总结
从目前交流的情况来看 envoy 的编译确实是一个大问题，编译环境配置还是比较复杂的，开发体验并不是很好。包括我看源码中提供的 ci 脚本，也是非常复杂，大家入门会有很大的障碍。

所以我这里梳理了几篇文章，从简单的例子编译到相对复杂的多种编译方式介绍了入门级的编译方法。对大家入门估计比较有用。


下一篇，我会根据这几篇文章来从原理上介绍一下 envoy 的 filter。

***系列文章：***
1. [envoy filter 开发实践系列 1：官网 echo filter 示例编译测试](http://www.helight.cn/blog/2020/build-envoy-filter-echo/)
2. [envoy filter 开发实践系列 2：官网 http filter 示例编译测试](http://www.helight.cn/blog/2020/build-envoy-filter-http/)


<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)
</center>