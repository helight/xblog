+++
title = "Linux内核中的P，V操作之P"
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

最近作辅导讲到了P，V操作，我就在内核中看了一下Linux中的P，V操作的实现。很真是，理解比本科学习的时候深多了。操作系统中的P操作在Linux内核中对应的是down函数，V操作对用up函数。 
``` c
void down(struct semaphore *sem) { 
    unsigned long flags; 
    spin_lock_irqsave(&sem->lock, flags); 
    if (likely(sem->count > 0)) //从这里可以看出我们操作系统中P操作的身影 
        sem->count--; 
    else 
        __down(sem); //在这里没有资源时，阻塞进程。 
    spin_unlock_irqrestore(&sem->lock, flags); 
} 
```
semaphore结构是这样的:
``` c
struct semaphore { 
    spinlock_t lock; 
    unsigned int count; 
    struct list_head wait_list; 
}; 
```
顺便列出信号量初始化的两个宏和相关函数： 
``` c
#define init_MUTEX(sem) sema_init(sem, 1) //将信号量count初始化为1。 
#define init_MUTEX_LOCKED(sem) sema_init(sem, 0)//将信号量count初始化为0 
static inline void sema_init(struct semaphore *sem, int val) { 
    static struct lock_class_key __key; 
    *sem = (struct semaphore) __SEMAPHORE_INITIALIZER(*sem, val); 
    lockdep_init_map(&sem->lock.dep_map, "semaphore->lock", &__key, 0); 
} 
#define __SEMAPHORE_INITIALIZER(name, n) \ { \ .lock = __SPIN_LOCK_UNLOCKED((name).lock), \ .count = n, \ .wait_list = LIST_HEAD_INIT((name).wait_list), \ } 
```
在这里我们主要看一下__down(sem);这个阻塞函数的实现过程。 
``` c
static noinline void __sched __down(struct semaphore *sem) { 
    __down_common(sem, TASK_UNINTERRUPTIBLE, MAX_SCHEDULE_TIMEOUT); 
} 

#define MAX_SCHEDULE_TIMEOUT LONG_MAX 
#define LONG_MAX ((long)(~0UL>>1)) __down_common
```
这个函数的代码有点长，就不列取了，对它的功能作一些解释吧！ 首先它进行状态检查和时间片检查。再将当前进程的状态设置为UNINTERRUPTIBLE，然后进行schedule。将当前进程阻塞。

<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
