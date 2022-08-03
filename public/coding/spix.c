/*
 * Copyright (c) 2009-~ Helight.Xu
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License
 *
 * Author:       Helight.Xu<Helight.Xu@gmail.com>
 * Created Time: Sun 13 Sep 2009 05:12:13 PM CST
 * File Name:    spix.c
 *
 * Description:  
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>

#include "spix.h"

#define DEV_NAME "spix"

static int init_spix(void)
{
	__raw_writel((1 << 5), p2mux);
	__raw_writel((1 << 30), p2dir);
	disable_spi_flash();

	__raw_writel(((3 << 9) | (1 << 12)), spimux);
	__raw_writel(0x03, spix_clk);
	__raw_writel(0x01, spix_gobal);
	__raw_writel(0x03, spix_gobal);
	__raw_writel(0x01, spix_gobal);

	__raw_writel(((1 << 23) | (1 << 13) | (7 << 9) | (1 << 7) | 15), spix_con);
	__raw_writel(0x00, spix_ier);
	__raw_writel(((3 << 7) | (1 << 2) | 1), spix_stat);

	__raw_writel(0x00, spix_tim);

	return 0;
}

static int spix_open(struct inode *inode, struct file *fp)
{
	init_spix();
	try_module_get(THIS_MODULE);

	printk("open dev %s \n", DEV_NAME);
	return 0;
}

static int spix_close(struct inode *inode, struct file *fp)
{
	
	module_put(THIS_MODULE);
	printk("close dev %s \n", DEV_NAME);
	return 0;
}

static ssize_t spix_write(struct file *fp, const char __user *buff, size_t count, loff_t *pp)
{
	int i = 0, len = 0;
	
	len = strlen(buff);
	if (len < count)
		count = len;
	for (i = 0; i < count; i++) {
		send_byte((unsigned char)(*buff & 0xFF));
		printk("buff: %02x \n", *buff);
		buff++;
	}
	printk("write dev %s \n", DEV_NAME);
	return 0;
}

static ssize_t spix_read(struct file *fp, char __user *buff, size_t count, loff_t *pp)
{
	int i = 0;
	unsigned char ch = 0;

	for (i = 0; i < count; i++) {
		ch = send_byte(0xFF);
		*(buff++) = ch;
	}
	printk("read dev %s \n", DEV_NAME);
	return 0;
}

static struct file_operations spix_ops = {
	.owner		= THIS_MODULE,
	.open		= spix_open,
	.write		= spix_write,
	.read		= spix_read,
	.release	= spix_close,
};

static struct miscdevice spix_miscdev = {
	.minor		= MISC_DYNAMIC_MINOR,
	.name		= DEV_NAME,
	.fops		= &spix_ops,
};

static int lpc3250_spix_probe(struct platform_device *dev)
{
	int tmp = 0;

	tmp = misc_register(&spix_miscdev);
	printk("lpc3250_spix_probe\n");

	return tmp;
}

static int lpc3250_spix_remove(struct platform_device *dev)
{
	int tmp = 0;

	tmp = misc_deregister(&spix_miscdev);
	printk("lpc3250_spix_remove\n");

	return tmp;
}

static int lpc3250_spix_suspend(struct platform_device *dev, pm_message_t state)
{

	printk("lpc3250_spix_suspend\n");
	return 0;
}

static int lpc3250_spix_resume(struct platform_device *dev)
{

	printk("lpc3250_spix_resume\n");
	return 0;
}

static struct platform_driver lpc3250_spix = {
	.probe		= lpc3250_spix_probe,
	.remove		= lpc3250_spix_remove,
	.suspend	= lpc3250_spix_suspend,
	.resume		= lpc3250_spix_resume,
	.driver		= {
		.name	= DEV_NAME,
		.owner	= THIS_MODULE,
	},
};

static struct platform_device *lpc3250_spix_dev;

static int __init platest_init(void)
{
	int tmp = 0;
	
	lpc3250_spix_dev = platform_device_register_simple(DEV_NAME, -1, NULL, 0);
	if (IS_ERR(lpc3250_spix_dev))
		return -1;
	tmp = platform_driver_register(&lpc3250_spix);
	if (tmp < 0) {
		platform_device_unregister(lpc3250_spix_dev);
		return -1;
	}
	printk("platest_init!!!\n");
	return tmp;
}

static void __exit platest_exit(void)
{

	platform_driver_unregister(&lpc3250_spix);
	platform_device_unregister(lpc3250_spix_dev);
	printk("platest_exit!!!\n");
	return;
}

module_init(platest_init);
module_exit(platest_exit);

MODULE_AUTHOR("http://zhwen.org");
MODULE_LICENSE("Dual BSD/GPL");
