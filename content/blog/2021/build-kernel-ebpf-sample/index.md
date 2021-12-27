---
title: "linux5.3.0编译运行LINUX内核源码中的BPF示例代码"
date: 2021-03-31T08:45:20+08:00
tags: ["kernel", "ebpf"]
categories: ["kernel", "ebpf"]
banner: "img/banners/linux_ebpf_support.png"
author: "helight"
authorlink: "http://helight.cn"
summary: ""
keywords: ["kernel","ebpf"]
draft: false
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
make oldconfig && make prepare
make scripts
make M=samples/bpf
```
### 问题1
#### BPF_PROG_TYPE_RAW_TRACEPOINT’ undeclared
```sh
samples/bpf/bpf_load.c: In function ‘load_and_attach’:
samples/bpf/bpf_load.c:108:15: error: ‘BPF_PROG_TYPE_RAW_TRACEPOINT’ undeclared (first use in this function); did you mean ‘BPF_PROG_TYPE_TRACEPOINT’?
   prog_type = BPF_PROG_TYPE_RAW_TRACEPOINT;
               ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
               BPF_PROG_TYPE_TRACEPOINT
samples/bpf/bpf_load.c:122:15: error: ‘BPF_PROG_TYPE_SK_MSG’ undeclared (first use in this function); did you mean ‘BPF_PROG_TYPE_SK_SKB’?
   prog_type = BPF_PROG_TYPE_SK_MSG;
               ^~~~~~~~~~~~~~~~~~~~
               BPF_PROG_TYPE_SK_SKB
scripts/Makefile.host:107: recipe for target 'samples/bpf/bpf_load.o' failed
make[1]: *** [samples/bpf/bpf_load.o] Error 1
Makefile:1667: recipe for target '_module_samples/bpf' failed
make: *** [_module_samples/bpf] Error 2
root@VM-74-51-ubuntu:/data/linux-source-5.3.0# ls samples/bpf/
```
#### 解决
把内核源码文件：include/uapi/linux/bpf.h 中的定义拷贝到系统头文件：/usr/include/linux/bpf.h
拷贝内容：
```c
enum bpf_prog_type {
        BPF_PROG_TYPE_UNSPEC,
        BPF_PROG_TYPE_SOCKET_FILTER,
        BPF_PROG_TYPE_KPROBE,
        BPF_PROG_TYPE_SCHED_CLS,
        BPF_PROG_TYPE_SCHED_ACT,
        BPF_PROG_TYPE_TRACEPOINT,
        BPF_PROG_TYPE_XDP,
        BPF_PROG_TYPE_PERF_EVENT,
        BPF_PROG_TYPE_CGROUP_SKB,
        BPF_PROG_TYPE_CGROUP_SOCK,
        BPF_PROG_TYPE_LWT_IN,
        BPF_PROG_TYPE_LWT_OUT,
        BPF_PROG_TYPE_LWT_XMIT,
        BPF_PROG_TYPE_SOCK_OPS,
        BPF_PROG_TYPE_SK_SKB,
        BPF_PROG_TYPE_CGROUP_DEVICE,
        BPF_PROG_TYPE_SK_MSG,
        BPF_PROG_TYPE_RAW_TRACEPOINT,
        BPF_PROG_TYPE_CGROUP_SOCK_ADDR,
        BPF_PROG_TYPE_LWT_SEG6LOCAL,
        BPF_PROG_TYPE_LIRC_MODE2,
        BPF_PROG_TYPE_SK_REUSEPORT,
        BPF_PROG_TYPE_FLOW_DISSECTOR,
        BPF_PROG_TYPE_CGROUP_SYSCTL,
        BPF_PROG_TYPE_RAW_TRACEPOINT_WRITABLE,
        BPF_PROG_TYPE_CGROUP_SOCKOPT,
};
```

### 问题2
#### /usr/include/linux/ip.h:102:2: error: unknown type name ‘__sum16’
```sh
  LINK     libbpf.so.0.0.4
  LINK     test_libbpf
  HOSTCC  samples/bpf/test_lru_dist
  HOSTCC  samples/bpf/sock_example
In file included from samples/bpf/sock_example.c:27:0:
/usr/include/linux/ip.h:102:2: error: unknown type name ‘__sum16’
  __sum16 check;
  ^~~~~~~
scripts/Makefile.host:90: recipe for target 'samples/bpf/sock_example' failed
make[1]: *** [samples/bpf/sock_example] Error 1
Makefile:1667: recipe for target '_module_samples/bpf' failed
make: *** [_module_samples/bpf] Error 2
root@VM-74-51-ubuntu:/data/linux-source-5.3.0# 
```
#### 解决

```sh
vim tools/include/linux/types.h
```
```c
typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;

typedef __u16 __bitwise __sum16;  //这里加入
typedef __u32 __bitwise __wsum;   //这里加入

typedef struct {
        int counter;
} atomic_t;
```
### 问题3
#### error: ‘test_attr__enabled’ undeclared (first use in this function)
```sh
/tools/perf/perf-sys.h: In function ‘sys_perf_event_open’:
./tools/perf/perf-sys.h:68:15: error: ‘test_attr__enabled’ undeclared (first use in this function)
  if (unlikely(test_attr__enabled))
               ^
./tools/include/linux/compiler.h:74:43: note: in definition of macro ‘unlikely’
 # define unlikely(x)  __builtin_expect(!!(x), 0)
                                           ^
./tools/perf/perf-sys.h:68:15: note: each undeclared identifier is reported only once for each function it appears in
  if (unlikely(test_attr__enabled))
               ^
./tools/include/linux/compiler.h:74:43: note: in definition of macro ‘unlikely’
 # define unlikely(x)  __builtin_expect(!!(x), 0)
                                           ^
In file included from samples/bpf/bpf_load.c:28:0:
./tools/perf/perf-sys.h:69:3: warning: implicit declaration of function ‘test_attr__open’ [-Wimplicit-function-declaration]
   test_attr__open(attr, pid, cpu, fd, group_fd, flags);
   ^~~~~~~~~~~~~~~
scripts/Makefile.host:107: recipe for target 'samples/bpf/bpf_load.o' failed
make[1]: *** [samples/bpf/bpf_load.o] Error 1
Makefile:1667: recipe for target '_module_samples/bpf' failed
make: *** [_module_samples/bpf] Error 2
```
#### 解决
```sh
vim ./tools/perf/perf-sys.h
```
编辑这个文件，注视相关代码。
```c
static inline int
sys_perf_event_open(struct perf_event_attr *attr,
                      pid_t pid, int cpu, int group_fd,
                      unsigned long flags)
{
        int fd;

        fd = syscall(__NR_perf_event_open, attr, pid, cpu,
                     group_fd, flags);
// 注视掉以下代码
// #ifdef HAVE_ATTR_TEST
//      if (unlikely(test_attr__enabled)) // 
//              test_attr__open(attr, pid, cpu, fd, group_fd, flags);
// #endif
        return fd;
}
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
```c
static inline int
sys_perf_event_open(struct perf_event_attr *attr,
                      pid_t pid, int cpu, int group_fd,
                      unsigned long flags)
{
        int fd;

        fd = syscall(__NR_perf_event_open, attr, pid, cpu,
                     group_fd, flags);

// #ifdef HAVE_ATTR_TEST
//      if (unlikely(test_attr__enabled))
//              test_attr__open(attr, pid, cpu, fd, group_fd, flags);
// #endif
        return fd;
}
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