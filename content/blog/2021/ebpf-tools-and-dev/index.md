---
title: "linux5.3.0编译测试ebpf"
date: 2021-03-30T08:45:20+08:00
tags: ["kernel", "ebpf"]
categories: ["kernel", "ebpf"]
banner: "img/banners/linux_ebpf_support.png"
author: "helight"
authorlink: "http://helight.cn"
summary: ""
keywords: ["kernel","ebpf"]
draft: true
---

## 前言
环境：ubuntu 18.04，内核版本 5.3.0。
安装内核：
```sh
apt-get install linux-image-5.3.0-70-generic
apt-get install linux-headers-5.3.0-70-generic
apt-get install linux-source-5.3.0
```

## ebpf-tools编译
下面简单记录编译 bpf-tools 时的一些过程。
源码解压到`/data/linux-source-5.3.0/`目录下，进行工具编译。直接进入工具目录进行编译：
```sh
cd /data/linux-source-5.3.0/tools/bpf/
```
### 问题1
#### bfd.h: No such file or directory
```sh
root@VM-74-51-ubuntu:/data/linux-source-5.3.0/tools/bpf# make

Auto-detecting system features:
...                        libbfd: [ OFF ]
...        disassembler-four-args: [ OFF ]

  CC       bpf_jit_disasm.o
/data/linux-source-5.3.0/tools/bpf/bpf_jit_disasm.c:23:10: fatal error: bfd.h: No such file or directory
 #include <bfd.h>
compilation terminated.
Makefile:57: recipe for target 'bpf_jit_disasm.o' failed
make: *** [bpf_jit_disasm.o] Error 1
```
#### 解决
```sh
apt-get install binutils-dev
```
### 问题2
#### readline/readline.h: No such file or directory
```sh
root@VM-74-51-ubuntu:/data/linux-source-5.3.0/tools/bpf# make

Auto-detecting system features:
...                        libbfd: [ on  ]
...        disassembler-four-args: [ on  ]

  CC       bpf_jit_disasm.o
  LINK     bpf_jit_disasm
  CC       bpf_dbg.o
/data/linux-source-5.3.0/tools/bpf/bpf_dbg.c:43:10: fatal error: readline/readline.h: No such file or directory
 #include <readline/readline.h>
          ^~~~~~~~~~~~~~~~~~~~~
compilation terminated.
Makefile:57: recipe for target 'bpf_dbg.o' failed
make: *** [bpf_dbg.o] Error 1
root@VM-74-51-ubuntu:/data/linux-source-5.3.0/tools/bpf#
```
#### 解决
```sh
apt-get install libreadline-dev
```
### 问题3
#### /bin/sh: 1: bison: not found
```sh
root@VM-74-51-ubuntu:/data/linux-source-5.3.0/tools/bpf# make
  CC       bpf_dbg.o
  LINK     bpf_dbg
  CC       bpf_asm.o
  BISON    bpf_exp.yacc.c
/bin/sh: 1: bison: not found
Makefile:51: recipe for target 'bpf_exp.yacc.c' failed
make: *** [bpf_exp.yacc.c] Error 127
root@VM-74-51-ubuntu:/data/linux-source-5.3.0/tools/bpf# 
```
#### 解决
```sh
apt-get install bison
```
### 问题4
#### /bin/sh: 1: flex: not found
```sh
root@VM-74-51-ubuntu:/data/linux-source-5.3.0/tools/bpf# make
  BISON    bpf_exp.yacc.c
  CC       bpf_exp.yacc.o
  FLEX     bpf_exp.lex.c
/bin/sh: 1: flex: not found
Makefile:54: recipe for target 'bpf_exp.lex.c' failed
make: *** [bpf_exp.lex.c] Error 127
root@VM-74-51-ubuntu:/data/linux-source-5.3.0/tools/bpf# 
```
#### 解决
```sh
apt-get install flex
```
### 问题5
#### No libelf found
```sh
root@VM-74-51-ubuntu:/data/linux-source-5.3.0/tools/bpf# make
  BISON    bpf_exp.yacc.c
  CC       bpf_exp.yacc.o
  FLEX     bpf_exp.lex.c
  ......
Auto-detecting system features:
...                        libelf: [ OFF ]
...                           bpf: [ on  ]

No libelf found
Makefile:269: recipe for target 'elfdep' failed
make[2]: *** [elfdep] Error 1
Makefile:30: recipe for target '/data/linux-source-5.3.0/tools/lib/bpf/libbpf.a' failed
make[1]: *** [/data/linux-source-5.3.0/tools/lib/bpf/libbpf.a] Error 2
Makefile:99: recipe for target 'bpftool' failed
make: *** [bpftool] Error 2
root@VM-74-51-ubuntu:/data/linux-source-5.3.0/tools/bpf# 
```
#### 解决
```sh
apt-get install libelf-dev
```
## ebpf 编译
### 编译环境安装
clang 安装，这里为了未来还可以编译 envoy，所以安装了 clang-10。
```
apt-get install clang-10
```

<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)
</center>