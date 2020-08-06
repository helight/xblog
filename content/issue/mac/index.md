
+++
title = "mac 使用问题记录"
description = "helight"
type = "about"
date = "2019-02-28"
+++

### 解决 MacBook Touch Bar esc 图标消失不见的问题

今天在用 vim 编辑器写文件写的好好的，想退出保存时，发现 Touch Bar 上的 esc 图标消失不见了，本文分享下出现这个问题的原因和解决办法（重新让 esc 回到 Touch Bar 上）。

MacBook Pro 版本：macOS Catalina Version 10.15.5

解决办法也很简单，直接打开终端（Terminal），杀死 Touch Bar 服务即可：
``` console
sudo pkill TouchBarServer
```

粘贴上面的命令拷贝到终端直接运行，之后输入密码确认执行，等待 3-5 秒，esc 图标就重新回来了。