+++
title = "使用proc来写GPIO驱动"
date = "2009-03-31T13:47:08+02:00"
tags = ["linux", "开源", "kernel"]
categories = ["programming"]
banner = "img/banners/banner-2.jpg"
draft = false
author = "helight"
authorlink = "https://helight.cn"
summary = "这是写的一个gpio的驱动，测试完，提好的。"
keywords = ["开源", "linux", "kernel", "proc", "gpio"]
+++

这是写的一个gpio的驱动，测试完，提好的。
``` c
#include <linux/kernel.h>                                                                                                                                                                                                                                                                                                
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <asm/arch/regs-gpio.h>
#include <asm/arch/leds-gpio.h>
#include <asm/io.h>


#define MODULE_NAME "led"
#define MAX_LEN 8


void __iomem *base = NULL;

struct led_stat {
	char name[MAX_LEN + 1];
	char value[MAX_LEN + 1];
	char stat[MAX_LEN];
};

static struct proc_dir_entry *led_dir, *led_file;
struct led_stat led_data;

static int
proc_read_led (char *page, char **stat,
		off_t off, int count, int *eof, void *data)
{
	int len;
	struct led_stat *p_data = (struct led_stat *) data;
	p_data->stat[0] = inb((void *)(base + 4));
	len = sprintf(page, "%02x", p_data->stat[0]);
	return len;
}

static int
proc_write_led (struct file *file,const char *buffer,
		unsigned long count,void *data)
{
	int len;
	char ch;
	struct led_stat *p_data = (struct led_stat *)data;
	if (count < MAX_LEN)
		len = count;
	else
		len = MAX_LEN;
       
	if(copy_from_user(p_data->stat, buffer, len))
		return -EFAULT;
	ch = buffer[0];
	printk("buff 0:%02x buff 1 :%02x\n ch :%02x \n", buffer[0], buffer[1], ch);
	if ((ch <= 0x39) && (ch >= 0x30))
		ch = ch - 0x30;
	else if ((ch <= 0x46) && (ch >= 0x41))
		ch = ch - 0x37;
	else if ((ch <= 0x66) && (ch >= 0x61))
		ch = ch - 0x57;
	else
		ch = 0x0f;
	ch = ch << 4;
	printk("buff kernel:%s ch :%02x\n", buffer, ch);

	outb(ch, (void *)(base + 4));
	return len;
}

static int __init led_proc_init(void)
{
	int rv = 0;

	led_dir = proc_mkdir(MODULE_NAME,NULL);
	if(led_dir == NULL){
		rv = -ENOMEM;
		goto out;
	}
	led_dir->owner = THIS_MODULE;
	led_file = create_proc_entry("led",0666,led_dir);
	if(led_file == NULL){
		rv = -ENOMEM;
		goto no_led;
	}
	strcpy(led_data.name,"led");
	strcpy(led_data.value,"led");
	memset(led_data.stat, '\0', sizeof(led_data.stat));
	led_file->data = &led_data;
	led_file->read_proc = proc_read_led;
	led_file->write_proc = proc_write_led;
	led_file->owner = THIS_MODULE;

	 printk(KERN_INFO "%s initialised\n",MODULE_NAME);
	 return 0;
no_led:
	 remove_proc_entry("led",led_dir);
	 remove_proc_entry(MODULE_NAME,NULL);
out:
	 return rv;
}
static void __exit led_proc_exit(void)
{
	remove_proc_entry("led",led_dir);
	remove_proc_entry(MODULE_NAME,NULL);
}

static int __init led_io_init(void)
{
	base = S3C24XX_GPIO_BASE(S3C2410_GPF4);
	s3c2410_gpio_cfgpin(S3C2410_GPF4, S3C2410_GPF4_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPF5, S3C2410_GPF5_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPF6, S3C2410_GPF6_OUTP);
	s3c2410_gpio_cfgpin(S3C2410_GPF7, S3C2410_GPF7_OUTP);
	outb(0x00, (void *)(base + 4));
       
	return 0;
}

static void __exit led_io_exit(void)
{
	outb(0xff, (void *)(base + 4));
       
}

static int __init led_init(void)
{
	led_io_init();
	led_proc_init();
       
	return 0;
}

static void __exit led_exit(void)
{
       
	led_proc_exit();
	led_io_exit();

	printk(KERN_INFO "%s removed\n",MODULE_NAME);
       
	return;
}

module_init(led_init);
module_exit(led_exit);

MODULE_AUTHOR("Helight.Xu");
MODULE_LICENSE("Dual BSD/GPL"); 

```

<center>
看完本文有收获？请分享给更多人<br>

关注「黑光技术」，关注大数据+微服务<br>

![](/img/qrcode_helight_tech.jpg)
</center>
