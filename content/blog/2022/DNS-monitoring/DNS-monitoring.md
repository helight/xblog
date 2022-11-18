---
title: "深入学习 eBPF：编写高效 DNS 监控"
date: 2022-10-23T08:47:20+08:00
tags: ["ebpf"]
categories: ["ebpf",]
banner: "img/banners/ebpf.png"
author: "helight"
authorlink: "http://helight.cn"
summary: ""
keywords: ["ebpf", "dns"]
draft: true
---

## A Deep Dive into eBPF: Writing an Efficient DNS Monitoring.

[eBPF](https://docs.kernel.org/bpf/classic_vs_extended.html) is an in-kernel virtual machine, provides a high-level library, instruction set and an execution environment inside the Linux kernel. It’s used in many Linux kernel subsystems, most prominently networking, tracing, debugging and security. Including to modify the processing of packets in the kernel and also allows the programming of network devices such as SmartNICs.

![](DNS-monitoring/imgs/1.png)

_Use cases in eBPF implementation._

I will not talk here the detail about what eBPF is. [A lot of posts have already been published about the eBPF](https://github.com/zoidbergwill/awesome-ebpf) and in a variety of languages. Although many of these are fairly informative, they don’t answer the most important questions: How does the eBPF process packets and monitor the packet take from the host to the user?. I will describe the process of creating an actual application from the beginning, especially in monitoring requests, responses and process in DNS, gradually enriching the functionality and accompanying all this with explanations, comments, and links to the source code. And sometimes a little off the side because you want to give a few more examples, not just a solution to a specific problem. As a result, I hope those who want to get acquainted with eBPF will spend less time researching for useful materials and start programming faster.

## Introduction

Let’s say the host can send legitimate DNS requests, but the IP addresses it will send them are unknown. In the network filter log, you can see that the requests are still coming. But it’s not clear — is this just legitimate, or is the information already leaking to the attackers? It would be easier if the domain to which the server sends data were known. Unfortunately, PTR is out of fashion, and securitytrails show either nothing or too much on this IP.

You can run [_tcpdump_](https://www.tcpdump.org/). But who wants to look at the monitor constantly? And if there is more than one server? There is a [_packetbeat_](https://www.elastic.co/beats/packetbeat) from ELK Stack and this is a monster that has eaten out the processor on all my servers. [_Osquery_](https://osquery.io/) is a good tool that knows much about network connections and not about DNS queries. The relevant offer was closed. [_Zeek_](https://zeek.org/) — I learned about it while looking for how to track DNS queries. It seems like it’s not bad, but I was confused by two points: it monitors not only DNS, which means resources will be spent on work that I don’t need the result of (although, perhaps, you can select protocols in the settings); and it also doesn’t know anything about which process sent the request.

We will write in Python and start with the simplest — we will understand how Python and eBPF interact. First, we will install these packages:

``` sh
#apt install python3-bpfcc bpfcc-tools libbpfcc linux-headers-$(uname -r)
```


This is for Ubuntu. But if you go into the kernel, finding the necessary packages for your distribution should not be a problem. Now let’s get started:
``` python

#!/usr/bin/env python3from bcc import BPFFIRST_BPF = r"""  
int first(void *ctx) {  
  bpf_trace_printk("Hello world! execve() is calling\n");  
  return 0;  
}  
"""bpf = BPF(text=FIRST_BPF)  
bpf.attach_kprobe(event=bpf.get_syscall_fnname("execve"), fn_name="first")while True:  
    try:  
        (_, _, _, _, _, event_b) = bpf.trace_fields()  
        events = event_b.decode('utf8')  
        if 'Hello world' in events:  
            print(events)  
    except ValueError:  
        continue  
    except KeyboardInterrupt:  
        break
```

> **Note:** Unprivileged users were allowed to load eBPF programs by default in Ubuntu 20.04 LTS and 18.04 LTS, however, on more recent Ubuntu releases (21.10 and 22.04 LTS) this was disabled by default for security concern. You can re-enable this ability :
> 
> $ sudo sysctl kernel.unprivileged_bpf_disabled=0

As befits all hello-world examples, it doesn’t do anything useful but introduces us to the basics. Every time any program on the host calls the **execve()** system call, the **first()** function of our program gets executed. To trigger it, you can run the command “ls|cat|grep|clear or any command containing **execve()**” on a different console, then our code gets executed. eBPF programs can be called on various events occurring in the kernel. **attach_kprobe()** means triggered when a specific kernel function is called. But we are more used to dealing with system calls. Who knows the names of the corresponding functions? Therefore, a helper function converts the system call name to a kernel function **get_syscall_fnname()**.

The simplest output option in eBPF is a function **bpf_trace_printk()**. But this is the output for debugging. Everything you pass to this function will be available via a file _/sys/kernel/debug/tracing/trace_pipe_. And to not read this file in the next console, we use a function **trace_fields()** that reads this file itself and makes its contents available to us in the program.

The rest of it should be clear — in an infinite loop interrupted by Ctrl-C, we read the debug output, and if “Hello world” occurs in the string, we output it in its entirety.

> **Note**: **bpf_trace_printk()** can format text, similar **printf()**, but with important restrictions — no more than 3 arguments and only one of them **%s**.

Now that we understand how to work with eBPF in general let’s start building an actual application. It will monitor all DNS requests and responses and log who asked what and what response they received.

## The Beginning

Let’s start with eBPF. The easiest way to work with packets is to attach them to a network socket. In this case, our program will be triggered for each packet. I’ll show you exactly how this is done later, but for now, we need to catch UDP with port 53 among all the packets. And to do this, we will have to disassemble the package structure ourselves and separate all the nested protocols in C. Starting with Ethernet. A macro that [**_cursor_advance_**](https://github.com/iovisor/bcc/blob/master/src/cc/export/helpers.h#L524) moves the cursor (pointer) around the packet, returning its current position and shifting it by the specified amount, will help us do this:

``` c

#include **<linux/if_ether.h>**  
#include **<linux/in.h>**  
#include **<bcc/proto.h>**int dns_matching(struct __sk_buff *skb) {  
 u8 *cursor = 0;// Checking the IP protocol::  
 struct **ethernet_t** *ethernet = cursor_advance(cursor, sizeof(*ethernet));if (ethernet->type == ETH_P_IP) {  
 …
```
The structure **ethernet_t** described in the [proto.h](https://github.com/iovisor/bcc/blob/master/src/cc/export/proto.h#L25) file:

``` c
struct ethernet_t {  
  unsigned long long  dst:48;  
  unsigned long long  src:48;  
  unsigned int        type:16;  
} BPF_PACKET_HEADER;
```

The Ethernet frame format itself is pretty simple — it’s 6 bytes (48 bits) of the destination, the same number of sources, and then two bytes (16 bits) of the content type.

The content type is encoded by a constant **ETH_P_IP** equal to 0x0800 and defined in the file [**if_ether.h**](https://kernel.googlesource.com/pub/scm/linux/kernel/git/nico/archive/+/d9cc76127bcc137e3214b9166c439e02d2060cda/include/linux/if_ether.h#32) — it allows you to ensure that the next-level protocol is IP (this code, as well as other possible values, is described by the [IEEE](https://standards-oui.ieee.org/ethertype/eth.txt)).

Let’s move on and check if the IP is nested in UDP with port 53:

``` c
// Checking the UDP protocol:  
struct ip_t *ip = cursor_advance(cursor, sizeof(*ip));if (ip->nextp == IPPROTO_UDP) {  
    // Checking port 53:  
    struct udp_t *udp = cursor_advance(cursor, sizeof(*udp));    if (udp->dport == 53) {  
        // Request  
        return -1;  
    }    if (udp->sport == 53) {  
        // Respose  
        return -1;  
    }  
}
```

**ip_t** and **udp_t** they’re still the same proto.h. But **IPPROTO_UDP** is already from the file [in.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/in.h#L43). In general, this example is not entirely correct. The IP structure is already a little more complicated — it has an optional field, which is why the header length may vary. It would be correct to first get the value of its length from the header and only perform the offset, but we have just started — we will not immediately complicate it.

We found the DNS package, and it was not difficult. Now we need to analyse its structure. To make this easier, we will pass the package to user space (it’s responsible for return -1 for this — the return code 0 would mean that the package does not need to be copied).

Back to Python. First, we will still attach our program to the socket:
``` python
#!/usr/bin/env python3
import dnslib  
import sys
from bcc import BPF  

...

bpf = BPF(text=BPF_PROGRAM)  
function_dns_matching = bpf.load_func("dns_matching", BPF.SOCKET_FILTER)  
BPF.attach_raw_socket(function_dns_matching, '')
```
This difference from the previous example is because now our program will be called not when calling any function but for each package. An empty argument in attach_raw_socket means “all network interfaces,” If we needed a specific one, its name should be there.

Switching the socket to blocking mode:
``` python
import fcntl  
import ossocket_fd = function_dns_matching.sock  
fl = fcntl.fcntl(socket_fd, fcntl.F_GETFL)  
fcntl.fcntl(socket_fd, fcntl.F_SETFL, fl & ~os.O_NONBLOCK)
```

The rest is simple — we use a similar infinite loop, where we read data from the socket, cut off all headers, get directly to the DNS packet and decode it.

Full code here:
``` python
#!/usr/bin/env python3

import dnslib

import fcntl

import os

import sys

from bcc import BPF

BPF_APP = r'''

#include <linux/if_ether.h>

#include <linux/in.h>

#include <bcc/proto.h>

int dns_matching(struct __sk_buff *skb) {

u8 *cursor = 0;

// Checking the IP protocol:

struct ethernet_t *ethernet = cursor_advance(cursor, sizeof(*ethernet));

if (ethernet->type == ETH_P_IP) {

// Checking the UDP protocol:

struct ip_t *ip = cursor_advance(cursor, sizeof(*ip));

if (ip->nextp == IPPROTO_UDP) {

// Check the port 53:

struct udp_t *udp = cursor_advance(cursor, sizeof(*udp));

if (udp->dport == 53 || udp->sport == 53) {

return -1;

}

}

}

return 0;

}

'''

bpf = BPF(text=BPF_APP)

function_dns_matching = bpf.load_func("dns_matching", BPF.SOCKET_FILTER)

BPF.attach_raw_socket(function_dns_matching, '')

socket_fd = function_dns_matching.sock

fl = fcntl.fcntl(socket_fd, fcntl.F_GETFL)

fcntl.fcntl(socket_fd, fcntl.F_SETFL, fl & ~os.O_NONBLOCK)

while True:

try:

packet_str = os.read(socket_fd, 2048)

except KeyboardInterrupt:

sys.exit(0)

packet_bytearray = bytearray(packet_str)

ETH_HLEN = 14

UDP_HLEN = 8

# IP header length

ip_header_length = packet_bytearray[ETH_HLEN]

ip_header_length = ip_header_length & 0x0F

ip_header_length = ip_header_length << 2

# Starting the DNS packet

payload_offset = ETH_HLEN + ip_header_length + UDP_HLEN

payload = packet_bytearray[payload_offset:]

dnsrec = dnslib.DNSRecord.parse(payload)

# If it’s the response:

if dnsrec.rr:

print(f'Resp: {dnsrec.rr[0].rname} {dnslib.QTYPE.get(dnsrec.rr[0].rtype)} {", ".join([repr(dnsrec.rr[i].rdata) for i in range(0, len(dnsrec.rr))])}')

# If it’s the request:

else:

print(f'Request: {dnsrec.questions[0].qname} {dnslib.QTYPE.get(dnsrec.questions[0].qtype)}')
```

This example will show you which DNS requests/responses pass through your network interface, but this way, **_we won’t know what process works with them_**. That is, just the information, due to the lack of which I did not choose Zeek.

## **From Packet to Process**

To get information about the process in eBPF, the following functions are used — [**bpf_get_current_pid_tgid()**](https://github.com/torvalds/linux/blob/master/tools/perf/include/bpf/unistd.h#L5), **bpf_get_current_uid_gid()**, **bpf_get_current_comm(char *buf, int size_of_buf)**. They are available when we bind our program to a call to some kernel function (as in the first example). The _UID/GID_ should be clear. But the first one requires an explanation for those who have not previously encountered such details of the kernel operation. The fact is that what is seen as a _PID_ in the kernel is displayed in user space as the process _thread ID_. And what the kernel considers _thread group ID_-in user space is the PID. Similarly, **bpf_get_current_comm()** returns not the usual process name, which can be seen through **ps** command, but the thread name.

All right, we’ll get the process data. How do we pass them to the user space? Tables are used for this purpose. They are created as **BPF_PERF_OUTPUT(event)**, passed by the method **event.perf_submit(ctx, data, data_size)**, and received by polling via **b.perf_buffer_poll()**. After that, as soon as the data is available, the function **callback()** will be called, thus: **b[“event”].open_perf_buffer(callback)**.

I will describe all of this in detail below, but for now, let’s continue the theory and reflect on this. We can transmit the packet itself as well as the data. But to do this, we must select a variable of a certain length in the structure with the transmitted data. Which one? The quick and incorrect answer is _512 bytes_. But it does not consider EDNS, and I would also like to track (correctly!) DNS packets going over TCP. So we would have to allocate a large amount _“in reserve”,_ discard packages that are still larger, and most of the time, we will have more memory allocated than necessary. I wouldn’t say I like this approach. Fortunately, there is another method — **perf_submit_skb()**. In addition to data, it also transmits the specified number of bytes of the packet from the buffer. But there is a caveat — the method is only available for network programs eBPF- socket, XDP. I.e., those where we can not get information about the process.

Fortunately, we can use multiple eBPF programs and exchange data between them! And this also happens through tables. They are declared as follows:

``` python

BPF_TABLE_PUBLIC("hash", key, val, name, max_elements);
```

This is to make it available to other eBPF programs. And to access it, in another program, we write like this:
``` python
BPF_TABLE("extern", key, val, name, max_elements);
```
So that we don’t lose our packet among the rest-just 5 unique parameters: protocol, source address, source port, destination address, and destination port, so the key will be the following structure:
``` c
**struct** **port_key** {  
     u8 proto;  
     u32 saddr;  
     u32 daddr;  
     u16 sport;  
     u16 dport;  
 };
 ```

And the value is everything we want to know about the process:
``` c
**struct** **port_val** {  
     u32 ifindex;  
     u32 pid;  
     u32 tgid;  
     u32 uid;  
     u32 gid;  
     **char** comm[64];  
 };
```
_ifindex_ — this is a network device. We will fill in this value in another program running on the socket. And here, we use it to transfer the entire structure to the user’s space in the future.

Total: when calling the kernel function to send a packet, we store information about which process is involved. And when a packet appears on the network interface (and it doesn’t matter whether it’s outgoing or incoming), we check whether we have any information for packets traveling between these destinations using such and such a protocol. If it exists, we pass it along with the package to Python, where we do the rest of the work.

Well, the basic logic of the future program was talked through — let’s already program!

## My Name Is Process

Let’s start by getting information about the process. The [**udp_sendmsg()**](https://github.com/torvalds/linux/blob/master/net/ipv4/udp.c#L1045) and [**tcp_sendmsg()**](https://github.com/torvalds/linux/blob/master/net/ipv4/tcp.c#L1478) functions are used to send packets. Both take the [_sock_](https://github.com/torvalds/linux/blob/master/include/net/sock.h#L352) structure that we need as the first argument. There are two ways to access the arguments of the function under investigation in eBPF: specify them as parameters of our function, or use the macro **PT_REGS_PARMx**, where x is the argument number. I’ll show you both of these options below. And here is our first program, **C_BPF_KPROBE**:

``` c
// The structure that will be used as the key for   
// eBPF table 'proc_ports':  
struct port_key {  
    u8 proto;  
    u32 saddr;  
    u32 daddr;  
    u16 sport;  
    u16 dport;  
};// The structure that will be stored in the eBPF table 'proc_ports'   
// contains information about the process:  
struct port_val {  
    u32 ifindex;  
    u32 pid;  
    u32 tgid;  
    u32 uid;  
    u32 gid;  
    char comm[64];  
};// Public (accessible from other eBPF programs) eBPF table in which   
// information about the process is written.   
// It's read when a packet appears on the socket:  
BPF_TABLE_PUBLIC("hash", struct port_key, struct port_val, proc_ports, 20480);// These are two ways to get access to the function arguments:  
//int trace_udp_sendmsg(struct pt_regs *ctx) {  
// struct sock *sk = (struct sock *)PT_REGS_PARM1(ctx);  
int trace_udp_sendmsg(struct pt_regs *ctx, struct sock *sk) {  
    u16 sport = sk->sk_num;  
    u16 dport = sk->sk_dport;  
    
    // Processing packets only on port 53.  
    // 13568 = ntohs(53);  
    if (sport == 13568 || dport == 13568) {  
        // Preparing the data:  
        u32 saddr = sk->sk_rcv_saddr;  
        u32 daddr = sk->sk_daddr;  
        u64 pid_tgid = bpf_get_current_pid_tgid();  
        u64 uid_gid = bpf_get_current_uid_gid();        // Forming the key structure.  
        // These strange transformations will be explained below.  
        struct port_key key = {.proto = 17};  
        key.saddr = htonl(saddr);  
        key.daddr = htonl(daddr);  
        key.sport = sport;  
        key.dport = htons(dport);        // Forming a structure with the process properties:  
        struct port_val val = {};  
        val.pid = pid_tgid >> 32;  
        val.tgid = (u32)pid_tgid;  
        val.uid = (u32)uid_gid;  
        val.gid = uid_gid >> 32;  
        bpf_get_current_comm(val.comm, 64);        //Writing the value into the eBPF table:  
        proc_ports.update(&key, &val);  
    }  
    return 0;  
}
```

Working with _tcp_sendmsg_ will be absolutely the same. The only difference is that in the structure _port_key_, the field proto will be equal 6. These two values (17 and 6) are the codes of the UDP and TCP protocols, respectively. You can view these values in the file _/etc/protocols_.

Both functions _bpf_get_current_*_return 64 bits, so we take the lower and upper 32 bits separately to extract data. Moreover, for PID/TGID, we immediately take them in the usual form (i.e.pid, we write the upper 32 bits in the field, which contain what the kernel considers to be the TGID).

Now let’s talk about transformations when forming the key structure. We will create a similar structure in the program in the next section. But we will take data, not from the nuclear structure _sock_ but eBPF’s [___sk_buff_](https://github.com/iovisor/bcc/blob/master/src/cc/compat/linux/virtual_bpf.h#L5746), and in it, the data is stored in this form:
``` c
__u32 remote_ip4; /* Stored in network byte order */  
__u32 local_ip4; /* Stored in network byte order */  
__u32 remote_port; /* Stored in network byte order */  
__u32 local_port; /* stored in host byte order */
```

## Extracted to User Space

Our second program **BPF_SOCK_TEXT**, which will “hang” on the socket, will check for information about the corresponding process for each packet and transmit it, along with the packet itself, to user space:

``` c
// The structure that will be used as the key for  
// eBPF table 'proc_ports':  
struct port_key {  
    u8 proto;  
    u32 saddr;  
    u32 daddr;  
    u16 sport;  
    u16 dport;  
};// The structure that will be stored in the eBPF table 'proc_ports',  
// Contains information about the process:  
struct port_val {  
    u32 ifindex;  
    u32 pid;  
    u32 tgid;  
    u32 uid;  
    u32 gid;  
    char comm[64];  
};// eBPF table from which information about the process is extracted.  
// Filled when calling kernel functions udp_sendmsg()/tcp_sendmsg():  
BPF_TABLE("extern", struct port_key, struct port_val, proc_ports, 20480);// Table for transferring data to the user space:  
BPF_PERF_OUTPUT(dns_events);// Look for DNS packets among the data passing through the socket and   
// check if there is any information about the process:  
int dns_matching(struct __sk_buff *skb) {  
    u8 *cursor = 0;// Checking the IP protocol:  
struct ethernet_t *ethernet = cursor_advance(cursor, sizeof(*ethernet));     if (ethernet->type == ETH_P_IP) {  
        struct ip_t *ip = cursor_advance(cursor, sizeof(*ip));        u8 proto;  
        u16 sport;  
        u16 dport;        // Checking the transport layer protocol:  
        if (ip->nextp == IPPROTO_UDP) {  
            struct udp_t *udp = cursor_advance(cursor, sizeof(*udp));            proto = 17;            // Getting the data about the ports:  
            sport = udp->sport;  
            dport = udp->dport;  
        } else if (ip->nextp == IPPROTO_TCP) {  
            struct tcp_t *tcp = cursor_advance(cursor, sizeof(*tcp));            // We don't need packets where no data is transmitted:  
            if (!tcp->flag_psh) {  
                return 0;  
            }            proto = 6;            // Getting the data about the ports:  
            sport = tcp->src_port;  
            dport = tcp->dst_port;  
        } else {  
            return 0;  
        }        // If it's a DNS query:  
        if (dport == 53 || sport == 53) {  
            // Form a key structure:  
            struct port_key key = {};  
            key.proto = proto;  
            if (skb->ingress_ifindex == 0) {  
                key.saddr = ip->src;  
                key.daddr = ip->dst;  
                key.sport = sport;  
                key.dport = dport;  
            } else {  
                key.saddr = ip->dst;  
                key.daddr = ip->src;  
                key.sport = dport;  
                key.dport = sport;  
            }            // By the key, look for a value in the eBPF table:  
            struct port_val *p_val;  
            p_val = proc_ports.lookup(&key);            // If no value is found, then we have no information about the   
            // process and there is no point in continuing:            if (!p_val) {  
                return 0;  
            }            // Network device index:  
            p_val->ifindex = skb->ifindex;            // Transmit the structure with the process information along with   
            // skb->len bytes sent to the socket:  
            dns_events.perf_submit_skb(skb, skb->len, p_val,  
                                       sizeof(struct port_val));  
            return 0;  
        } //dport == 53 || sport == 53  
    } //ethernet->type == ETH_P_IPreturn 0;  
}
```

The program starts in the same way as one of the first examples considered. We move around the packet and collect information from protocols at different levels. The comment that this approach does not consider the actual length of the IP header is still valid. But something new has also been added — for TCP packets, we check the flag — we don’t need packets that don’t carry data (_SYN_, _ACK_, etc.).

But then we must restore the key to get data from the table **proc_ports**. At the same time, we must distinguish the direction of traffic — after all, when we entered data in the table, we meant that we were the source. But for incoming packets, the source will be the remote server. To understand the direction of movement of packets, I used a field **ingress_ifindex** that is 0 for outgoing traffic.

## Serving

From Python, we need three things: load our programs into the kernel, get data from them, and process it.

The first two tasks are simple. Moreover, we have already considered both methods of working with eBPF in the first examples:

``` python
# BPF initialization:  
bpf_kprobe = BPF(text=C_BPF_KPROBE)  
bpf_sock = BPF(text=BPF_SOCK_TEXT)# Send UDP:  
bpf_kprobe.attach_kprobe(event="udp_sendmsg", fn_name="trace_udp_sendmsg")# Send TCP:  
bpf_kprobe.attach_kprobe(event="tcp_sendmsg", fn_name="trace_tcp_sendmsg")# Socket:  
function_dns_matching = bpf_sock.load_func("dns_matching", BPF.SOCKET_FILTER)  
BPF.attach_raw_socket(function_dns_matching, '')
```

Getting data is even shorter:

``` python
bpf_sock["dns_events"].open_perf_buffer(print_dns)while True:  
    try:  
        bpf_sock.perf_buffer_poll()  
    except KeyboardInterrupt:  
        exit()
```

But data processing will be more cumbersome. Despite the availability of ready-made modules, I decided to parse the protocol headers myself. First, I wanted to figure out for myself how this happens (and finally correctly process the length of the IP packet header, although in this case, it’s pointless because packets with additional options in the header will be discarded in eBPF), and secondly — to reduce dependence on modules. However, for parsing DNS directly, I still (so far) use the module-the DNS structure is slightly more complex than IP / TCP. Another module (_ctypes_) is needed for working with C-sh data types:

``` python
def print_dns(cpu, data, size):  
    import ctypes as ct  
    class SkbEvent(ct.Structure):  
        _fields_ = [  
            ("ifindex", ct.c_uint32),  
            ("pid", ct.c_uint32),  
            ("tgid", ct.c_uint32),  
            ("uid", ct.c_uint32),  
            ("gid", ct.c_uint32),  
            ("comm", ct.c_char * 64),  
            ("raw", ct.c_ubyte * (size - ct.sizeof(ct.c_uint32 * 5) - ct.sizeof(ct.c_char * 64)))  
        ]  
    # We get our 'port_val' structure and also the packet itself in the 'raw' field:  
    sk = ct.cast(data, ct.POINTER(SkbEvent)).contents    # Protocols:  
    NET_PROTO = {6: "TCP", 17: "UDP"}    # eBPF operates on thread names.  
    # Sometimes they coincide with process names, but often not.  
    # So we try to get the process name by its PID:  
    try:  
        with open(f'/proc/{sk.pid}/comm', 'r') as proc_comm:  
            proc_name = proc_comm.read().rstrip()  
    except:  
        proc_name = sk.comm.decode()    # Get the name of the network interface by index:  
    ifname = if_indextoname(sk.ifindex)    # The length of the Ethernet frame header is 14 bytes:  
    ip_packet = bytes(sk.raw[14:])    # The length of the IP packet header is not fixed due to the arbitrary  
    # number of parameters.  
    # Of all the possible IP header we are only interested in 20 bytes:  
    (length, _, _, _, _, proto, _, saddr, daddr) = unpack('!BBHLBBHLL', ip_packet[:20])  
    # The direct length is written in the second half of the first byte (0b00001111 = 15):  
    # len_iph = length & 15  
    # Length is written in 32-bit words, convert it to bytes:  
    # len_iph = len_iph * 4  
    # Convert addresses from numbers into IPs, assembling it into octets:  
    saddr = ".".join(map(str, [saddr >> 24 & 0xff, saddr >> 16 & 0xff, saddr >> 8 & 0xff, saddr & 0xff]))  
    daddr = ".".join(map(map(str, [daddr >> 24 & 0xff, daddr >> 16 & 0xff, daddr >> 8 & 0xff, daddr & 0xff]))    # If the transport layer protocol is UDP:  
    if proto == 17:  
        udp_packet = ip_packet[len_iph:]  
        (sport, dport) = unpack('!HH', udp_packet[:4])  
        # UDP datagram header length is 8 bytes:  
        dns_packet = udp_packet[8:]  
    # If the transport layer protocol is TCP:  
    elif proto == 6:  
        tcp_packet = ip_packet[len_iph:]  
        # TCP packet header length is also not fixed due to the optional  
        # options. Of the entire TCP header, we are only interested in the data up to the 13th  
        # byte (header length):  
        (sport, dport, _, length) = unpack('!HHQB', tcp_packet[:13])  
        # The direct length is written in the first half (4 bits):  
        len_tcph = length >> 4  
        # Length is written in 32-bit words, converted to bytes:  
        len_tcph = len_tcph * 4  
        # That's the tricky part.  
        # I don't know where I went wrong or why I need a 2 byte offset,  
        # but it's necessary because the DNS packet doesn't start until after it:  
        dns_packet = tcp_packet[len_tcph + 2:]  
    # other protocols are not handled:  
    else:  
        return    # DNS data decoding:  
    dns_data = dnslib.DNSRecord.parse(dns_packet)    # Resource record types:  
    DNS_QTYPE = {1: "A", 28: "AAAA"}    # Query:  
    If dns_data.header.qr == 0:  
        # We are only interested in A (1) and AAAA (28) records:  
        for q in dns_data.questions:  
            If q.qtype == 1 or q.qtype == 28:  
                print(f'COMM={proc_name} PID={sk.pid} TGID={sk.tgid} DEV={ifname} PROTO={NET_PROTO[proto]} SRC={saddr} DST={daddr} SPT={sport} DPT={dport} UID={sk.uid} GID={sk.gid} DNS_QR=0 DNS_NAME={q.qname} DNS_TYPE={DNS_QTYPE[q.qtype]}')  
    # Response:  
    elif dns_data.header.qr == 1:  
        # We are only interested in A (1) and AAAA (28) records:  
        For rr in dns_data.rr:  
            If rr.rtype == 1 or rr.rtype == 28:  
                print(f'COMM={proc_name} PID={sk.pid} TGID={sk.tgid} DEV={ifname} PROTO={NET_PROTO[proto]} SRC={saddr} DST={daddr} SPT={sport} DPT={dport} UID={sk.uid} GID={sk.gid} DNS_QR=1 DNS_NAME={rr.rname} DNS_TYPE={DNS_QTYPE[rr.rtype]} DNS_DATA={rr.rdata}')  
    else:  
        print('Invalid DNS query type.')
```

## At The End

Launch the app _.py_ and make a request with [_dig_](https://linux.die.net/man/1/dig) tools in the next console:
``` sh
# dig @1.1.1.1 google.com +tcp
```
If that works, the output of the program should look like this:
``` sh
# python3 final_code_eBPF_dns.py  
The program is running. Press Ctrl-C to abort.  
COMM=dig PID=10738 TGID=10739 DEV=ens18 PROTO=TCP SRC=192.168.44.3 DST=1.1.1.1 SPT=57915 DPT=53 UID=0 GID=0 DNS_QR=0 DNS_NAME=google.com. DNS_TYPE=A  
COMM=dig PID=10738 TGID=10739 DEV=ens18 PROTO=TCP SRC=1.1.1.1 DST=192.168.44.3 SPT=53 DPT=57915 UID=0 GID=0 DNS_QR=1 DNS_NAME=google.com. DNS_TYPE=A DNS_DATA=142.251.12.101  
COMM=dig PID=10738 TGID=10739 DEV=ens18 PROTO=TCP SRC=1.1.1.1 DST=192.168.44.3 SPT=53 DPT=57915 UID=0 GID=0 DNS_QR=1 DNS_NAME=google.com. DNS_TYPE=A DNS_DATA=142.251.12.113  
COMM=dig PID=10738 TGID=10739 DEV=ens18 PROTO=TCP SRC=1.1.1.1 DST=192.168.44.3 SPT=53 DPT=57915 UID=0 GID=0 DNS_QR=1 DNS_NAME=google.com. DNS_TYPE=A DNS_DATA=142.251.12.102  
COMM=dig PID=10738 TGID=10739 DEV=ens18 PROTO=TCP SRC=1.1.1.1 DST=192.168.44.3 SPT=53 DPT=57915 UID=0 GID=0 DNS_QR=1 DNS_NAME=google.com. DNS_TYPE=A DNS_DATA=142.251.12.139  
COMM=dig PID=10738 TGID=10739 DEV=ens18 PROTO=TCP SRC=1.1.1.1 DST=192.168.44.3 SPT=53 DPT=57915 UID=0 GID=0 DNS_QR=1 DNS_NAME=google.com. DNS_TYPE=A DNS_DATA=142.251.12.100  
COMM=dig PID=10738 TGID=10739 DEV=ens18 PROTO=TCP SRC=1.1.1.1 DST=192.168.44.3 SPT=53 DPT=57915 UID=0 GID=0 DNS_QR=1 DNS_NAME=google.com. DNS_TYPE=A DNS_DATA=142.251.12.138
```
So we have created a useful application that shows all DNS queries in our system. I hope my explanations were quite detailed, and if you are interested in writing eBPF programs, it will be easier for you to start. This _code_ has already helped me better understand what is happening on the servers. Below I post its full code.

## Conclusion

Can it be done even better? Of course! First, you should add IPv6 support. Secondly, finally stop relying on the fixed length of the IP header and usually parse it. It’s not for nothing that I refused to use the library in Python to work with packages — in C, and you still have to do it manually. Thirdly, it would be good to rewrite the code in C, abandoning Python completely and of course adding lines of code for JSON output, making it easier for later in developing the UI dashboard. This will lead to the fourth point-manual analysis of the DNS packet. And finally, the most tempting point is to stop looking at ports _(Because maybe DNS packets don’t always cross the port 53)_ and try to analyze each packet and search among them for those that fit the DNS format. This will allow us to detect packets even on non-standard ports.

[**Here’s the final code**](https://gist.github.com/oghie/b4e3accf1f87afcb939f884723e2b462)**.**