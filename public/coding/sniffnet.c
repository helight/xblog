/*This program is wrote by helight--Zhenwen Xu
 *version 0.1
 *2008-04-13
*/
#define DRIVER_AUTHOR "Net4-Helight"
#define DRIVER_DESC   "A sample test"
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ip.h>
#include <linux/tcp.h>

MODULE_LICENSE("GPL");

/*regist the hooks*/
static struct nf_hook_ops nfho;

unsigned int hook_func(unsigned int hookunm,struct sk_buff **skb,const struct net_device *in,const struct net_device *out,int (*okfn)(struct sk_buff *))
{
int i=0;
struct tcphdr *tcph;
struct iphdr *iph;
struct sk_buff *pskb=*skb;
printk("\n the pskb->protocol :%d",pskb->protocol);

printk("MAC--mark:%d\n",(unsigned int)pskb->pkt_type);
iph=(struct iphdr*)pskb->network_header;

printk("IPHDR:%d\n",(unsigned int)iph->protocol);
printk("IP: [%u.%u.%u.%u]-->[%u.%u.%u.%u]",NIPQUAD(iph->saddr),NIPQUAD(iph->daddr));

if(iph->protocol==6)
{
    tcph=(struct tcphdr*)pskb->transport_header;
    printk("TCP: [%u]-->[%u]",ntohs(tcph->source),ntohs(tcph->dest));
    printk("\n");
}
return NF_ACCEPT;

}

/*init the module*/
static int init_sniffer(void)
{
nfho.hook=hook_func;
nfho.hooknum=NF_IP_PRE_ROUTING;
nfho.pf=PF_INET;
nfho.priority=NF_IP_PRI_FIRST;

nf_register_hook(&nfho);

printk("This is helight's sniffer\n");
return 0;
}

/*Clear the module*/

void exit_sniffer(void)
{
printk("This is helight's sniffer\n");
nf_unregister_hook(&nfho);
}

module_init(init_sniffer);
module_exit(exit_sniffer);
