+++
title = "Redis的事件处理模型2：客户端命令执行过程"
date = "2019-06-18T13:47:08+02:00"
tags = ["redis", "db"]
categories = ["redis"]
banner = "img/banners/redis.png"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "Redis的事件处理模型2：客户端命令执行过程"
keywords = ["redis","事件驱动", "client"]
+++

# Redis的事件处理模型2：客户端命令执行过程

# 前言

上篇分析了Redis的事件处理模型，了解了Redis的事件处理基本过程，这篇还想继续顺着上面的思路分析一下这种情况：在客户端向服务端发起一个set指令或者get指令后，服务端接收后怎么执行这个命令，这个命令操作的简单过程是怎么样的。在分析了这个过程之后，我们就知道了Redis客户端和服务端交互的整个过程。清楚交互过程之后，我再会分析每个数据类型的结构和原理。所以今天继续分析Redis的事件处理模型。

# redis客户端命令处理分析
## 命令字解析
还是从上一篇分析的延续下来，我们从服务端接收到客户端请求开始，从`createClient`分析起来。下面的代码就是`createClient`中创建初始化客户端c的一些参数。
``` c
    c->reqtype = 0; // 请求类型,因为Redis服务器支持Telnet的连接，因此Telnet命令请求协议类型是PROTO_REQ_INLINE，而redis-cli命令请求的协议类型是PROTO_REQ_MULTIBULK。
    c->argc = 0;    // 客户端传参的个数
    c->argv = NULL; // 传的参数
    c->cmd = c->lastcmd = NULL;       // 请求命令  
    c->multibulklen = 0;      
    c->bulklen = -1;
    c->sentlen = 0; 
    c->flags = 0;   // 客户端状态的标志。中在server.h中定义了29种客户端状态。
```
接下来我们持续跟进这个函数的执行，`readQueryFromClient`是绑定在每个客户端链接fd上的，这个函数的主要目标就是读取客户端传来的数据。看这函数的主干调用关系是下面的顺序:
readQueryFromClient -> processInputBufferAndReplicate -> processInputBuffer -> processCommand
在`processInputBuffer`函数中还根据请求类型进行数据读取处理，`processMultibulkBuffer`是针对多个命令请求的数据读取解析，并且放到命令字段argv中。下面的代码是`processInputBuffer`的。
``` c
if (c->reqtype == PROTO_REQ_INLINE) {         
            if (processInlineBuffer(c) != C_OK) break;
        } else if (c->reqtype == PROTO_REQ_MULTIBULK) {   
            if (processMultibulkBuffer(c) != C_OK) break;    // 这里读取buff中的数据，并且根据argc来填写argv数据                 
        } else {               
            serverPanic("Unknown request type");      
        }   
        
        /* Multibulk processing could see a <= 0 length. */                  
        if (c->argc == 0) {    
            resetClient(c);    
        } else {               
            /* Only reset the client when the command was executed. */       
            if (processCommand(c) == C_OK) {          // 这里正真处理命令
                if (c->flags & CLIENT_MASTER && !(c->flags & CLIENT_MULTI)) {
 /* Update the applied replication offset of our master. */   
 c->reploff = c->read_reploff - sdslen(c->querybuf) + c->qb_pos;                 
                }              
      
```
在`processMultibulkBuffer`中可以看到是根据argc的个数量而生成响应数量的命令结构体`redisObject`。这个数据结构是redis中非常重要的一个数据结构，这里主要用来封装命令字段。关于这个命令字段，我后面会进行介绍。
``` c
            /* Optimization: if the buffer contains JUST our bulk element    
             * instead of creating a new object by *copying* the sds we      
             * just use the current sds string. */   
            // 如果读入的长度大于32k 
            if (c->qb_pos == 0 &&  
                c->bulklen >= PROTO_MBULK_BIG_ARG &&  
                sdslen(c->querybuf) == (size_t)(c->bulklen+2))               
            {                  
                c->argv[c->argc++] = createObject(OBJ_STRING,c->querybuf);    // 这里命令结构生成， 创建对象保存在client的参数列表中
                // 跳过换行
                sdsIncrLen(c->querybuf,-2); /* remove CRLF */                
                /* Assume that if we saw a fat argument we'll see another one
                 * likely... */
                // 设置一个新长度
                c->querybuf = sdsnewlen(SDS_NOINIT,c->bulklen+2);            
                sdsclear(c->querybuf);                
            } else {           
                c->argv[c->argc++] =                  
 createStringObject(c->querybuf+c->qb_pos,c->bulklen);    
                c->qb_pos += c->bulklen+2;            
            }                  
            c->bulklen = -1;   
            c->multibulklen--;
```
## 命令字执行
从这里开始，我们开始解析`processCommand`函数，从上面的函数，把从buff中把命令字都解析出来，放到了了argv命令数组中，接下来就要去执行这个命令了。让我们看看是怎么执行的。
### 先来看一个简单的：
``` c
int processCommand(client *c) {
    /* The QUIT command is handled separately. Normal command procs will     
     * go through checking for replication and QUIT will cause trouble       
     * when FORCE_REPLICATION is enabled and would be implemented in         
     * a regular command proc. */  
    if (!strcasecmp(c->argv[0]->ptr,"quit")) {      // 如果是quit命令，  
        addReply(c,shared.ok);          // 设置回复信息
        c->flags |= CLIENT_CLOSE_AFTER_REPLY;         // 设置关闭客户端标志
        return C_ERR;          
    }       
```
quit命令是比较特殊的，直接退出客户端的链接，所以这里单独处理。
### 一般命令执行
``` c
    /* Now lookup the command and check ASAP about trivial error conditions  
     * such as wrong arity, bad command name and so forth. */                
    c->cmd = c->lastcmd = lookupCommand(c->argv[0]->ptr);   //  查询获取具体的命令函数，并拼接参数。                 
    if (!c->cmd) {             
    ......
    /* Exec the command */     
    if (c->flags & CLIENT_MULTI && 
        c->cmd->proc != execCommand && c->cmd->proc != discardCommand &&     
        c->cmd->proc != multiCommand && c->cmd->proc != watchCommand)        
    {   
        queueMultiCommand(c);  
        addReply(c,shared.queued); 
    } else {
        call(c,CMD_CALL_FULL); // 经过上面的异常检查之后，这里才开始正真执行命令
        c->woff = server.master_repl_offset;          
        if (listLength(server.ready_keys))            
            handleClientsBlockedOnKeys();             
    }   
    return C_OK;        
```
在call中执行的时候的时候，其实是根据`lookupCommand`函数返回的cmd结构体来执行的。这里面把命令字比如`get`，转换为它真实要调用的函数，把函数和参数都封装到一个struct中，最后在直接调用执行这个cmd。cmd的结构体如下。
``` c
typedef void redisCommandProc(client *c);             
typedef int *redisGetKeysProc(struct redisCommand *cmd, robj **argv, int argc, int *numkeys);       
struct redisCommand {          
    char *name;                
    redisCommandProc *proc;    
    int arity;                 
    char *sflags; /* Flags as string representation, one char per flag. */   
    int flags;    /* The actual flags, obtained from the 'sflags' field. */  
    /* Use a function to determine keys arguments in a command line.         
     * Used for Redis Cluster redirect. */            
    redisGetKeysProc *getkeys_proc;
    /* What keys should be loaded in background when calling this command? */
    int firstkey; /* The first argument that's a key (0 = no keys) */        
    int lastkey;  /* The last argument that's a key */
    int keystep;  /* The step between first and last key */                  
    long long microseconds, calls; 
}; 
```

