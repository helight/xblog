+++
title = "守护进程实例"
date = "2008-09-25T13:47:08+02:00"
tags = ["linux", "开源"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "来自网上一个比较流行的例子，我做了一些修改，可以产生子进程，并且在/tmp/test.log中做了记录。 "
keywords = ["开源", "linux"]
+++

``` c
/* 
*来自网上一个比较流行的例子，我做了一些修改， 
*可以产生子进程，并且在/tmp/test.log中做了记录。 
*/ 
#include <unistd.h> 
#include <signal.h> 
#include <sys/param.h> 
#include <sys/types.h> 
#include <sys/stat.h> 
#include <stdio.h> 
#include <time.h> 

//守护进程初始化函数 
void init_daemon(void) { 
    int pid; int i; 
    if((pid=fork())) 
        exit(1);//是父进程，结束父进程 
    else if(pid< 0) 
            exit(1);//fork失败，退出 //是第一子进程，后台继续执行 
    setsid();//第一子进程成为新的会话组长和进程组长并与控制终端分离 
    if((pid=fork())) 
        exit(0);//是第一子进程，结束第一子进程 
    else if(pid< 0) 
        exit(1);//fork失败，退出 
    //是第二子进程，继续 //第二子进程不再是会话组长 
    for(i=0;i< NOFILE;++i)//关闭打开的文件描述符 
        close(i); 
    chdir("/tmp");  //改变工作目录到/tmp 
    umask(0);//重设文件创建掩模 
    return; 
}
int main(int argc,char **argv) { 
    FILE *fp; 
    time_t t; 
    pid_t pid; 
    init_daemon();//初始化为Daemon 
    while(1)//每隔一分钟向test.log报告运行状态 { 
        sleep(10);//睡眠一分钟 
        if((fp=fopen("test.log","a")) >=0) { 
            t=time(0); 
            fprintf(fp,"deam-fork %d here at %s\n",getpid(),asctime(localtime(&t)) ); 
            fclose(fp);
        } 
        signal(SIGCLD,SIG_IGN);        /*avoid zombie process*/ 
        pid=fork(); 
        if((pid<0)||(pid>0)) continue; 
        if(pid==0) break; 
    } 
    if(pid==0){ 
        sleep(5); 
        if((fp=fopen("test.log","a")) >=0) { 
            t=time(0); 
            fprintf(fp,"fork-child %d here at %s\n",getpid(),asctime(localtime(&t)) ); 
            fclose(fp); 
        } 
        exit(0); 
    } 
    return 0; 
}
```

<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
