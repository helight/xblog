+++
title = "Linux下守护进程认识"
date = "2008-10-25T13:47:08+02:00"
tags = ["linux", "开源"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "守护进程（Daemon）是运行在后台的一种特殊进程。它独立于控制终端并且周期性地执行某种任务或等待处理某些发生的事件。"
keywords = ["开源", "linux", "Daemon"]
+++

守护进程（Daemon）是运行在后台的一种特殊进程。它独立于控制终端并且周期性地执行某种任务或等待处理某些发生的事件。

守护进程是一种很有用的进程。Linux的大多数服务器就是用守护进程实现的。比如，Web服务器httpd等。同时，守护进程完成许多系统任务。比如，作业规划进程crond， 打印进程lpd等。

下面将我理解的Linux下守护进程做一些解释和说明。同时将网上一个常用的解说Linux下守护进程的程序作为实例介绍一下。 Linux下的守护进程 守护进程有三个最基本的特点：
* 后台运行
* 独立于终端
* 完成一定的任务。 

首先所谓的后台运行过程是一般是在图形界面或是终端不可见的；而独立于终端是说它不和终端联系，运行之后一般不接受终端的输入也不向终端输出；而完成一点的任务是每一个守护进程的运行都是为了完成一定的任务而运行的，这些任务一般都是系统相关的任务。 也就是控制台除开这些特殊性以外，守护进程与普通进程基本上没有什么区别。因此，守护进程可以由一个普通进程按照上述的守护进程的特性而改造成为守护进程。 这里首先要说几个概念：进程组，会话和控制终端 

# 进程组：

每运行一个程序或是命令就会产生一个进程组，而每一个进程组有以个领头进程，一般进程组的第一个进程是领头进程，领头进程fork的进程也属于同一个进程组，但是子进程一旦执行exec等就会不属于该进程组。当然子进程也可以成为领头进程，下面的例子就是这样的。 

# 会话:
一次登录形成一个会话 一个会话可包含多个进程组, 但只能有一个前台进程组. setsid 可建立一个新的会话。

# 控制终端:
会话的领头进程打开一个终端之后, 该终端就成为该会话的控制终端 与控制终端建立连接的会话领头进程称为控制进程,一个会话只能有一个控制终端,产生在控制终端上的输入和信号将发送给会话的前台进程组中的所有进程 终端上的连接断开时 (比如网络断开或 Modem 断开), 挂起信号将发送到控制进程(session leader) 。

# 编程守护进程： 
编程实现守护进程也就是要是想上面的三个特点： 
1. 在后台运行。 为避免挂起控制终端将Daemon放入后台执行。方法是在进程中调用fork使父进程终止，让Daemon在子进程中后台执行。 if(pid=fork()) exit(0);//是父进程，结束父进程，子进程继续 
2. 脱离控制终端，登录会话和进程组 先介绍一下Linux中的进程与控制终端，登录会话和进程组之间的关系：进程属于一个进程组，进程组号（GID）就是进程组长的进程号（PID）。登录会话可以包含多个进程组。这些进程组共享一个控制终端。这个控制终端通常是创建进程的登录终端。 
3. 控制终端，登录会话和进程组通常是从父进程继承下来的。我们的目的就是要摆脱它们，使之不受它们的影响。方法是在第1点的基础上，调用setsid()使进程成为会话组长： setsid(); 以下是内核中的一段代码。kernel/sys.c 中系统调用setsid所对应的函数。 
``` c
asmlinkage long sys_setsid(void) {
    struct task_struct *group_leader = current->group_leader; 
    struct pid *sid = task_pid(group_leader); 
    pid_t session = pid_vnr(sid); 
    int err = -EPERM; 
    write_lock_irq(&tasklist_lock); /* Fail if I am already a session leader */ 
    if (group_leader->signal->leader) 
        goto out; /* Fail if a process group id already exists that equals the * proposed session id. */ 
    if (pid_task(sid, PIDTYPE_PGID)) 
        goto out; 
    group_leader->signal->leader = 1; 
    __set_special_pids(sid); 
    spin_lock(&group_leader->sighand->siglock); 
    group_leader->signal->tty = NULL; //这里将是脱离终端 
    spin_unlock(&group_leader->sighand->siglock); 
    err = session; 
out: 
    write_unlock_irq(&tasklist_lock); 
    return err; 
}
```
setsid()调用成功后，进程成为新的会话组长和新的进程组长，并与原来的登录会话和进程组脱离。由于会话过程对控制终端的独占性，进程同时与控制终端脱离。 

1. 禁止进程重新打开控制终端 现在，进程已经成为无终端的会话组长。但它可以重新申请打开一个控制终端。可以通过使进程不再成为会话组长来禁止进程重新打开控制终端：这里也就数只有会话组长才能打开控制主终端。 if(pid=fork()) exit(0);//结束第一子进程，第二子进程继续（第二子进程不再是会话组长） 
2. 关闭打开的文件描述符 进程从创建它的父进程那里继承了打开的文件描述符。如不关闭，将会浪费系统资源，造成进程所在的文件系统无法卸下以及引起无法预料的错误。按如下方法关闭它们： for(i=0;i 关闭打开的文件描述符close(i);> 
3. 改变当前工作目录 进程活动时，其工作目录所在的文件系统不能卸下。一般需要将工作目录改变到根目录。对于需要转储核心，写运行日志的进程将工作目录改变到特定目录如/tmpchdir(\"/\") 
4. 重设文件创建掩模 进程从创建它的父进程那里继承了文件创建掩模。它可能修改守护进程所创建的文件的存取位。为防止这一点，将文件创建掩模清除：umask(0); 当 我们登录系统之后创建一个文件总是有一个默认权限的，那么这个权限是怎么来的呢？这就是umask干的事情。umask设置了用户创建文件的默认权限，它 与chmod的效果刚好相反，umask设置的是权限“补码”，而chmod设置的是文件权限码。也就是是说它可以决定当前进程打开或是创建文件的权限。 如：umask值为022，则默认目录权限为755. 
5. 处理SIGCHLD信号 处 理SIGCHLD信号并不是必须的。但对于某些进程，特别是服务器进程往往在请求到来时生成子进程处理请求。如果父进程不等待子进程结束，子进程将成为僵 尸进程（zombie）从而占用系统资源。如果父进程等待子进程结束，将增加父进程的负担，影响服务器进程的并发性能。在Linux下可以简单地将 SIGCHLD信号的操作设为SIG_IGN。 signal(SIGCHLD,SIG_IGN); 这样，内核在子进程结束时不会产生僵尸进程。这一点与BSD4不同，BSD4下必须显式等待子进程结束才能释放僵尸进程。 这里我的理解是：主要应用在进程分离当中，最典型的当属ftp，http等网络服务程序的守护进程。这里使用 SIGCHLD信号后，子进程将会不属于父进程，而从属于init进程，父进程则不须等待子进程的结束或着负责子进程结束后的回收，子进程的回收完全有init进程来完成。