那么怎么把`get`转成装成它所对应的的函数呢？主要就是`lookupCommand`来执行的，那么再来看看这个函数的执行。
``` c
struct redisCommand *lookupCommand(sds name) {        
    return dictFetchValue(server.commands, name);     
    // 这里看到了server.commands，server是全局变量，所以commands也是在服务启动的时候初始化的
} 
...
dictEntry *dictFind(dict *d, const void *key)         
{       
    dictEntry *he;             
    uint64_t h, idx, table;    
        
    if (d->ht[0].used + d->ht[1].used == 0) return NULL; /* dict is empty */ 
    if (dictIsRehashing(d)) _dictRehashStep(d);       
    h = dictHashKey(d, key);   
    for (table = 0; table <= 1; table++) {       // 这里对dict中的连个hash表都进行了搜索     
        idx = h & d->ht[table].sizemask;              
        he = d->ht[table].table[idx];                 
        while(he) {            
            if (key==he->key || dictCompareKeys(d, key, he->key))            
                return he;     
            he = he->next;     
        }   
        if (!dictIsRehashing(d)) return NULL;         
    }   
    return NULL;               
}   
void *dictFetchValue(dict *d, const void *key) {      
    dictEntry *he;             
        
    he = dictFind(d,key);      
    return he ? dictGetVal(he) : NULL;                
} 
```
`dict`这个结构体是什么样的呢？这个结构体也是Redis中非常重要的一个结构体，基本的kv存储就是使用这个结构体，从上面的调用关系可以看出最终是查找`dictht`中的`table`散列数组，所以可以预想到，在Redis中初始化的时候必然要初始化这个结构体的。
``` c
typedef struct dict {          
    dictType *type;      // 字典类型      
    void *privdata;      // 私有数据    
    dictht ht[2];        // 一个字典中有两个哈希表     
    long rehashidx; /* rehashing not in progress if rehashidx == -1 */   // 数据动态迁移的下标位置    
    unsigned long iterators; /* number of iterators currently running */ // 当前正在使用的迭代器的数量  
} dict;   
// 哈希表结构 
typedef struct dictht {
    dictEntry **table;  // 散列数组。
    unsigned long size; // 散列数组的长度
    unsigned long sizemask; // sizemask等于size减1
    unsigned long used;// 散列数组中已经被使用的节点数量
} dictht;
```
### 命令字注册初始化
在`initServerConfig`中可以看到由下面的代码：
``` c
/* Command table -- we initiialize it here as it is part of the   
     * initial configuration, since command names may be changed via  
     * redis.conf using the rename-command directive. */    
    server.commands = dictCreate(&commandTableDictType,NULL);         
    server.orig_commands = dictCreate(&commandTableDictType,NULL);    
    populateCommandTable();  // 这里初始化基本redis支持的数据命令
```
再来看`populateCommandTable`这个函数中的关键点
``` c
/* Populates the Redis Command Table starting from the hard coded list  
 * we have on top of redis.c file. */       
void populateCommandTable(void) {           
    int j;      
    int numcommands = sizeof(redisCommandTable)/sizeof(struct redisCommand);          
  
    for (j = 0; j < numcommands; j++) {     
        struct redisCommand *c = redisCommandTable+j;     // redisCommandTable命令字数组
        char *f = c->sflags;  
        int retval1, retval2; 
......
        // 把这个cmd添加到`server.commands`中。
        retval1 = dictAdd(server.commands, sdsnew(c->name), c);         
        /* Populate an additional dictionary that will be unaffected    
         * by rename-command statements in redis.conf. */ 
        retval2 = dictAdd(server.orig_commands, sdsnew(c->name), c);    
        serverAssert(retval1 == DICT_OK && retval2 == DICT_OK);         
    }           
}   
```
上面到`redisCommandTable`是一个静态输出化的结构体，里面的内容也比较简单。首先是name，接下来是函数的名字等。
``` c
struct redisCommand redisCommandTable[] = { 
    {"module",moduleCommand,-2,"as",0,NULL,0,0,0,0,0},    
    {"get",getCommand,2,"rF",0,NULL,1,1,1,0,0},           
    {"set",setCommand,-3,"wm",0,NULL,1,1,1,0,0},          
    {"setnx",setnxCommand,3,"wmF",0,NULL,1,1,1,0,0},      
    {"setex",setexCommand,4,"wm",0,NULL,1,1,1,0,0},       
    {"psetex",psetexCommand,4,"wm",0,NULL,1,1,1,0,0}, 
```
这里我们以`get`为例来看看它的具体函数。
``` c
// t_string.c
int getGenericCommand(client *c) {         
    robj *o;            
    // 这里传入的是argv[1]，进行值的查询 
    if ((o = lookupKeyReadOrReply(c,c->argv[1],shared.nullbulk)) == NULL)        
        return C_OK;    
     
    if (o->type != OBJ_STRING) {           
        addReply(c,shared.wrongtypeerr);   
        return C_ERR;   
    } else {            
        addReplyBulk(c,o);   // 返回查询结果              
        return C_OK;    
    }
} 
void getCommand(client *c) {               
    getGenericCommand(c);                  
}   
```
可以看出这里查询的就是c指向的db，db是`redisDb`这个结构体，这个结构体是Redis中保存数据的结构。这个结构我们从下次开始分析。
``` c
robj *lookupKeyRead(redisDb *db, robj *key) {                 
    return lookupKeyReadWithFlags(db,key,LOOKUP_NONE);        
}       
robj *lookupKeyReadOrReply(client *c, robj *key, robj *reply) {                  
    robj *o = lookupKeyRead(c->db, key);   
    if (!o) addReply(c,reply);             
    return o;           
}   
```

# 总结
分析到这里就可以看出了，Redis的命令字执行过程也是非常清晰的，在事件框架中接收到客户端的请求之后就进行数据的读取，读取之后再把它按照具体情况转换为cmd，最终执行这个cmd。在redis中所有已经支持的命令字都是事先初始化到`server.commonds`这个结构体中。在接收到命令字之后在这个结构体中查找具体的执行函数，再来执行。

而且从上面3篇的分析，我们看到Redis的事件处理始终是单进程中处理的，也没有多线程处理。只有在第一篇中介绍的时候说到在`initServer`函数最后面有创建一些后台的bio任务，这些任务是以线程的方式启动的。而整个redis的主要服务都是单线程处理的。

<center>
看完本文有收获？请分享给更多人

关注「黑光技术」，关注大数据+微服务

![](/img/qrcode_helight_tech.jpg)
</center>