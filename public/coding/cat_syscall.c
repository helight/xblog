#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/unistd.h> 
#include <linux/init.h>
#include <asm/ptrace.h>

#define NRB 2

typedef asmlinkage int (*__routine)(struct pt_regs);

__routine old, new;
unsigned long *sys_call_table = 0;
static int counts = 0;

unsigned long* find_sys_call_table(void)
{
        struct {
                unsigned short  limit;
                unsigned int    base;
        } __attribute__ ( ( packed ) ) idtr;

        struct {
                unsigned short  offset_low;
                unsigned short  segment_select;
                unsigned char   reserved,   flags;
                unsigned short  offset_high;
        } __attribute__ ( ( packed ) ) * idt;

        unsigned long system_call = 0;        // x80中断处理程序system_call 地址
        char *call_hex = "\xff\x14\x85";        // call 指令
        char *code_ptr = NULL;
        char *p = NULL;
        unsigned long sct = 0x0;
        int i = 0;

        __asm__ ( "sidt %0": "=m" ( idtr ) );
        idt = ( void * ) ( idtr.base + 8 * 0x80 );
        system_call = ( idt->offset_high << 16 ) | idt->offset_low;

        code_ptr = (char *)system_call;
        for(i = 0;i < ( 100 - 2 ); i++) {
            if(code_ptr[i] == call_hex[0]
                && code_ptr[i+1] == call_hex[1]
                && code_ptr[i+2] == call_hex[2] ) {
                p = &code_ptr[i] + 3;
                break;
            }
        }
        if ( p ){
                sct = *(unsigned long*)p;
        }
        return (unsigned long*)sct;
}

asmlinkage int audit_sys_call(struct pt_regs regs)
{
        int ret = 0;
        counts++;
        printk("audit_sys_call!\n");
        printk("call %ld sys_call! times: %d\n", regs.ax, counts);
        ret = ((__routine)old)(regs);
        return ret;
}

int mysys_init ( void ) {
        if (( sys_call_table = find_sys_call_table())) {
                printk( "sys_call_table = %p\n", sys_call_table );
                ld = (__routine)sys_call_table[NRB];
                new = &audit_sys_call;
                sys_call_table[NRB] = (unsigned long)new;
        }
        return 0;
}

void mysys_exit ( void ) {

        printk("keep back the sys_call_table \n");
        sys_call_table[NRB] = (unsigned long)old;
}

module_init(mysys_init);
module_exit(mysys_exit);

MODULE_LICENSE("GPL2.0");
MODULE_AUTHOR("Helight.Xu");
