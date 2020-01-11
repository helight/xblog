#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/sched.h>

static int mycount = 0;
static long mytime = 0;

static int irq;
static char *interface;

module_param(interface,charp,0644);
module_param(irq,int,0644);
         static struct tasklet_struct mytasklet;
       static void mylet(unsigned long data)
{               printk("mylet begin\n");

   if(mycount==0)
   mytime=jiffies;
      if (mycount < 10)
   {
       mytime=jiffies-mytime;
       printk("Interrupt number %d --time %ld \n",irq,mytime);
       mytime=jiffies;
   }
   mycount++;

   return;
}

static irqreturn_t myinterrupt(int intno,void *dev_id)
{
   tasklet_schedule(&mytasklet);
   return IRQ_NONE;
}

static int __init myirqtest_init(void)
{
   static unsigned data=0;
   printk ("My module worked!\n");

   tasklet_init(&mytasklet, mylet, data);


   //if (request_irq(irq,&myinterrupt,SA_SHIRQ,interface,&irq))   //for 2.6.22
   if (request_irq(irq,&myinterrupt,IRQF_SHARED,interface,&irq)) //for 2.2.24
   {
       printk(KERN_ERR "myirqtest: cannot register IRQ %d\n", irq);
       free_irq(irq,&irq);
       return -EIO;
   }
   printk("%s Request on IRQ %d succeeded\n",interface,irq);

   return 0;
}

static void __exit myirqtest_exit(void)
{
   printk ("Unloading my module.\n");

   tasklet_kill(&mytasklet);
   free_irq(irq,&irq);
   printk("Freeing IRQ %d\n", irq);

   return;
}

module_init(myirqtest_init);
module_exit(myirqtest_exit);

MODULE_LICENSE("GPL"); 
