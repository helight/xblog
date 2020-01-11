#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/delay.h>
#include <linux/devfs_fs_kernel.h>
#include <asm/arch/hardware.h>
#include <asm/arch/regs-adc.h>

#define xdebug

#ifdef xdebug
#define xdebug_p(x...) printk(x)
#else
#define xdebug_p(x...)
#endif

#define DEVICE_NAME "xaddev"

static void __iomem *base_addr; 
static int major = 0;
/*
#define S3C2410_ADCCON     S3C2410_ADCREG(0x00)
#define S3C2410_ADCTSC     S3C2410_ADCREG(0x04)
#define S3C2410_ADCDLY     S3C2410_ADCREG(0x08)
#define S3C2410_ADCDAT0    S3C2410_ADCREG(0x0C)
#define S3C2410_ADCDAT1    S3C2410_ADCREG(0x10)

#define S3C24XX_VA_ADC     S3C2410_ADDR(0x01000000)
#define S3C2400_PA_ADC     (0x15800000)
#define S3C2410_PA_ADC     (0x58000000)
#define S3C24XX_SZ_ADC     SZ_1M

*/
static int xad_open(struct inode *inode, struct file *file)
{
	xdebug_p("ad open\n");
	return 0;
}

static int xad_close(struct inode *inode, struct file *file)
{
	xdebug_p("ad release\n");
	return 0;
}

static ssize_t xad_read(struct file *file, char *buffer, size_t count, loff_t *loff)
{
	int data;   
	unsigned long tmp;   

	//writel( 0xbfc1, base_addr+S3C2410_ADCCON);   
	tmp = readl(base_addr+S3C2410_ADCCON) | S3C2410_ADCCON_ENABLE_START;   
	xdebug_p("ad tmp conf %02x \n", tmp);
	writel( tmp, base_addr+S3C2410_ADCCON);   
	do{   
		tmp = readl(base_addr+S3C2410_ADCCON);   
	}while(!(((unsigned int)tmp)&0x8000));   
	data = readl(base_addr+S3C2410_ADCDAT0) & 0x3ff;   
        
	if(copy_to_user(buffer, &data, sizeof(int)))   
		return -EFAULT;   
	xdebug_p("ad read %02x\n", data);
	return (sizeof(int));   
}


static struct file_operations xaddev_op = {
	.owner		= THIS_MODULE,
	.open		= xad_open,
	.release	= xad_close,
	.read		= xad_read,
};

static int __init ad_init(void)
{
	int ret = 0;
	
	ret = register_chrdev(0, DEVICE_NAME, & xaddev_op);
	if (ret < 0) {
		printk(DEVICE_NAME" can't register!!!!\n");
		return ret;
	}
	major = ret;
	devfs_mk_cdev(MKDEV(major, 0), S_IFCHR | 0666, DEVICE_NAME);

	base_addr=ioremap(S3C2410_PA_ADC,4);   
	if (base_addr == NULL){   
		printk(KERN_ERR "Failed to remap register block\n");   
		devfs_remove(DEVICE_NAME);
		unregister_chrdev(major, DEVICE_NAME);
		return -ENOMEM;   
	}
	xdebug_p("ad init\n");
	return ret;
}

static void __exit ad_exit(void)
{
	devfs_remove(DEVICE_NAME);
	unregister_chrdev(major, DEVICE_NAME);
	xdebug_p("ad exit\n");
	return;
}

module_init(ad_init);
module_exit(ad_exit);


MODULE_AUTHOR("Helight.Xu");
MODULE_LICENSE("Dual BSD/GPL");
