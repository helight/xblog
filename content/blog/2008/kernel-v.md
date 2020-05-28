+++
title = "Linux内核中的P，V操作之V"
date = "2008-11-29T13:47:08+02:00"
tags = ["linux", "开源", "kernel"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "最近作辅导讲到了P，V操作，我就在内核中看了一下Linux中的P，V操作的实现。很真是，理解比本科学习的时候深多了。操作系统中的P操作在Linux内核中对应的是down函数，V操作对用up函数。"
keywords = ["开源", "linux", "kernel"]
+++

V操作：也在文件：kernel/semaphore.c中。 
``` c
void up(struct semaphore *sem) { 
    unsigned long flags; 
    spin_lock_irqsave(&sem->lock, flags); 
    if (likely(list_empty(&sem->wait_list))) //在这里用list_empty判断sem的等待队列是否为空。 
        sem->count++; //如果为空则只是信号量的计数加1 
    else 
        __up(sem); //否则在这里去唤醒信号量的等待队列上的进程。 
    spin_unlock_irqrestore(&sem->lock, flags); 
} 
```
下面来看看__up(sem)这个函数： 
``` c
static noinline void __sched __up(struct semaphore *sem) {
    //下面一句是在上面确定有等待进程了之后，来取第一个等待进程（当然这里取的一个信号量结构体）。应为第一个进程是等待最久的。 
    struct semaphore_waiter *waiter = list_first_entry(&sem->wait_list, struct semaphore_waiter, list); 
    list_del(&waiter->list); //然后在这个等待队列上将其删除 
    waiter->up = 1; //标识允许唤醒 
    wake_up_process(waiter->task); //这里正真去唤醒进程。 
} 
```
wake_up_process(waiter->task);再调用函数try_to_wake_up(p, TASK_ALL, 0);进行了进程的唤醒。

<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
