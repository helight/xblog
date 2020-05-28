+++
title = "源代码中的中断分析(一)"
date = "2008-11-13T13:47:08+02:00"
tags = ["linux", "开源", "kernel"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "（代码版本2.6.26） 中断描述符数组：irq_desc[]"
keywords = ["开源", "linux", "kernel"]
+++

（代码版本2.6.26） 中断描述符数组：irq_desc[]。
include/linux/irq.h 
``` c
extern struct irq_desc irq_desc[NR_IRQS];
```
由结构体struct irq_desc来构成中断描述符数组。 
NR_IRQS: 
``` c
#define NR_VECTORS 256 
#define NR_IRQS (NR_VECTORS + (32 * NR_CPUS)) 
```
NR_CPUS:cpu数量。这个值是在编译内核时指定的。 
struct irq_desc：include/linux/irq.h 
``` c
struct irq_desc { 
    irq_flow_handler_t handle_irq; /*高级中断事件控制*/ 
    struct irq_chip chip; /*低级别硬件中断*/ 
    struct msi_desc msi_desc; /*可屏蔽软中断描述符*/ 
    void *handler_data; 
    void *chip_data; 
    struct irqaction *action; /* IRQ 服务程序链表 */ 
    unsigned int status; /* IRQ 状态 */ 
    unsigned int depth; /* 中断禁止次数,也称为禁止深度*/ 
    unsigned int wake_depth; /* nested wake enables */ 
    unsigned int irq_count; /* 中断发生次数*/ 
    unsigned int irqs_unhandled; /*未处理中断的计数，这个的计数是在HZ／10内未处理的中断数*/ 
    unsigned long last_unhandled; /* 未处理中断计数的计时，他的值是上一次发生未处理中断时的jiffies值 */ 
    spinlock_t lock; /*用于串行访问中断描述符数组的自旋锁*/ 
    #ifdef CONFIG_SMP 
        cpumask_t affinity; 
        unsigned int cpu; /*SMP中cup的索引号，用于平衡调度＊/ 
    #endif 
    #if defined(CONFIG_GENERIC_PENDING_IRQ) || defined(CONFIG_IRQBALANCE) 
        cpumask_t pending_mask; 
    #endif 
    #ifdef CONFIG_PROC_FS 
        struct proc_dir_entry *dir; /*在/proc/irq/中对应的文件*/ 
    #endif 
    const char *name; /*要在interrupts中显示的名字*/ 
} ____cacheline_internodealigned_in_smp; 
```
这是在SMP中使用的一个数据结构，当然在单cpu（更准确的说应该是单核cpu中）有一些数据项是没有使用的。这其中比较重要的数据项是action和status。一个是真正要执行服务程序的一个链表，另一个是当前irq的状态。

<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
