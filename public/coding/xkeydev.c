#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/irq.h>
#include <linux/poll.h>
#include <asm/uaccess.h>
#include <linux/interrupt.h>
#include <linux/devfs_fs_kernel.h>
#include <asm/arch/hardware.h>
#include <asm/arch/regs-gpio.h>

#define xdebug

#ifdef xdebug
#define xdebug_p(x...) printk(x)
#else
#define xdebug_p(x...)
#endif

#define DEVICE_NAME "xkeydev"

static DECLARE_WAIT_QUEUE_HEAD(waitq_key);
static int major = 0;
static volatile char key_value = 0;
static volatile int press_key = 0;

struct xkey_info{
	int irq;
	int pin;
	int stat;
	int num;
	char *name;
};

static struct xkey_info xkey_if[] = {
	{IRQ_EINT8, S3C2410_GPG0, S3C2410_GPG0_EINT8, 0, "key0"},
	{IRQ_EINT11, S3C2410_GPG3, S3C2410_GPG3_EINT11, 1, "key1"},
	{IRQ_EINT13, S3C2410_GPG5, S3C2410_GPG5_EINT13, 2, "key2"},
	{IRQ_EINT14, S3C2410_GPG6, S3C2410_GPG6_EINT14, 3, "key3"},
	{IRQ_EINT15, S3C2410_GPG7, S3C2410_GPG7_EINT15, 4, "key4"},
	{IRQ_EINT19, S3C2410_GPG11, S3C2410_GPG11_EINT19, 5, "key5"},
};

static irqreturn_t xkey_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	int ret = 0;
	struct xkey_info *xkey_info = (struct xkey_info *)dev_id;
	
	ret = s3c2410_gpio_getpin(xkey_info->pin);
	if(!ret)
		key_value = (char)xkey_info->num + 1;
	else
		key_value = 0;

	press_key = 1;

	wake_up_interruptible(&waitq_key);
	
	xdebug_p("button interrrupt:%d : %02x key value: %02x\n",
			xkey_info->irq, ret, key_value);
	return IRQ_HANDLED;
}

static int xkey_open(struct inode *inode, struct file *file)
{
	int i = 0, ret = 0;
	
	for (i = 0; i < 6; i++) {
		s3c2410_gpio_cfgpin(xkey_if[i].pin, xkey_if[i].stat);
		ret = request_irq(xkey_if[i].irq, &xkey_interrupt, SA_SHIRQ,
				 xkey_if[i].name, (void *)&xkey_if[i]);
		if(ret)
			break;
	}
	if (ret) {
		for (i = 0; i < 6; i++) {
			disable_irq(xkey_if[i].irq);
			free_irq(xkey_if[i].irq, (void *)&xkey_if[i]);
		}
		return -1;
	}
	xdebug_p("button open\n");
	return 0;
}

static int xkey_close(struct inode *inode, struct file *file)
{
	int i = 0;

	for (i = 0; i < 6; i++) {
		disable_irq(xkey_if[i].irq);
		free_irq(xkey_if[i].irq, &xkey_if[i]);
	}
	xdebug_p("button release\n");
	return 0;
}

static ssize_t xkey_read(struct file *file, char *buffer, size_t count, loff_t *loff)
{
	int ret = 0;

	if (!press_key) {
		if(file->f_flags & O_NONBLOCK)
			return -EAGAIN;
		else
			wait_event_interruptible(waitq_key, press_key);
	}
	press_key = 0;
	ret = copy_to_user(buffer, (const void *)&key_value, 1);
	xdebug_p("button read: %02x\n", key_value);
	key_value = 0;
	return ret ? -EFAULT : 1;
}

static unsigned int xkey_poll(struct file *file, struct poll_table_struct *table)
{
	unsigned int mask = 0;

	poll_wait(file, &waitq_key, table);
	if (press_key)
		mask |= POLLIN | POLLRDNORM;
	xdebug_p("button poll\n");
	return mask;
}

static ssize_t xkey_write(struct file *file, const char *buffer, size_t count, loff_t *loff)
{
	xdebug_p("button write\n");
	return 0;
}

static struct file_operations xkeydev_op = {
	.owner		= THIS_MODULE,
	.open		= xkey_open,
	.release	= xkey_close,
	.read		= xkey_read,
	.poll		= xkey_poll,
	.write		= xkey_write,
};

static int __init button_init(void)
{
	int ret = 0;
	
	ret = register_chrdev(0, DEVICE_NAME, & xkeydev_op);
	if (ret < 0) {
		printk(DEVICE_NAME" can't register!!!!\n");
		return ret;
	}
	major = ret;
	devfs_mk_cdev(MKDEV(major, 0), S_IFCHR | 0666, DEVICE_NAME);

	
	xdebug_p("button init\n");
	return ret;
}

static void __exit button_exit(void)
{
	devfs_remove(DEVICE_NAME);
	unregister_chrdev(major, DEVICE_NAME);
	xdebug_p("button exit\n");
	return;
}

module_init(button_init);
module_exit(button_exit);


MODULE_AUTHOR("Helight.Xu");
MODULE_LICENSE("Dual BSD/GPL");
