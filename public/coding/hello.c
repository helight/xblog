/*
 * Copyright (c) 2009-~ Helight.Xu
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License
 *
 * Author:       Helight.Xu<Helight.Xu@gmail.com>
 * Created Time: Sun 06 Sep 2009 10:14:06 AM CST
 * File Name:    hello.c
 *
 * Description:  
 */

#include <linux/init.h>
#include <linux/module.h>


static __init int hello_init(void)
{
	printk("hello world\n");
	return 0;
}

static __exit void hello_exit(void)
{
	printk("bey world\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("http://zhwen.org");
