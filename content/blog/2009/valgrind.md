+++
title = "使用valgrind检测内存泄露"
date = "2009-02-27T13:47:08+02:00"
tags = ["valgrind", "tools", "linux"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "Valgrind是x86架构Linux上的多重用途代码剖析和内存调试工具。但它的主要功能还是对内存的调试，默认工具也是启动 memcheck。用户可以在它的环境中运行程序来监视内存的使用情况，比如C语言中的malloc和free或者C++中的new和delete。"
keywords = ["Valgrind", "c++", "malloc"]
+++

# 介绍
Valgrind是x86架构Linux上的多重用途代码剖析和内存调试工具。但它的主要功能还是对内存的调试，默认工具也是启动 memcheck。用户可以在它的环境中运行程序来监视内存的使用情况，比如C语言中的malloc和free或者C++中的new和delete。

Valgrind主要检测如下内存问题：
* （1）使用未初始化的内存。
* （2）使用已经释放了的内存。
* （3）使用超过malloc分配的内存空间。
* （4）对堆栈的非法访问。
* （5）内存泄漏。
* （6）malloc/free/new/delete非匹配的使用内存申请和释放函数。
* （7）使用memcpy等函数时源地址和目的地址的重叠错误。

# 示例程序1：
``` c
#include <stdio.h>
#include <stdlib.h>

int main() {
    struct mm{
        int a;
        int b;
    }kk,*p;
    kk.a = 10;
    printf("kk.a : %d \n",kk.a);
    p = (struct mm *)malloc(sizeof(struct mm));
    p->a = 9;
    printf("p->a : %d \n",p->a);
    return 0;
}
```

## 编译源码：
``` sh
helight@helight:test$ vim hello.c
helight@helight:test$ gcc -g -o hello hello.c
helight@helight:test$
```
## 使用valgrind检测内存问题：
``` sh
helight@helight:test$ valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./hello
==6448== Memcheck, a memory error detector. 。。。
==6448== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 13 from 1)
==6448== malloc/free: in use at exit: 8 bytes in 1 blocks.
==6448== malloc/free: 1 allocs, 0 frees, 8 bytes allocated.
==6448== For counts of detected errors, rerun with: -v
==6448== searching for pointers to 1 not-freed blocks.
==6448== checked 59,964 bytes.
==6448==
==6448== 8 bytes in 1 blocks are definitely lost in loss record 1 of 1
==6448==    at 0x4023D6E: malloc (vg_replace_malloc.c:207)
==6448==    by 0x804840A: main (hello.c:12)
==6448== ==6448== LEAK SUMMARY:
==6448==    definitely lost: 8 bytes in 1 blocks.
==6448==      possibly lost: 0 bytes in 0 blocks.
==6448==    still reachable: 0 bytes in 0 blocks.
==6448==         suppressed: 0 bytes in 0 blocks.
helight@helight:test$
```
可以看出上面提示“malloc/free: 1 allocs, 0 frees, 8 bytes allocated.”，“definitely lost: 8 bytes in 1 blocks.”。即丢失了8个字节。

# 实例程序2：

``` c
#include <stdio.h>
#include <stdlib.h>
int main() {
    struct mm{
        int a;
        int b;
    }kk;
    printf("kk.a : %d \n",kk.a);
    return 0;
}
```

## 编译源码：
``` sh
helight@helight:test$ vim hello.c
helight@helight:test$ gcc -g -o hello hello.c
helight@helight:test$
```
## 使用valgrind检测内存问题：
``` sh
helight@helight:test$ valgrind --tool=memcheck --leak-check=yes --show-reachable=yes ./hello
==6635== Memcheck, a memory error detector.
==6635== Copyright (C) 2002-2007, and GNU GPL'd, by Julian Seward et al.
==6635== Using LibVEX rev 1854, a library for dynamic binary translation.
==6635== Copyright (C) 2004-2007, and GNU GPL'd, by OpenWorks LLP.
==6635== Using valgrind-3.3.1-Debian, a dynamic binary instrumentation framework.
==6635== Copyright (C) 2000-2007, and GNU GPL'd, by Julian Seward et al.
==6635== For more details, rerun with: -v
==6635==
==6635== Use of uninitialised value of size 4
==6635==    at 0x40734C6: (within /lib/i686/cmov/libc-2.7.so)
==6635==    by 0x4076CA3: vfprintf (in /lib/i686/cmov/libc-2.7.so)
==6635==    by 0x407E46F: printf (in /lib/i686/cmov/libc-2.7.so)
==6635==    by 0x80483C7: main (hello.c:10) ... kk.a : 134513673
==6635==
==6635== ERROR SUMMARY: 21 errors from 5 contexts (suppressed: 13 from 1)
==6635== malloc/free: in use at exit: 0 bytes in 0 blocks.
==6635== malloc/free: 0 allocs, 0 frees, 0 bytes allocated.
==6635== For counts of detected errors, rerun with: -v
==6635== All heap blocks were freed -- no leaks are possible.
helight@helight:test$
```
可以看出上面提示“Use of uninitialised value of size 4“，”by 0x80483C7: main (hello.c:10) ”。即使用了未初始化的值。


<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
