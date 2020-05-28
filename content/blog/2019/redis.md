+++
title = "Redis源码分析1:Redis启动分析"
date = "2019-05-18T13:47:08+02:00"
tags = ["redis", "db"]
categories = ["redis"]
banner = "img/banners/redis.png"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "Redis源码分析1:Redis启动分析"
keywords = ["redis","事件驱动", "client"]
+++

# 前言
近期决定把redis的源码阅读分析一下，在[官网](http://redis.io)下载了最新稳定版本5.0.3。整个代码包还是比较小的，下载之后整个包才9M，解压之后看src文件夹也才3.7M，也就是说redis的源码就这么点，其它占空间的主要是几个依赖组件:hiredis(redis的C客户端), lua, jemalloc（内存池）, linenoise（配置文件解析），这些代码占了大概6.3M.

看了redis的代码首先直观感觉就是：规范，非常规范。代码格式，缩紧，注释，命名都是非常规范的，可以说又是一个C代码的软件的典型。所以读起来也相对比较舒服。

计划从这几个方面开始
1. 基本的代码结构和服务启动过程
2. redis的网络模型
3. 主要数据类型
   1. kv
   2. set
   3. zset
4. 持久化和复制

# 基本的代码结构
这里首先就开始一部分的分析，这里先介绍一下redis的主体结构，文件的分类，服务的基本编译，测试和启动流程。

从目录结构和文件的组织形式，我感觉redis也做的非常好；任何一个软件项目，源码组织形式都是非常重要的，而且也能从侧面反映出软件的组织架构。所以这里也想介绍一下redis的目录结构和源码组织形式。

解压redis源码压缩包之后，主要的目录有这样几个：deps，src，utils，tests。基本上看目录名就知道做什么的了。
deps主要是redis以来的一些第三方库:
``` sh
HELIGHTXU-MB0:redis-5.0.3 helightxu$ ls deps/
Makefile           README.md          hiredis            jemalloc           linenoise          lua   update-jemalloc.sh
HELIGHTXU-MB0:redis-5.0.3 helightxu$ 
```
hiredis是redis的一个c客户端库，jemalloc是内存池库，linenoise是一个文本行读取解析库，lua文件夹内是lua5.1的一个c实现。
utils主要放一些工具包，直接看里面是有hashtable，hyperloglog，lru等，不过从我读代码的过程来看，这里面的内容貌似用的很少。
tests主要是针对redis的测试脚本。
src就是redis的全部源码了，这里面的代码组织也是非常简洁的，至少代码阅读起来比较便利

## 服务启动过程
## 编译运行

解压之后直接进入首目录，直接make即可，redis的代码很规范，编译过程也写的非常好，很流程，包括测试用例。

可以进入src目录，使用lldb（我使用mac，在mac上gdb使用起来不是很方便）启动redis-server这个程序，再在另外一个终端中启动客户端redis-cli，发送指令和server进行交互和调用过程分析。
``` sh
HELIGHTXU-MB0:src helightxu$ sudo lldb ./redis-server 
Password:
(lldb) target create "./redis-server"
Current executable set to './redis-server' (x86_64).
(lldb) b readQueryFromClient
Breakpoint 1: where = redis-server`readQueryFromClient + 17 at networking.c:1508, address = 0x0000000100019e91
(lldb) r
Process 33656 launched: './redis-server' (x86_64)
33656:C 06 Mar 2019 18:04:46.253 # oO0OoO0OoO0Oo Redis is starting oO0OoO0OoO0Oo
33656:C 06 Mar 2019 18:04:46.253 # Redis version=5.0.3, bits=64, commit=00000000, modified=0, pid=33656, just started
33656:C 06 Mar 2019 18:04:46.253 # Warning: no config file specified, using the default config. In order to specify a config file use ./redis-server /path/to/redis.conf
33656:M 06 Mar 2019 18:04:46.254 * Increased maximum number of open files to 10032 (it was originally set to 256).
   _._           
           _.-``__ ''-._      
      _.-``    `.  `_.  ''-._           Redis 5.0.3 (00000000/0) 64 bit
  .-`` .-```.  ```\/    _.,_ ''-._         
 (    '      ,       .-`  | `,    )     Running in standalone mode
 |`-._`-...-` __...-.``-._|'` _.-'|     Port: 6379
 |    `-._   `._    /     _.-'    |     PID: 33656
  `-._    `-._  `-./  _.-'    _.-'         
 |`-._`-._    `-.__.-'    _.-'_.-'|        
 |    `-._`-._        _.-'_.-'    |           http://redis.io        
  `-._    `-._`-.__.-'_.-'    _.-'         
 |`-._`-._    `-.__.-'    _.-'_.-'|        
 |    `-._`-._        _.-'_.-'    |        
  `-._    `-._`-.__.-'_.-'    _.-'         
      `-._    `-.__.-'    _.-'
          `-._        _.-'    
 `-.__.-'        

33656:M 06 Mar 2019 18:04:46.255 # Server initialized
33656:M 06 Mar 2019 18:04:46.255 * Ready to accept connections
Process 33656 stopped
```
这里看到我在readQueryFromClient这个函数上设置了一个断点，主要是想看服务端接受客户端请求时的处理过程。后面基本的调试方式都是使用上面类似的方式，在某个函数上设置断点，在用客户端去链接服务端，发送命令分析服务端的执行过程。这样分析起来还是非常方便的。

下面时客户端发起链接之后的一个处理过程，在调用这个函数的时候就停下来了。
``` sh
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.1
    frame #0: 0x0000000100019e91 redis-server`readQueryFromClient(el=0x000000010052f240, fd=8, privdata=0x000000010200be00, mask=1) at networking.c:1508
   1505	     * at the risk of requiring more read(2) calls. This way the function
   1506	     * processMultiBulkBuffer() can avoid copying buffers to create the
   1507	     * Redis Object representing the argument. */
-> 1508	    if (c->reqtype == PROTO_REQ_MULTIBULK && c->multibulklen && c->bulklen != -1
   1509	        && c->bulklen >= PROTO_MBULK_BIG_ARG)
   1510	    {
   1511	        ssize_t remaining = (size_t)(c->bulklen+2)-sdslen(c->querybuf);
Target 0: (redis-server) stopped.
```
再可以使用bt来看堆栈来整个调用过程。采用这种方式来
``` sh
(lldb) bt
* thread #1, queue = 'com.apple.main-thread', stop reason = breakpoint 1.1
  * frame #0: 0x0000000100019e91 redis-server`readQueryFromClient(el=0x000000010052f240, fd=8, privdata=0x000000010200be00, mask=1) at networking.c:1508
    frame #1: 0x00000001000046bc redis-server`aeProcessEvents(eventLoop=0x000000010052f240, flags=11) at ae.c:443
    frame #2: 0x00000001000049db redis-server`aeMain(eventLoop=0x000000010052f240) at ae.c:501
    frame #3: 0x00000001000105be redis-server`main(argc=1, argv=0x00007ffeefbffb88) at server.c:4197
    frame #4: 0x00007fff5d3e108d libdyld.dylib`start + 1
    frame #5: 0x00007fff5d3e108d libdyld.dylib`start + 1
(lldb) 

```
以上是简答的调试，那么接下来我想想从redis的启动上做一个简单的分析，之后在对其基本网络模型进行分析。

# redis的核心
看了redis的代码，发现整个redis都是围绕一个数据结构来的，这个结构就是`struct redisServer server`，这个结构是整个redis的核心，不夸张的说，只要看懂了整个数据结构基本上redis的代码也就看到了一大半了，所以这里介绍的时候，首先介绍一下整个数据结构。

在redis中server这个结构体变量是一个全局的变量，有点单例的感觉，全局只有一个。这也是C语言中的一个特点吧，在server.h文件是把server这变量设置为extern的。redis的定义是在server.c中的。server这个变量是redis中最终要的一个结构体了，这个结构体也非常庞大，同样是定义在server.h这个文件中
``` c
// server.c 70行之后       
/* Global vars */     
struct redisServer server; /* Server global state */         
volatile unsigned long lru_clock; /* Server global current LRU time. */  
```
server.h中的定义。
``` c
// server.h 928行之后
struct redisServer {
    /* General */
    pid_t pid;     /* Main process pid. */
    char *configfile;           /* Absolute config file path, or NULL */
    char *executable;           /* Absolute executable file path. */
    char **exec_argv;           /* Executable argv vector (copy). */
    int dynamic_hz;/* Change hz value depending on # of clients. */
    int config_hz; /* Configured HZ value. May be different than
         the actual 'hz' field value if dynamic-hz
         is enabled. */
    int hz;        /* serverCron() calls frequency in hertz */
    redisDb *db;
    dict *commands;/* Command table */
    dict *orig_commands;        /* Command table before command renaming. */
    aeEventLoop *el;
    ...
}
// ...
// server.h 1368行之后
/*-----------------------------------------------------------------------------
 * Extern declarations
 *----------------------------------------------------------------------------*/

extern struct redisServer server;
extern struct sharedObjectsStruct shared;
```
redisServer这个结构体我大概看了一下，成员变量在5.0.3这个版本中大概有300多个成员，非常庞大呀。因为redis中的注释做的非常好，所以这个结构体中大部分的成员都有注释的，所以现对都好理解，我这就不过多介绍了。

# redis启动过程
redis的main函数是在src的server.c这个文件中，在文件的最后面，所以它的启动就可以从这个点开始。

从main函数中看来，redis启动的过程可以分为一下9个过程
1. 基本设置
2. 配置初始化
3. module初始化
4. 哨兵设置
5. 从配置文件加载配置
6. deamon
7. 初始化服务
8. 启动检测，加载模块和数据
9. 最后进入事件循环阶段，服务启动等待客户端连接，并根据命令字处理

## 1. 基本设置
main函数开始是一些基本的设置，包括了环境变量设置，时区设置，hash种子设置，还有一个哨兵模式的设置，这个设置是只要参数中包含了sentinel这个关键字，就会把redis设置为哨兵模式。哨兵模式主要是针对redis高可用中主备的监控和自动切换。
``` c
int main(int argc, char **argv) {      
    struct timeval tv;    
    int j;       
    /* We need to initialize our libraries, and the server configuration. */  
#ifdef INIT_SETPROCTITLE_REPLACEMENT   
    spt_init(argc, argv); 
#endif       
    setlocale(LC_COLLATE,"");          
    tzset(); /* Populates 'timezone' global. */     
    zmalloc_set_oom_handler(redisOutOfMemoryHandler);            
    srand(time(NULL)^getpid());        
    gettimeofday(&tv,NULL); 
    char hashseed[16];
    getRandomHexChars(hashseed,sizeof(hashseed));            
    dictSetHashFunctionSeed((uint8_t*)hashseed);
    server.sentinel_mode = checkForSentinelMode(argc,argv);  
    ...
```

## 2.配置初始化
接下来主要的还有就是配置初始化设置：`initServerConfig()`。`initServerConfig`这个函数中的大部分作用就是给`server`这个变量中的成员赋值，算是配置参数初始化为默认值。比如时间，配置文件地址，配置的服务模式，日志级别，复制模式，基本命令，集群配置，落地方式等等等等。

``` c
void initServerConfig(void) {      
    int j;            
         
    pthread_mutex_init(&server.next_client_id_mutex,NULL);   
    pthread_mutex_init(&server.lruclock_mutex,NULL);         
    pthread_mutex_init(&server.unixtime_mutex,NULL);         
         
    updateCachedTime();            
    getRandomHexChars(server.runid,CONFIG_RUN_ID_SIZE);      
    server.runid[CONFIG_RUN_ID_SIZE] = '\0';    
    changeReplicationId();         
    clearReplicationId2();         
    server.timezone = getTimeZone(); /* Initialized by tzset(). */        
    server.configfile = NULL;      
    server.executable = NULL;      
    server.hz = server.config_hz = CONFIG_DEFAULT_HZ;        
...
```
这里参数太多了看看就可以了，这里都是设置服务参数的默认值，相当于是在做参数初始化。

## 3. module初始化
接下来还有一个`moduleInitModulesSystem()`，redis有个`module`扩展机制，实际上各种数据结构都是按照这个规范来接入到redis服务里面的，当然我们也可以扩展自己的数据结构和命令字。后面有时间也想测试一下这个看怎么按照一些业务特点来扩展功能。

这里列出了关键函数，可以看出通过这个初始化，它把已有的redis的api都注册到了server上了。
``` c
// module.c
int moduleRegisterApi(const char *funcname, void *funcptr) { 
    return dictAdd(server.moduleapi, (char*)funcname, funcptr);           
}        
         
#define REGISTER_API(name) \       
    moduleRegisterApi("RedisModule_" #name, (void *)(unsigned long)RM_ ## name) 
...
void moduleInitModulesSystem(void) {            
    moduleUnblockedClients = listCreate();      
    server.loadmodule_queue = listCreate();     
    modules = dictCreate(&modulesDictType,NULL);
    ...     
    moduleRegisterCoreAPI();    // 这里注册了已有的module结构
    ...          
}    
...
/* Register all the APIs we export. Keep this function at the end of the      
 * file so that's easy to seek it to add new entries. */         
void moduleRegisterCoreAPI(void) {     
    server.moduleapi = dictCreate(&moduleAPIDictType,NULL);      
    REGISTER_API(Alloc);    
    ...           
    REGISTER_API(ZsetAdd);         
    REGISTER_API(ZsetIncrby);      
    REGISTER_API(ZsetScore);       
    ...           
    REGISTER_API(ZsetRangeEndReached);          
    REGISTER_API(HashSet);         
    REGISTER_API(HashGet); 
    ...
}

```
## 4. 哨兵设置
接下来在main函数中比较重要的就是根据上面哨兵的配置，进行哨兵模式的初始化设置。
``` c
    /* We need to init sentinel right now as parsing the configuration file            
     * in sentinel mode will have the effect of populating the sentinel   
     * data structures with master nodes to monitor. */      
    if (server.sentinel_mode) {    
        initSentinelConfig();      
        initSentinel();            
    }      
```
## 5. 从配置文件加载配置
接下来比较重要的就是加载配置文件`loadServerConfig`，并且根据配置文件中的配置，对服务器参数进行更新。这块的代码较多，现对容易读，就不过多解析了。因为这里面就是一堆的ifelse语句。
## 6. deamon化
redis的deamon函数写的也是非常小巧，如果有需要写后台服务的同学可以借鉴一下。
``` c
void daemonize(void) {
    int fd;           

    if (fork() != 0) exit(0); /* parent exits */    
    setsid(); /* create a new session */            

    /* Every output goes to /dev/null. If Redis is daemonized but
     * the 'logfile' is set to 'stdout' in the configuration file
     * it will not log at all. */      
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);        
        dup2(fd, STDOUT_FILENO);   
        dup2(fd, STDERR_FILENO);   
        if (fd > STDERR_FILENO) close(fd);      
    }    
}  
```
## 7. 初始化服务
再下来也是非常重要的一个函数`initServer`，这个函数中是对Server进行启动前的初始化工作，这个函数就正式到服务的启动阶段了，主要有一下一些事情：
1. 服务信号处理函数设置
2. 一些服务和客户端参数的初始化
3. 创建共享对象，调整打开文件限制
4. 创建事件驱动循环处理器
5. 端口监听，
6. 数据库文件初始化，
7. 状态、统计数据初始化，
8. 定时器事件添加
9. tcp连接事件处理器添加
10. 管道事件处理器添加，设置最大内存，默认是3G。
11. 复制初始化等等。
12. 一些lua虚拟机和脚本添加
13. 一些后台任务线程创建
14. 其它慢日志等初始化
redis的事件驱动框架也是redis中非常重要的一个设计，下次分析网络的时候会梳理这块。

## 8. 启动检测，加载模块和数据
这里如果设置了哨兵模式，则会启动哨兵服务进程；在非哨兵模式下会加载服务module和数据。这里比较简单就不多介绍了。

## 9. 最后进入事件循环阶段，服务启动等待客户端连接，并根据命令字处理
最后就是做一些检测，启动哨兵，进入网络事件监听状态了，在`aeMain`中就进入事件驱动的网络事件处理了，在监听到网络请求之后就在这个主循环中进行处理了。
``` c
// server.c 4194行之后
    ...       
    aeSetBeforeSleepProc(server.el,beforeSleep);    
    aeSetAfterSleepProc(server.el,afterSleep);      
    aeMain(server.el);    
    aeDeleteEventLoop(server.el);      
    return 0;
}        
         
/* The End */    
```

# 总结
以上对redis的启动代码做了一个初步的分析，只是梳理出来redis的基本启动过程，但是每个参数、事件驱动、哨兵等机制和功能到底是怎么样的，还没有具体的分析，这个放到后面逐步分析。看了redis的代码总体的感觉有2个：1）代码整体上非常整洁，读起来非常舒服，模块的设计也非常到位，阅读理解相对没有那么难。2）代码中有大量的注释，这个更加方便了代码阅读。

今年上半年的计划就是在工作之余能够挤点事件把redis的代码梳理梳理，下次应该画几个脑图就更能帮助理解了。


<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)
</center>