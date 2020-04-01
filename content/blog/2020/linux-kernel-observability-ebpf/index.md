---
title: "【译】基于 eBPF 的 Linux 可观测性"
subtitle: "基于 eBPF 的 Linux 可观测性"
description: "基于 eBPF 的 Linux 可观测性"
date: 2020-03-30T20:14:34+08:00
tags: ["kernel", "ebpf"]
categories: ["kernel"]
banner: "img/banners/linux_ebpf_support.png"
author: "helight"
authorlink: "http://helight.info"
summary: "周五下午在公司的服务网格月度讨论会上，一位同事为大家分享了在服务网格中使用 ebpf 来优化提升 istio 中 sidecar 和 RS 间的通信效率。听过之后手痒难，想测试一把 ebpf。"
keywords: ["kernel","ebpf", "go", "trace"]
draft: true
---

最近发布的 Linux 内核带了一个针对内核的能力强大的 Linux 监控框架。它起源于历史上人们所说的的 [BPF](https://en.wikipedia.org/wiki/Berkeley_Packet_Filter)。

## BPF 是什么?
BPF (Berkeley Packet Filter) 是一个非常高效的网络包过滤机制，它的目标是避免不必要的用户空间申请。它直接在内核空间处理网络数据包。 BPF 支持的最常见的应用就是 **tcpdump** 工具中使用的过滤器表达式。在 tcpdump 中，表达式被编译转换为 BPF 的字节码。内核加载这些字节码并且用在原始网络包流中，以此来高效的把符合过滤条件的数据包发送到用户空间。

## eBPF 又是什么?
eBPF 是对 Linux 观测系统 BPF 的扩展和加强版本。可以把它看作是 BPF 的同类。有了 eBPF 就可以自定义沙盒中的字节码，这个沙盒是 eBPF 在内核中提供的，可以在内核中安全的执行几乎所有内核符号表抛出的函数，而不用担心搞坏内核。实际上，eBPF 也是加强了在和用户空间交互的安全性。在内核中的检测器会拒绝加载引用了无效指针的字节码或者是以达到最大栈大小限制。循环也是不允许的（除非在编译时就知道是有常数上线的循环），字节码只能够调用一小部分指定的 eBPF 帮助函数。eBPF 程序保证能及时终止，避免耗尽系统资源，而这种情况出现在内核模块执行中，内核模块会造成内核的不稳定和可怕的内核奔溃。相反的，你可能会发现和内核模块提供的自由度来比，eBPF有太多限制了，但是综合考虑下来还是更倾向于 eBPF，而不是面向模块的代码，主要是基于授权后的 eBPF 不会对内核造成损害。然而这还不是它唯一的优势。

## 为什么用 eBPF 来做 Linux 监控？
作为 Linux 内核核心的一部分，eBPF 不依赖于任何第三方模块或者扩展依赖。它包含了稳定的 **ABI**（应用程序二进制接口），可以让在老内核上编译的程序在新内核上运行。由 eBPF 带来的性能开销通常可以忽略不计，这让它非常适合做[应用监控](https://sematext.com/application-monitoring/)和跟踪很重的系统执行。窗口用户没有 eBPF，但是他们可以使用[窗口事件跟踪](https://sematext.com/blog/fibratus-windows-kernel-logging/)。

eBPF 是非常灵活而且可以跟踪几乎所有的主要内核子系统：涵盖了 CPU 调度，内存管理，网络，系统调用，块设备请求等等。而且仍然在扩展中。

可以在终端里运行下面的命令看到所有能用 eBPF 跟踪的内核符号列表：
```sh
$ cat /proc/kallsyms
```
![](./kallsyms.png)
可以跟踪的符号

The above command will produce a huge output. If we were only interested in instrumenting syscall interface, a bit of grep magic will help filter out unwanted symbol names:

```sh
$ cat /proc/kallsyms | grep -w -E “sys.*”
ffffffffb502dd20 T sys_arch_prctl
ffffffffb502e660 T sys_rt_sigreturn
ffffffffb5031100 T sys_ioperm
ffffffffb50313b0 T sys_iopl
ffffffffb50329b0 T sys_modify_ldt
ffffffffb5033850 T sys_mmap
ffffffffb503d6e0 T sys_set_thread_area
ffffffffb503d7a0 T sys_get_thread_area
ffffffffb5080670 T sys_set_tid_address
ffffffffb5080b70 T sys_fork
ffffffffb5080ba0 T sys_vfork
ffffffffb5080bd0 T sys_clone
```
不同类型的钩子点负责对不同内核模块触发的事件作出响应。内核程序运行在指定的内存地址上，网络数据包的流入或者用户空间代码的调用执行都是可以通过 eBPF 程序跟踪的，通过给 **kprobes**，**XDP** 下发 eBPF 可以跟踪进入的网络包，给 **uprobes** 下发 eBPF 可以跟踪用户空间程序调用。

在 Sematext（是一家公司，本文就是这家公司博客上的一篇文章），他们对 eBPF 非常痴迷，想尽办法挖掘 eBPF 的能力，用于服务监控和容器可视化。他们也在招这方面的人才。如果有兴趣可以试试。

下面深入介绍一下eBPF 程序如何构建并加载到内核中的。

## Linux eBPF程序剖析
在进一步分析 eBPF 程序的结构之前，有必要说一下 [BCC](https://github.com/iovisor/bcc)（BPF 编译器），这是一个工具集，用于编译 eBPF 需要的字节码，并且提供了 Python 和 Lua 的绑定支持，可以把代码加载到内核并，和底层的 eBPF 设施交互执行。它还包含了许多有用的工具，让你可以了解来用 eBPF 可以做那些事情。

在过去，BPF 程序是通过手工组合原始 BPF 指令集的方式生成的。幸运的是，[clang](https://clang.llvm.org/) （是 LLVM 前端的一部分）可以把 C 语言转换成为 eBPF 字节码，这就省去了我们自己处理 BPF 指令的麻烦了。如今，它也是唯一能够生成 eBPF 字节码的编译器，虽然也可以通过 [Rust 生成 eBPF 字节码](https://unhandledexpression.com/general/rust/2018/02/02/poc-compiling-to-ebpf-from-rust.html)。

一旦成功编译了 eBPF 程序，并且生成了目标文件，我们就可以准备注入到内核中了。为了实现注入，内核引入了一个新的 bpf 系统调用。除了加载 eBPF 字节码，这个看起来简单的系统调用还早了很多其它的事情。它创建和操控内核中的 maps（后面会详细介绍，一个非常重要的数据结构），map 是 eBPF 指令中非常高级的特性。你可以从 bpf 的帮助手册上了解跟多相关说明（man 2 bpf）。

When user space process decides to push eBPF bytecode by invoking bpf syscall, the kernel will verify it and after that will JIT (translate to machine code) the instructions to equivalent target architecture instruction set. The resulting code will be quite fast! If for any reasons the JIT compiler is not available, the kernel will fall back to the interpreter that doesn’t enjoy aforementioned bare-metal performance.

## Linux eBPF 例子
Let’s see an example of a Linux eBPF program now. Our goal is to trap the invocation to setns system call. Processes call this syscall when they wish to join a new isolated namespace that’s created after child process’s descriptor is conceived (the child process can control which namespaces it should unlink from the parent by specifying a bit mask of flags in the clone syscall argument). This system call is very often used to provide processes a segregated overview of system resources such as TCP stack, mount points, PID number space, etc.

```sh
#include <linux/kconfig.h>
#include <linux/sched.h>
#include <linux/version.h>
#include <linux/bpf.h>
#ifndef SEC
#define SEC(NAME)                  
  __attribute__((section(NAME), used))
#endif
SEC("kprobe/sys_setns")
int kprobe__sys_setns(struct pt_regs *ctx) {
   return 0;
}
char _license[] SEC("license") = "GPL";
__u32 _version SEC("version") = 0xFFFFFFFE;
```

The above is the **bare minimum eBPF program**. It consists of different segments. First, we include various kernel header files which contain definitions for multiple data types. We also declare the SEC macro that’s used to generate sections inside object file that are later interpreted by ELF BPF loader. The loader will complain if it can’t find license and version sections so we need to provide both of them.

Now comes the most interesting part of our eBPF program – the actual hook point for the setns syscall. By starting the function name with kprobe__ prefix and binding the corresponding SEC macro we instruct the in-kernel virtual machine to attach instrumentation callback to sys_setns symbol that will trigger our eBPF program and execute the code inside the function’s body each time syscall is dispatched. Every eBPF program has a context. In the case of kernel probes, that’s the current state of the processor’s registers (pt_regs structure) that contain function arguments as placed by libc upon transition from user to kernel space. To compile the program (llvm and clang should be installed and properly configured) we can use the following command (please note you’ll need to specify the path to kernel headers through LINUX_HEADERS env variable) where clang will emit an intermediate LLVM representation of our program and LLVM compiler will produce the final eBPF bytecode:

```sh
$ clang -D__KERNEL__ -D__ASM_SYSREG_H
         -Wunused
         -Wall
         -Wno-compare-distinct-pointer-types
         -Wno-pointer-sign
         -O2 -S -emit-llvm ns.c
         -I $LINUX_HEADERS/source/include
         -I $LINUX_HEADERS/source/include/generated/uapi
         -I $LINUX_HEADERS/source/arch/x86/include
         -I $LINUX_HEADERS/build/include
         -I $LINUX_HEADERS/build/arch/x86/include
         -I $LINUX_HEADERS/build/include/uapi
         -I $LINUX_HEADERS/build/include/generated/uapi
         -I $LINUX_HEADERS/build/arch/x86/include/generated
         -o - | llc -march=bpf -filetype=obj -o ns.o
```
You can use readelf tool to introspect ELF sections and the symbol table of the object file:
```sh
$ readelf -a -S ns.o
…
 2: 0000000000000000         0 NOTYPE  GLOBAL DEFAULT        4 _license
 3: 0000000000000000         0 NOTYPE  GLOBAL DEFAULT        5 _version
 4: 0000000000000000         0 NOTYPE  GLOBAL DEFAULT        3 kprobe__sys_setns
```
The above output proves that the symbol table is built up as expected. We have a valid eBPF object file and now it’s the time to load it into the kernel and see the magic happen.

## 使用 Go 语言给内核下发 eBPF 程序
We already mentioned **BCC** and how it does the heavy lifting while offering an ergonomic interface to the eBPF machinery. In order to build and run eBPF programs BCC requires installing LLVM and kernel headers on the target node and sometimes we might not have the luxury to make that tradeoff. In such scenarios it would be ideal if we could ship the resulting ELF object baked in the data segment of our binary and maximize the portability across machines.

Apart from providing bindings for libbcc, [gobpf](https://github.com/iovisor/gobpf) package has the ability to load eBPF programs from precompiled bytecode. If we combine it with a tool such as [packr](https://github.com/gobuffalo/packr) which can embed blobs in a Go app we have all needed ingredients to distribute our binary with zero runtime dependencies.

We’ll slightly modify the eBPF program so it prints to the kernel tracing pipe when kprobe is triggered. For brevity, we won’t include the definition of printt macro as well as other eBPF helper functions, but you can find them in [this](https://github.com/cilium/cilium/blob/master/bpf/include/bpf/api.h) header file.

```sh
SEC("kprobe/sys_setns")
int kprobe__sys_setns(struct pt_regs *ctx) {
  int fd = (int)PT_REGS_PARM1(ctx);
  int pid = bpf_get_current_pid_tgid() >> 32;
  printt("process with pid %d joined ns through fd %d", pid, fd);
  return 0;
}
```
Now we can start writing Go code that handles eBPF bytecode loading. We will implement a tiny abstraction (KprobeTracer) atop gobpf:

```sh
import (
  "bytes"
  "errors"
  "fmt"

  bpflib "github.com/iovisor/gobpf/elf"
)

type KprobeTracer struct {
  // bytecode is the byte stream with embedded eBPF program
  bytecode []byte

  // eBPF module associated with this tracer. The module is a  collection of maps, probes, etc.
  mod *bpflib.Module
}

func NewKprobeTracer(bytecode []byte) (*KprobeTracer, error) {
   mod := bpflib.NewModuleFromReader(bytes.NewReader(bytecode))
   if mod == nil {
      return nil, errors.New("ebpf is not supported")
   }
   return KprobeTracer{mod: mod, bytecode: bytecode}, nil
}

// EnableAllKprobes enables all kprobes/kretprobes in the module. The argument
// determines the maximum number of instances of the probed functions the can
// be handled simultaneously.
func (t *KprobeTracer) EnableAllKprobes(maxActive int) error {

  params := make(map[string]*bpflib.PerfMap)

  err := t.mod.Load(params)
  if err != nil {
     return fmt.Errorf("unable to load module: %v", err)
  }

  err = t.mod.EnableKprobes(maxActive)
  if err != nil {
     return fmt.Errorf("cannot initialize kprobes: %v", err)
  }
  return nil
}
```
We are ready to bootstrap the kernel probe tracer:
```sh
package main

import (
  "log"
  "github.com/gobuffalo/packr"
)

func main() {
  box := packr.NewBox("/directory/to/your/object/files")
  bytecode, err := box.Find("ns.o")
  if err != nil {
      log.Fatal(err)
  }

  ktracer, err := NewKprobeTracer(bytecode)

  if err != nil {
     log.Fatal(err)
  }

  if err := ktracer.EnableAllKprobes(10); err != nil {
     log.Fatal(err)
  }
}
```
Use sudo cat /sys/kernel/debug/tracing/trace_pipe to read debug info pushed to the pipe. The easiest way to test eBPF program is by attaching it to the running Docker container:
```sh
$ docker exec -it nginx /bin/bash
```
Behind the scenes, the container runtime will re-associate the bash process to the namespace of the nginx container. The first argument we captured via PT_REGS_PARM1 macro is the file descriptor of the namespace that’s represented with symbolic link inside /proc/<pid>/ns directory. Yay! So we can monitor each time a process joins the namespace. It might not be something super useful, but it illustrates how easy it is to trap syscall’s execution and have access to its arguments.

## 使用 eBPF Maps
Writing results to the tracing pipe is good for debugging purposes, but for production environments we’ll definitely need a more sophisticated mechanism for sharing state between user and kernel spaces. That’s where eBPF maps come to the rescue. They represent a very efficient in-kernel key / value stores for data aggregation and can be accessed asynchronously from user space. There are many types of eBPF maps, but for this particular use case we will rely on BPF_MAP_TYPE_PERF_EVENT_ARRAY map. It can store custom structures that are pushed via perf event ring buffer and broadcasted to user space process.

Go-bpf allows for perf map creation and event streaming to provided Go channel. We can add the following code to transmit C structures to our program.

```sh
rxChan := make(chan []byte)
lostChan := make(chan uint64)
pmap, err := bpflib.InitPerfMap(
  t.mod,
  mapName,
  rxChan,
  lostChan,
)

if err != nil {
  return quit, err
}

if _, found := t.maps[mapName]; !found {
  t.maps[mapName] = pmap
}

go func() {
  for {
     select {

     case pe := <-rxChan:
        nsJoin := (*C.struct_ns_evt_t)(unsafe.Pointer(&(*pe)[0]))
        log.Info(nsJoin)

     case l := <-lostChan:
        if lost != nil {
           lost(l)
        }
     }
  }
}()

pmap.PollStart()
```
We initialize receiver and lost event channels and pass them to InitPerfMap function along with the module reference and the name of the perf map we are supposed to consume events from. Each time new event is pushed on the receiver channel, we cast the raw pointer to C struct (ns_evt_t) as defined in our eBPF program. We also need to declare a perf map and emit the structure through bpf_perf_event_output helper:

```sh
struct bpf_map_def SEC("maps/ns") ns_map = {
  .type = BPF_MAP_TYPE_HASH,
  .key_size = sizeof(u32),
  .value_size = sizeof(struct ns_evt_t),
  .max_entries = 1024,
  .pinning = 0,
  .namespace = "",
};

struct ns_evt_t evt = {};

/* Initialize structure fields.*/
u32 cpu = bpf_get_smp_processor_id();
bpf_perf_event_output(ctx, &ns_map,
                     cpu,
                     &evt, sizeof(evt));
```
## 结论
eBPF is constantly evolving and getting wider adoption. With each kernel release new features and improvements are being addressed. Low overhead and native programmability support makes it very attractive for a variety of use cases. For example, [Suricata](https://suricata-ids.org/) intrusion detection system uses it for [implementing](https://github.com/OISF/suricata/pull/3193) advanced socket load balancing strategies and packet filtering at the very early stage in the Linux network stack. [Cilium](https://github.com/cilium/cilium) relies heavily on eBPF for delivering sophisticated security policies for containers.  [Sematext Agent](https://sematext.com/blog/streamlined-kubernetes-cluster-agent/) leverages eBPF for pinpointing interesting events such as kill signals broadcasting or OOM notifications for [Docker and Kubernetes monitoring](https://sematext.com/docker), as well as regular [server monitoring](https://sematext.com/spm). It also provides an efficient network tracer for [network monitoring](https://sematext.com/network-monitoring) by using eBPF for capturing TCP/UDP traffic statistics. It seems eBPF is aiming to become a de facto standard for Linux monitoring via Linux kernel instrumentation.

## Next Steps
If you find this stuff exciting, we’re looking for people like you – we work on performance monitoring, log management, transaction tracing, and other forms of observability and utilize things like Go, Kotlin, Node.js, Kubernetes, Kafka, Elasticsearch, Akka, eBPF of course, etc.

Looking for a Full Stack Observability platform? We suggest checking out Sematext Cloud which brings together logs, metrics, user monitoring and tracing. Want instead only tracing solution? Sematext Tracing provides end to end visibility into your distributed applications so you can find bottlenecks quickly, resolve production issues faster and with less effort. Interested in trying it out? Sign up today for a free exclusive beta invite by clicking here.

We would love to hear your opinions, experiments or anything else you would like to share regarding eBPF ecosystem. Before you go, don’t forget to download your OpenTracing eBook: [Distributed Tracing’s Emerging Industry Standard](https://sematext.com/opentracing-emerging-standard-for-distributed-tracing-ebook/).

--------------------------------------------------------------------------------


原文：[Linux Observability with eBPF](https://sematext.com/blog/linux-kernel-observability-ebpf/)

作者：[Nedim Šabić](https://sematext.com/blog/author/nedims/)

原文发表时间：2019 年 3 月 4 号

译者：[helight](https://github.com/helight)

校对：[helight](https://github.com/helight)
