+++
title = "ROMFS文件系统分析"
date = "2010-01-18T13:47:08+02:00"
tags = ["romfs", "开源"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "ROMFS是一种简单的只读文件系统，主要是用来当做初始文件系统来使用的，在嵌入式linux或是uclinux中通常使用这中文件系统来作为引导系统的文件系统，甚至uclinux有时就直接把ROMFS作为其根文件系统，而不是将其作为系统启动中的过渡文件系统。"
keywords = ["romfs", "文件系统", "开源"]
+++

# ROMFS文件系统分析
作者：<a href="mailto:zhwenxu@gmail.com">许振文</a>

ROMFS是一种简单的只读文件系统，主要是用来当做初始文件系统来使用的，在嵌入式linux或是uclinux中通常使用这中文件系统来作为引导系统的文件系统，甚至uclinux有时就直接把ROMFS作为其根文件系统，而不是将其作为系统启动中的过渡文件系统。在前面我也分析过，linux操作系统启动中一个是要加载内核，另一个就是要加载一个用于系统简单初始化的文件系统。这个文件系统的格式也是经过了很多发展的。现在一般使用的是一中cpio的格式。在嵌入式系统中一般使用romfs＋其它的可读文件系统。romfs由于它的小巧性（其内核编译只有4000字节），所以非常适合作为系统启动初始化的文件系统。本文就是对ROMFS文件系统进行结构上的分析。前面我也曾对其源代码结构进行了简单的分析。关于ROMFS最为权威的资料是内核源代码树下的“Documentation/filesystems/romfs.txt”。本文多数资料就是来自于该文件。<br>

# ROMFS文件系统的制作
一般我们可以使用一些工具来制作ROMFS的文件系统。制作好之后其实也就是一个二进制的文件。制作工具一般使用”genromfs“，这个工具在网上就可下载到，其源代码并不是很多，只有不到900行。

以下是genromfs工具所支持的参数：
``` sh
xux@zhwen:~/fs-sys$ genromfs -h
genromfs 0.5.2
Usage: genromfs [OPTIONS] -f IMAGE
Create a romfs filesystem image from a directory

  -f IMAGE               Output the image into this file
  -d DIRECTORY           Use this directory as source
  -v                     (Too) verbose operation
  -V VOLUME              Use the specified volume name
  -a ALIGN               Align regular file data to ALIGN bytes
  -A ALIGN,PATTERN       Align all objects matching pattern to at least ALIGN bytes
  -x PATTERN             Exclude all objects matching pattern
  -h                     Show this help

Report bugs to chexum@shadow.banki.hu
xux@zhwen:~/fs-sys$ 
``` 
参数解释：<br>
-f IMAGE     指定输出romfs映像的名字<br>
-d DIRECTORY 指定源目录（将该目录制作成romfs文件系统）<br>
-v           显示详细的创建过程<br>
-V VOLUME    指定卷标<br>
-a ALIGN     指定普通文件的对齐边界（默认为16字节）<br>
-A ALIGN,PATTERN 匹配参数PATTERN的对象对齐在ALIGN边界上<br>
-x PATTERN 不包括匹配PATTERN的对象。<br>
-h 显示帮助文档。<br>
以下是如何制作生成一个romfs的文件系统：

``` sh
xux@zhwen:~/fs-sys$ ls
test  
xux@zhwen:~/fs-sys$ ls test/
test  xux  zhwen
xux@zhwen:~/fs-sys$ genromfs -V "xromfs" -f romfs.img -d test
xux@zhwen:~/fs-sys$ ls
romfs.img  test 
xux@zhwen:~/fs-sys$ file romfs.img 
romfs.img: romfs filesystem, version 1 592 bytes, named xromfs.
xux@zhwen:~/fs-sys$ sudo mount romfs.img /mnt -o loop
xux@zhwen:~/fs-sys$ ls /mnt/
test  xux  zhwen
xux@zhwen:~/fs-sys$ 
```

# ROMFS文件系统结构分析
ROMFS系统中最大文件的大小理论上可以达到4G，文件名的大小一般小于16字节，而且整个文件系统都是以16字节来对齐。<br>
其结构如下：
``` sh
offset      content
        +---+---+---+---+
  0     |  -  |  r  |  o  | m  |  \
        +---+---+---+---+    The ASCII representation of those bytes
  4     |  1  |  f  |  s  |  -  |  /    (i.e. "-rom1fs-")
        +---+---+---+---+
  8     |     full size       |    The number of accessible bytes in this fs.
        +---+---+---+---+
 12     |    checksum   |       The checksum of the FIRST 512 BYTES.
        +---+---+---+---+
 16     |  volume name     |    The zero terminated name of the volume,
        :               :       padded to 16 byte boundary.
        +---+---+---+---+
 xx     |     file      |
        :    headers    :
```
## File headers之前的字节由如下的数据结构来控制：include/linux/romfs_fs.h
``` c
/* On-disk "super block" */
struct romfs_super_block {
        __be32 word0;
        __be32 word1;
        __be32 size;
        __be32 checksum;
        char name[0];           /* volume name */
};
```
（1）这个数据结构中的word0和word1的是固定的值：“-rom1fs-”，由如下的宏定义说明:<br>
include/linux/romfs_fs.h
``` c
#define ROMSB_WORD0 __mk4('-','r','o','m')
#define ROMSB_WORD1 __mk4('1','f','s','-')
```
（2）而size是对整个文件系统的大小的说明。<br>
（3）checksum是对前512个字节的校验和（如果小于512，就以实际大小计算）。<br>
（4）name是当前这个文件系统的名称。<br>

## 下面在来看文件的堆放格式
``` c
offset      content
        +---+---+---+---+
        |     file header    ｜      
        +---+---+---+---+      
        |      file date     ｜      
        +---+---+---+---+      
        |     file header    ｜      
        +---+---+---+---+      
        |      file date     ｜      
        |     file header    ｜      
        +---+---+---+---+      
        |      file date     ｜      
        +---+---+---+---+      
        |      …….        ｜      
        +---+---+---+---+      
```
## File header的格式如下
``` c
offset      content
        +---+---+---+---+
  0     | next filehdr   | X  |       The offset of the next file header
        +---+---+---+---+         (zero if no more files)
  4     |   spec.info       |       Info for directories/hard links/devices
        +---+---+---+---+
  8     |        size       |       The size of this file in bytes
        +---+---+---+---+
 12     |    checksum     |       Covering the meta data, including the file
        +---+---+---+---+         name, and padding
 16     |    file name      |       The zero terminated name of the file,
        :                    :       padded to 16 byte boundary
```
在内核源代码中如下：include/linux/romfs_fs.h
``` c
/* On disk inode */
struct romfs_inode {
        __be32 next;            /* low 4 bits see ROMFH_ */
        __be32 spec;
        __be32 size;
        __be32 checksum;
        char name[0];
};
```
（1）其中next的前面28位是指向下一个文件的地址，应为整个文件系统以16字节对齐，所以任何一个文件的起始地址的最后4位始终为“0”。<br>
而这最后的4位并没就此浪费，而是进行了新的利用－－指定文件的类型和是否可执行。在linux下文件的类型分为：目录，一般文件，链接文件，管道文件，设备文件等。
所以这4位中的最高一位使用来表示该文件是否可执行，而其余三位使用来表示该文件的类型。<br>
include/linux/romfs_fs.h
``` c
#define ROMFH_TYPE 7
#define ROMFH_HRD 0
#define ROMFH_DIR 1
#define ROMFH_REG 2
#define ROMFH_SYM 3
#define ROMFH_BLK 4
#define ROMFH_CHR 5
#define ROMFH_SCK 6
#define ROMFH_FIF 7
#define ROMFH_EXEC 8
```
（2）spec这个字段存放的是目录／硬链接／设备文件的相关信息：

这个域是文件类型相关的，也就是说对于不同的文件类型，这个域表示的含义是不一样的。下面是具体的说明；来自“Documentation/filesystems/romfs.txt”。
``` c
          mapping               spec.info means
 0      hard link       link destination [file header]
 1      directory       first file's header
 2      regular file    unused, must be zero [MBZ]
 3      symbolic link   unused, MBZ (file data is the link content)
 4      block device    16/16 bits major/minor number
 5      char device                 - " -
 6      socket          unused, MBZ
 7      fifo            unused, MBZ
```

（3）size是这个文件的大小。
（4）checksum这个域只是文件头和文件名的校验和。
（5）name是文件的名称。


<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
