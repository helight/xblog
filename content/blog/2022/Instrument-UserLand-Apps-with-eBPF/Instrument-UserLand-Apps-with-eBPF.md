---
title: "如何使用 eBPF 检测分析用户态程序"
date: 2022-10-25T08:47:20+08:00
tags: ["ebpf"]
categories: ["ebpf",]
banner: "img/banners/ebpf.png"
author: "helight"
authorlink: "http://helight.cn"
summary: ""
keywords: ["ebpf", "go"]
draft: false
---

eBPF 彻底改变了 Linux 内核中的可观察性。在我之前的系列文章中，我介绍了[eBPF 生态系统的基本构建模块](http://www.helight.cn/blog/2020/linux-kernel-observability-ebpf/)，简要介绍了[XDP](https://sematext.com/blog/ebpf-and-xdp-for-processing-packets-at-bare-metal-speed/)，并展示了它与 eBPF 基础设施如何密切合作，以便在网络堆栈中引入一个快速处理的数据路径。

然而，eBPF 并不只是用在内核空间跟踪。如果我们可以在生产环境中运行的应用程序上也能享受 eBPF 驱动的跟踪的，这是不是很好呢?

这就是 _uprobes_ 发挥作用的地方。可以将它们看作是一种 _kprobes_ ，它加载到了用户空间跟踪点而不是内核符号。多语言运行时、数据库系统和其他软件栈都包含了可以被 [BCC](https://github.com/iovisor/bcc/blob/master/tools/lib/ustat.py) 工具使用的钩子。具体地说，_ustat_ 工具收集了大量有用事件，如垃圾收集事件、对象创建统计信息、方法调用等。


但是“官方”语言运行时的版本，如 Node.js 和 Python，不带 DTrace 支持，这就需要你必须从源代码构建，将 --with-dtrace_ 标志传递给编译器。这不是说必须一定要本机编译语言。只要符号表可用，就可以对二进制文本段中出现的任何符号应用动态跟踪。在运行的二进制文件上检测 Go 或 Rust stdlib 函数调用就是通过这种方式完成的。

## 可用于检测分析应用程序的 eBPF 技术

跟踪用户空间进程有多种方法：

-  静态声明的 USDT
-  动态声明的 USDT
-  使用 uprobes 进行动态跟踪

### 静态声明的 USDT

USDT (Userland Statically Defined Tracing)  的做法是直接在用户代码中嵌入探测。该技术的起源可以追溯到 Solaris/BSD DTrace 时代，包括使用 _DTRACE_PROBE()_ 宏在重要代码位置上声明跟踪点。与常规符号不同，USDT 钩子保证即使代码被重构也能保持稳定。下图描述了在用户代码中声明 USDT 跟踪点的过程，直到在内核中执行为止。

![uprobe](Instrument-UserLand-Apps-with-eBPF/imgs/1.png)


开发人员可以先通过 _DTRACE_PROBE_ 和 _DTRACE_PROBE1_ 宏来在需要的代码块中植入跟踪点。两个宏都接受两个强制参数，如提供者/探测名称，后面跟着你希望从跟踪点查询的任何值。编译器将把USDT 跟踪点塞进目标二进制文件 ELF 段中 。编译器和跟踪工具之间规定了 USDT 元数据所在的位置必须存在 _.note.stapstd_ 段。

USDT 跟踪工具检查 ELF 段，并在被转为 _int3_ 中断的跟踪点位置上放置一个断点。每当在跟踪点的标记处执行时，就会触发中断处理程序，并在内核中调用与 _uprobe_ 关联的程序来处理事件并将它们广播到用户空间，执行映射聚合等等。

### 动态声明的 USDT

由于 USDT 被加入静态生成的 ELF 段，所以USDT不能运行在解释性语言或基于 jit 的语言上的软件上。幸运的是，可以通过 _libstapsdt_ 在运行时中定义跟踪点。它生成一个带有 USDT 信息的小型共享对象，该共享对象被映射到进程的地址空间，因此跟踪工具可以附加到目标跟踪点上。在许多语言中都有 _libstapsdt_ 。要了解如何在 Node.js 中安装 USDT 探测可以参考这个 [example](https://github.com/sthima/node-usdt#example)。

### 使用 uprobes 进行动态跟踪


这种类型的跟踪机制不需要目标进程提供任何额外的功能，只需要它的符号表是可访问的。这是最通用和最强大的插装方法，因为它允许在任意指令上注入断点，甚至不需要重启运行的进程。

### 跟踪例子

After a brief theoretical introduction, let’s see some concrete examples on how to instrument real-world apps crafted in diverse languages.
在简单的理论介绍之后，我们来看一些具体的例子，看看如何用跟踪分析不同的语言的应用程序。

**C 语言**

Redis is a popular key-value data structures server built in C. Taking a sneak peek into the Redis symbol table reveals a vast number of functions that are candidates for capturing via uprobes.
Redis 是一个用 C 语言开发的非常流行的 k-v 数据库服务，仔细看一下 Redis 符号表，就会发现大量可以通过 uprobes 捕获的函数。

``` sh
$ objdump -tT /usr/bin/redis-server
…
000000000004c160 g    DF .text  00000000000000cc  Base 
addReplyDouble
0000000000090940 g    DF .text  00000000000000b0  Base        sha1hex
00000000000586e0 g    DF .text  000000000000007c  Base        
replicationSetMaster
00000000001b39e0 g    DO .data  0000000000000030  Base        
dbDictType
00000000000ace20 g    DF .text  0000000000000030  Base        
RM_DictGetC
0000000000041bc0 g    DF .text  0000000000000073  Base        
sdsull2str
00000000000bba00 g    DF .text  0000000000000871  Base        raxSeek
00000000000ac8c0 g    DF .text  000000000000000c  Base        
RM_ThreadSafeContextUnlock
00000000000e3900 g    DF .text  0000000000000059  Base        
mp_encode_lua_string
00000000001cef60 g    DO .bss   0000000000000438  Base        rdbstate
0000000000047110 g    DF .text  00000000000000b5  Base        
zipSaveInteger
000000000009f5a0 g    DF .text  0000000000000055  Base        
addReplyDictOfRedisInstances
0000000000069200 g    DF .text  000000000000004a  Base        
zzlDelete
0000000000041e90 g    DF .text  00000000000008ba  Base        
sdscatfmt
000000000009ac40 g    DF .text  000000000000003a  Base        
sentinelLinkEstablishedCallback
00000000000619d0 g    DF .text  0000000000000045  Base        
psetexCommand
00000000000d92f0 g    DF .text  00000000000000fc  Base        
luaL_argerror
00000000000bc360 g    DF .text  0000000000000328  Base        
raxRandomWalk
0000000000096a00 g    DF .text  00000000000000c3  Base        
rioInitWithFdset
000000000003d160 g    DF .text  0000000000000882  Base        
serverCron
0000000000032907 g    DF .text  0000000000000000  Base        
je_prof_thread_name_set
0000000000043960 g    DF .text  0000000000000031  Base        zfree
00000000000a2a40 g    DF .text  00000000000001ab  Base        
sentinelFailoverDetectEnd
00000000001b8500 g    DO .data  0000000000000028  Base        
je_percpu_arena_mode_names
00000000000b5f90 g    DF .text  0000000000000018  Base        
geohashEstimateStepsByRadius
00000000000d95e0 g    DF .text  0000000000000039  Base        
luaL_checkany
0000000000048850 g    DF .text  00000000000002d4  Base        
createClient
...
```

有一个有趣的 _createStringObject_ 函数，Redis 内部利用它来分配围绕 _robj_ 结构建模的字符串。Redis 命令通过 _createStringObject_ 调用来执行。通过跟踪这个函数，我们可以监视发送到 Redis 服务器的任何命令。为此，我将使用 BCC 工具包中的 _trace_ 功能。

``` sh
$ /usr/share/bcc/tools/trace '/usr/bin/redis-server:createStringObject "%s" arg1'
PID     TID     COMM            FUNC             -
8984    8984    redis-server    createStringObject b'COMMANDrn'
8984    8984    redis-server    createStringObject 
b'setrn$4rnoctirn$4rnfestrn'
8984    8984    redis-server    createStringObject b'octirn$4rnfestrn'
8984    8984    redis-server    createStringObject b'festrn'
8984    8984    redis-server    createStringObject b'getrn$4rnoctirn'
8984    8984    redis-server    createStringObject b'octirn'
```

以上是在 Redis CLI 客户端执行 "set octi fest" 和 "get octi" 的输出结果。

**Java语言**

现代 JVM 版本自带对 USDT 的内置支持。所有探针都是用 _libjvm_ 共享对象带来的。我们可以在 ELF 段中到可用的追踪点。

``` sh
$ readelf -n /usr/lib/jvm/jdk-11-oracle/lib/server/libjvm.so
...
stapsdt              0x00000037       NT_STAPSDT (SystemTap probe 
descriptors)
  Provider: hs_private
  Name: cms__initmark__end
  Location: 0x0000000000e2420c, Base: 0x0000000000f725b4, Semaphore: 0x0000000000000000
  Arguments:
stapsdt              0x00000037       NT_STAPSDT (SystemTap probe descriptors)
  Provider: hs_private
  Name: cms__remark__begin
  Location: 0x0000000000e24334, Base: 0x0000000000f725b4, Semaphore: 0x0000000000000000
  Arguments:
stapsdt              0x00000035       NT_STAPSDT (SystemTap probe descriptors)
  Provider: hs_private
  Name: cms__remark__end
  Location: 0x0000000000e24418, Base: 0x0000000000f725b4, Semaphore: 0x0000000000000000
  Arguments:
stapsdt              0x0000002f       NT_STAPSDT (SystemTap probe descriptors)
  Provider: hotspot
  Name: gc__begin
  Location: 0x0000000000e2b262, Base: 0x0000000000f725b4, Semaphore: 0x0000000000000000
  Arguments: 1@$1
stapsdt              0x00000029       NT_STAPSDT (SystemTap probe descriptors)
  Provider: hotspot
  Name: gc__end
  Location: 0x0000000000e2b31a, Base: 0x0000000000f725b4, Semaphore: 0x0000000000000000
  Arguments:
...
```

为了捕获所有的类加载事件，我们可以使用以下命令：

``` sh
$ /usr/share/bcc/tools/trace 
'u:/usr/lib/jvm/jdk-11-oracle/lib/server/libjvm.so:class__loaded "%s", arg1'
```

类似的，我们可以观察线程创建事件：

``` sh
$ /usr/share/bcc/tools/trace 
'u:/usr/lib/jvm/jdk-11-oracle/lib/server/libjvm.so:thread__start "%s", arg1'
```

``` sh
PID     TID     COMM            FUNC
27390   27398   java            thread__start    b'Reference Handler'
27390   27399   java            thread__start    b'Finalizer'
27390   27400   java            thread__start    b'Signal Dispatcher'
27390   27401   java            thread__start    b'C2 CompilerThread0'
27390   27402   java            thread__start    b'C1 CompilerThread0'
27390   27403   java            thread__start    b'Sweeper thread'
27390   27404   java            thread__start    b'Service Thread'
```

当扩展探测通过 _-XX:+ExtendedDTraceProbes_ 属性启用时，_uflow 工具_ 能够实时跟踪和绘制所有方法执行的图形。

``` sh
$ /usr/share/bcc/tools/lib/uflow -l java 27965
Tracing method calls in java process 27965... Ctrl-C to quit.
CPU PID    TID    TIME(us) METHOD
5   27965  27991  0.736    <- jdk/internal/misc/Unsafe.park
5   27965  27991  0.736    -> 
java/util/concurrent/locks/LockSupport.setBlocker'
5   27965  27991  0.736      -> jdk/internal/misc/Unsafe.putObject
5   27965  27991  0.736      <- jdk/internal/misc/Unsafe.putObject
5   27965  27991  0.736    <- 
java/util/concurrent/locks/LockSupport.setBlocker'
5   27965  27991  0.736    <- 
java/util/concurrent/locks/LockSupport.parkNanos
5   27965  27991  0.736    -> 
java/util/concurrent/locks/AbstractQueuedSynchronizer$ConditionObject.checkInterruptWhileWaiting
5   27965  27991  0.737      -> java/lang/Thread.interrupted
5   27965  27991  0.737        -> java/lang/Thread.isInterrupted
5   27965  27991  0.737        <- java/lang/Thread.isInterrupted
5   27965  27991  0.737      <- java/lang/Thread.interrupted
5   27965  27991  0.737    <- 
java/util/concurrent/locks/AbstractQueuedSynchronizer$ConditionObject.checkInterruptWhileWaiting
5   27965  27991  0.737    -> java/lang/System.nanoTime
5   27965  27991  0.737    <- java/lang/System.nanoTime
```

但是，就扩展探针所产生的开销而言，这往往非常昂贵，因此不适合在生产环境中调试。

**Go语言**

我将用一个 Go 中的例子来结束跟踪技术的演示。因为 Go 是一种原生编译语言，所以使用跟踪工具将 _uprobe_ 程序附加到目标符号上是尝试性的。你可以用下面这个简单的代码片段自己尝试一下:

``` go
package main

import "fmt"

func main() {
  fmt.Println("Hi")
}
```
``` sh
$ go build -o hi build.go

$/usr/share/bcc/tools/trace '/tmp/hi:fmt.Println "%s" arg1'
PID     TID     COMM            FUNC             -
31545   31545   hi              fmt.Println      b'xd6x8dK'
```

除了打印这里的 " Hi "字符串，我们在参数列中看到打印了一些随机的垃圾信息。这在一定程度上是由于 trace 不能处理 Println 可变参数造成的，但也可能和 ABI 调用约定的参数使用的错误假设有关。在 C/C++ 中，传递参数的首选方式是在常规寄存器中，而 Go 在堆栈上传递参数。

由于我们不能依赖 trace 工具来来演示如何跟踪 Go 代码，所以我将构建一个简单的工具来跟踪由  _http.Get_ 函数发出的所有 HTTP GET 请求。 你可以很容易地修改它来捕获其他 HTTP 请求动词，大家可以参与贡献。完整的源代码可以在这个 [repo](https://github.com/sematext/uprobe-http-tracer) 中找到。

我不会详细介绍 _uprobe_ 附加/加载过程，因为我们正在使用 Go 绑定 来 帮 _libbcc_ 完成复杂的工作。让我们分析一下实际的 _uprobe_ 程序。

在必需的 include 语句之后，有宏的定义，该宏通过偏移量处理的方式负责从堆栈中获取参数。

``` c
#define SP_OFFSET(offset) (void *)PT_REGS_SP(ctx) + offset * 8
```

接下来，我们声明数据结构用于封装通过 _reqs_ map流传递的事件。map 用 _BPF_PERF_OUTPUT_ 宏来定义。我们程序的核心是__uprobe_http_get 函数。当 _http.Get_ 被调用，则在内核空间中触发前面的函数。我们知道 [HTTP . get](https://golang.org/pkg/net/http/#Client.Get) 只有一个参数，表示 HTTP 请求被发送到的 URL。C 语言和 Go 语言的另一个区别是字符串内存中布局处理。

C 字符串是以空结束的序列串，而 Go 用 2 个值来描述：指向内存缓冲区的指针和字符串长度。这就解释了我们需要对 bpf_probe_read 进行两个调用——一个用于读取字符串，另一个用于提取字符串的长度。

``` c
bpf_probe_read(&url, sizeof(url), SP_OFFSET(1));
bpf_probe_read(&req.len, sizeof(req.len), SP_OFFSET(2));
```

稍后在用户空间中，URL 将从字节片裁剪到相应的长度。作为一个附加说明，我想提到的是，该工具的草案版本能够通过注入 _uretprobe_ 来检测出每个 _HTTP GET_ 请求的延迟。然而，每次 Go 运行时决定收缩/增长堆栈就会有一个灾难性的影响，因为 _uretprobes_ 补丁的返回地址是在堆栈上的跳转函数，它在 eBPF VM 的上下文中执行。在退出 _uretprobe_ 函数时，指令指针被恢复到原始的返回地址，该地址可能指向一个无效的位置，打乱堆栈并导致进程崩溃。有一些[建议](https://github.com/iovisor/bcc/issues/1320#issuecomment-407927542)来解决这个问题。

## 结论

我们的 eBPF 之旅已经走到了终点。在这最后一篇文章中，我们介绍了一些 eBPF 特性用于用户空间进程插装 。通过几个实用案例，我们展示了 BCC 框架在捕获可观察性信号方面的多功能性。最后，我们亲自动手建立了一个小工具，用于跟踪实时 Go 应用程序上的 HTTP 请求。