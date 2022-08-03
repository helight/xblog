#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/semaphore.h>
#include <linux/sched.h>

static DEFINE_MUTEX(mut1);
struct semaphore sem1,sem2;

int test1(void *p)
{
        down(&sem2);
        mutex_lock(&mut1);
        printk("get out test1\n");
        mutex_unlock(&mut1);
        up(&sem1);

        return 0;
}

int test2(void *p)
{
        down(&sem1);
        mutex_lock(&mut1);
        printk("get out test2\n");
        mutex_unlock(&mut1);
        up(&sem2);

        return 0;
}

static int __init mutex_init_test()
{
        init_MUTEX(&sem1);
        init_MUTEX_LOCKED(&sem2);

        printk("get out \n");
        kernel_thread(test1, test1, CLONE_KERNEL);
        kernel_thread(test2, test2, CLONE_KERNEL);

        return 0;
}

static void __exit mutex_exit_test()
{
        printk("get out \n");
        return;
}

module_init(mutex_init_test);
module_exit(mutex_exit_test);

MODULE_LICENSE("Dual BDS/GPL");    
