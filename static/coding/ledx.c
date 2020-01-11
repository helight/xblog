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
#include <linux/delay.h>
#include <asm/io.h>
#include <mach/hardware.h>
//#include <mach/platform.h>
//#include <mach/lpc32xx_gpio.h>

#define p3_out1		io_p2v(0x40028004)
#define p3_out0		io_p2v(0x40028008)

static __init int hello_init(void)
{
	int i = 0, tmp = 20;

	while (tmp) {
		__raw_writel((1 << 5), p3_out1);
		for (i = 100; i > 0; i--)
			udelay(1000);

		__raw_writel((1 << 5), p3_out0);
		for (i = 100; i > 0; i--)
			udelay(1000);
		tmp--;
	};

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
