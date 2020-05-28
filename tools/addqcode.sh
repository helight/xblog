#!/bin/sh
# Copyright (c) 2020, HelightXu
# Author: Zhwen Xu<HelightXu@gmail.com>
# Created: 2020-05-28
# Description:
#

mdfile_list=`find . -name "*.md" -print`

for item in $mdfile_list ; do
    #echo $content >> $item
    echo $item
    cat >>$item<<EOF


<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
EOF
done
