+++
title = "The Linux Kernel Device Model"
date = "2010-03-18T13:47:08+02:00"
tags = ["GNU", "开源"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "The Linux Kernel Device Model"
keywords = ["GNU", "nm", "开源", "linux"]
+++


# The Linux Kernel Device Model
# Linux内核设备模型

Patrick Mochel&nbsp;&nbsp;&lt;mochel@digitalimplant.org&gt;

Drafted 26 August 2002

起草于 2002年 8月26日

Updated 31 January 2006

于2006年1月31日更新


# Overview
# 概述

The Linux Kernel Driver Model is a unification of all the disparate driver<br>
models that were previously used in the kernel. It is intended to augment the<br>
bus-specific drivers for bridges and devices by consolidating a set of data<br>
and operations into globally accessible data structures.

Linux内核设备模型是对所有以前在内核中使用过的不同驱动模型的一种统一.它设<br>
计的目的是通过把一组数据和操作统一到全局可访问的数据结构中,来为桥接器和设<br>
备增加具体总线的驱动.

Traditional driver models implemented some sort of tree-like structure<br>
(sometimes just a list) for the devices they control. There wasn't any<br>
uniformity across the different bus types.

传统的驱动模型为描述它所控制的设备实现了一系列的树形结构(有些仅仅是一个链表).<br>
这使得他们在不同类型的总线设备上区别很大.

The current driver model provides a common, uniform data model for describing<br>
a bus and the devices that can appear under the bus. The unified bus<br>
model includes a set of common attributes which all busses carry, and a set<br>
of common callbacks, such as device discovery during bus probing, bus<br>
shutdown, bus power management, etc.

现在的驱动模型给描述一种总线和会出现在这个总线下的设备提供了一种公共,统一的数<br>
据模型.这种统一的总线模型包括了一组所有总线都有的公共属性和一组公共的回掉函数,<br>
例如能在总线探测,总线关闭和总线电源管理时发现设备.


The common device and bridge interface reflects the goals of the modern<br>
computer: namely the ability to do seamless device &quot;plug and play&quot;, power<br>
management, and hot plug. In particular, the model dictated by Intel and<br>
Microsoft (namely ACPI) ensures that almost every device on almost any bus<br>
on an x86-compatible system can work within this paradigm.&nbsp;&nbsp;Of course,<br>
not every bus is able to support all such operations, although most<br>
buses support a most of those operations.

公共设备和桥接口也体现了现代计算机的目标:换句话说也就是实现设备的即插即用,电源<br>
管理和热插拔的功能.特别是由Intel和Microsoft所说的模型(即ACPI),它确保了几乎所有<br>
的设备能在和X86兼容的系统中大多数任意总线上使用.当然并不是每一个总线都能够支持<br>
所有这些操作,但几乎所有的总线支持大多数这样的操作.<br>

# Downstream Access
# 底层访问

Common data fields have been moved out of individual bus layers into a common<br>
data structure. These fields must still be accessed by the bus layers,<br>
and sometimes by the device-specific drivers.

公共的数据项从单个的总线层中被移到了公共数据结构.当然这些项要仍然可以被原来的<br>
总线层访问,有时也要可被设备具体的驱动所访问.

Other bus layers are encouraged to do what has been done for the PCI layer.<br>
struct pci_dev now looks like this:

其它的总线层被用来做以前给PCI层所做的那些工作.所以现在pci_dev的结构是这个样子:

struct pci_dev {
    
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;...

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;struct device dev;

};

Note first that it is statically allocated. This means only one allocation on<br>
device discovery. Note also that it is at the _end_ of struct pci_dev. This is<br>
to make people think about what they're doing when switching between the bus<br>
driver and the global driver; and to prevent against mindless casts between<br>
the two.

首先要注明的是这个结构是静态分配的.这就是说在发现设备时只会分配一个.另外要注意<br>
的是pci_dev结构末尾的这个结构.这会使人们在总线驱动和全局驱动之间切换时有所<br>
顾虑；并且可防止搞混二者.

The PCI bus layer freely accesses the fields of struct device. It knows about<br>
the structure of struct pci_dev, and it should know the structure of struct<br>
device. Individual PCI device drivers that have been converted to the current<br>
driver model generally do not and should not touch the fields of struct device,<br>
unless there is a strong compelling reason to do so.

PCI总线层可以自如的访问结构struct device中的各成员.要了解pci_dev这个数据结构,<br>
也应该知道devibe这个数据结构.已经被转换成当前驱动模型的单独的PCI设备驱动不要也<br>
不应该去动device结构中的成员,除非有强烈的令人信服的理由才去这么做.

This abstraction is prevention of unnecessary pain during transitional phases.<br>
If the name of the field changes or is removed, then every downstream driver<br>
will break. On the other hand, if only the bus layer (and not the device<br>
layer) accesses struct device, it is only that layer that needs to change.

这种抽象是为了防止在过渡其间产生的不必要的麻烦.如果一个成员的名字变了或是被去<br>
除了,那所有底层的驱动将会不可用.另一方面来说,如果只有总线层(并不是设备层)访问<br>
device结构,那就只需修改需要修改的那一层即可.<br>

# User Interface
# 用户接口

By virtue of having a complete hierarchical view of all the devices in the<br>
system, exporting a complete hierarchical view to userspace becomes relatively<br>
easy. This has been accomplished by implementing a special purpose virtual<br>
file system named sysfs. It is hence possible for the user to mount the<br>
whole sysfs filesystem anywhere in userspace.

这种对系统中所有设备进行一种完全分层的组织的好处是可以相对容易的给用户空间提供<br>
一种完全分层次的设备关系图.通过实现一种称之谓sysfs的特殊的虚拟文件系统内核已经<br>
实现了这样的组织视图.因此用户就可以在用户空间的任意点挂载这个完整的sysfs文件系统.

This can be done permanently by providing the following entry into the<br>
/etc/fstab (under the provision that the mount point does exist, of course):

也可以把下面的语句写到/etc/fstab中来实现挂载(因为系统没有挂载点,所以要这样写):

none&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;/sys&nbsp;&nbsp;&nbsp;&nbsp;sysfs&nbsp;&nbsp;&nbsp;&nbsp;defaults&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 0

Or by hand on the command line:<br>
<br>
或者在命令行下敲如下的命令进行挂载:<br>
``` sh
# mount -t sysfs sysfs /sys<br>
```
Whenever a device is inserted into the tree, a directory is created for it.<br>
This directory may be populated at each layer of discovery - the global layer,<br>
the bus layer, or the device layer.

无论何时在这个树上插入设备,内核都会为它创建一个目录.这个目录可能在每个层中出现<br>
比如全局层,总线层,或设备层.

The global layer currently creates two files - 'name' and 'power'. The<br>
former only reports the name of the device. The latter reports the<br>
current power state of the device. It will also be used to set the current<br>
power state. 

在全局层一般创建两个文件-&quot;name&quot;和&quot;power&quot;.前面的只是列出了设备的名称.后一个是<br>
描述设备当前的电源状态.它通常被用来设置当前的电源状态.

The bus layer may also create files for the devices it finds while probing the<br>
bus. For example, the PCI layer currently creates 'irq' and 'resource' files<br>
for each PCI device.

总线层也会为在探测时发现的设备创建文件.例如,PCI层一般会为每一个PCI设备创建'irq'<br>
和'resource'文件.

A device-specific driver may also export files in its directory to expose<br>
device-specific data or tunable interfaces.

一个具体设备的驱动程序也可能会在它的目录下通过创建文件来导出该设备的数据或提供调<br>
整的接口.

More information about the sysfs directory layout can be found in<br>
the other documents in this directory and in the file <br>
Documentation/filesystems/sysfs.txt.

更多关于sysfs目录布局的信息可以查阅当前文件夹中的其它文档和文件<br>
Documentation/filesystems/sysfs.txt...<br>

<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
