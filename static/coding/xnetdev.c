#include <linux/module.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <asm/irq.h>
#include <asm/io.h>

static void *bwscon;
static void *gpfcon;
static void *extint0;
static void *intmsk;

#define BWSCON	(0x48000000)
#define GPFCON	(0x56000050)
#define EXTINT0	(0x56000088)
#define INTMSK	(0x4A000008)

#define DM9000_MIN_IO	0x20000300 //--
#define DM9000_MAX_IO	0x20000370 //--

#define DM9000_VID_L		0x28
#define DM9000_VID_H		0x29
#define DM9000_PID_L		0x2A
#define DM9000_PID_H		0x2B

#define DM9000_PKT_MAX          1536		//Received packet max size
#define DM9000_PKT_RDY          0x01		//Packet ready to receive

//#define XDEBUG

#ifdef  XDEBUG
#undef x_debug
#    define x_debug( x... )  printk( x )
#else
#    define x_debug( x... ) 
#endif


struct dm9000x{
	u32 ioaddr;		// Register I/O base address
	u32 iodata;		// Data I/O address
	u16 irq;		// IRQ
	u8 iomode;		// 0:16bits 1:word  2:byte
	u8 opmode;
	u16 Preg0, Preg4;

	u16 tx_pkt_cnt;
	u16 sent_pkt_len, queue_pkt_len;
	u8 device_wait_reset;	//device state
	u8 nic_type;		// NIC type

	spinlock_t lock;
};

static struct net_device *xnet_dev = NULL;

static void do_init_dm9000x(struct net_device *dev);
int xnet_probe(struct net_device *dev);
static int xnet_open(struct net_device *dev);
static int xnet_stop(struct net_device *dev);
static int xnet_xmit(struct sk_buff *skb, struct net_device *dev);
static irqreturn_t xnet_interrupt(int irq, void *dev_id, struct pt_regs *regs);

static void do_xnet_tx(void);
static void do_xnet_rx(void);

static void iowt(struct dm9000x *dm9x, int reg, u8 value);
static u8 iord(struct dm9000x *dm9x, int reg);
static u16 phy_read(struct dm9000x *dm9x, int reg);
static void phy_write(struct dm9000x *dm9x, int reg, u16 value);

int xnet_probe(struct net_device *dev)
{
	int i = 0;
	u32 id_val;
	u32 iobase;
	struct dm9000x *dm9x;
        unsigned char mac_add[6] = {0x00, 0x13, 0xf6, 0x6c, 0x87, 0x88};


	bwscon = ioremap_nocache(BWSCON,0x0000004);	//总线位宽和等待状态控制器
	gpfcon = ioremap_nocache(GPFCON,0x0000004);	//Port F 控制寄存器
	extint0 = ioremap_nocache(EXTINT0,0x0000004);	//外部中断控制
	intmsk = ioremap_nocache(INTMSK,0x0000004);	//中断控制

	writel(readl(bwscon) | 0xc0000, bwscon);		//允许等待,使用UB/LB
	//设置GFP7为外部中断模式 EINT7
	writel( (readl(gpfcon) & ~(0x3 << 14)) | (0x2 << 14), gpfcon);
	//设置EINT7 为 falling edge triggered 
	writel( (readl(extint0) & ~(0xf << 28)) | (0x4 << 28), extint0); 
	//中断掩码设置1为屏蔽,这里设置开启EINT4~7
	writel( (readl(intmsk))  & ~0x80, intmsk);

	iobase = ioremap(DM9000_MIN_IO, 0x400);		//进行地址隐射

	outb(DM9000_VID_L, iobase);
	id_val = inb(iobase + 4);
	outb(DM9000_VID_H, iobase);
	id_val |= inb(iobase + 4) << 8;
	outb(DM9000_PID_L, iobase);
	id_val |= inb(iobase + 4) << 16;
	outb(DM9000_PID_H, iobase);
	id_val |= inb(iobase + 4) << 24;

	if (id_val == 0x90000a46) {
		x_debug("id_val: %x, iobase: %p \n", id_val, (void *)iobase);
		
		dm9x = (void *)kmalloc(sizeof(struct dm9000x), GFP_KERNEL);
		memset(dm9x, 0, sizeof(struct dm9000x));
		dev->priv = dm9x;
		
		dm9x->ioaddr 		= iobase;
		dm9x->iodata 		= iobase + 4;
		ether_setup(dev);
		
		dev->base_addr	= iobase;
		dev->irq 		= 51;
		dev->open 		= &xnet_open;
		dev->stop 		= &xnet_stop;
		dev->hard_start_xmit = &xnet_xmit;
		SET_MODULE_OWNER(dev);
		for (i = 0; i < 6; i++)
			dev->dev_addr[i] = mac_add[i];
		request_region(iobase, 2, dev->name);	
	}	
	return 0;
}

static void do_init_dm9000x(struct net_device *dev)
{
	u16 i, oft, phy_reg3;
	u16 phy_reg0 = 0x1000;	//Auto-negotiation & non-duplux mode
	u16 phy_reg4 = 0x01e1;	//Default non flow control
	struct dm9000x *dm9x = (struct dm9000x *)dev->priv;

	//set the internal PHY power-on, GPIOs normal, and wait 2ms 
	iowt(dm9x, 0x1F, 0);		//GPR (reg_1Fh)bit GPIO0=0 pre-activate PHY 
	udelay(20);			//wait 2ms for PHY power-on ready 
	
	//0x00 network ctrl reg bit[0]:soft reset bit[1:2]:01 MAC inter loopback
	iowt(dm9x, 0x00, 3);	
	udelay(20);
	//set GPIO0=1 then GPIO0=0 to turn off and on the internal PHY 
	iowt(dm9x, 0x1F, 1);  	//GPR (reg_1Fh) bit[0] GPIO0=1 turn-off PHY  
	iowt(dm9x, 0x1F, 0);		//GPR (reg_1Fh) bit[0] GPIO0=0 activate PHY  
	udelay(1000);			//wait 4ms linking PHY (AUTO sense) if RX/TX 
	udelay(1000);
	udelay(1000);
	udelay(1000);
	
	dm9x->iomode = iord(dm9x, 0xFE) >> 6; //ISR bit[7:6] I/O mode
	
	//Full-Duplex Mode. Read only on Internal PHY mode. R/W on External PHY mode 
	iowt(dm9x, 0x00, 0x80);	
	phy_reg3 = phy_read(dm9x, 3);
	dm9x->nic_type = 0;
	x_debug("IOmode: %x phy_reg3: %x \n", dm9x->iomode, phy_reg3);
	iowt(dm9x, 0x00, 0x00);

	dm9x->opmode = 0;    
	phy_write(dm9x, 0, phy_reg0);
	phy_write(dm9x, 4, 0x0400 | phy_reg4);
	dm9x->Preg0 = phy_reg0;
	dm9x->Preg4 = phy_reg4 + 0x0400;

	/* Program operating register */
	iowt(dm9x, 0x00, 0x08);
	iowt(dm9x, 0x02, 0x00);		/* TX Polling clear */
	iowt(dm9x, 0x2f, 0x00);		/* Special Mode */
	iowt(dm9x, 0x01, 0x2c);	/* clear TX status */
	iowt(dm9x, 0xfe, 0x0f);	/* Clear interrupt status */
	iowt(dm9x, 0x08, 0x37);
	iowt(dm9x, 0x09, 0x38);	/* Flow control: High/Low water */
	iowt(dm9x, 0x0a, 0x29);	/* flow control */
	
	for (i = 0, oft = 0x10; i < 6; i++, oft++)
		iowt(dm9x, oft, dev->dev_addr[i]);	
	// Activate DM9000 
	iowt(dm9x, 0x05, 0x30 | 1);	//RX enable 
	iowt(dm9x, 0xff, 0x83);	//Enable TX/RX interrupt mask
	
	dm9x->tx_pkt_cnt	= 0;
	dm9x->queue_pkt_len	= 0;
	dev->trans_start	= 0;
	
	netif_carrier_on(dev);
	spin_lock_init(&dm9x->lock);
	x_debug("do init dm9000x xnte dev! \n");
}

static void do_xnet_tx(void)
{
	struct net_device *dev = xnet_dev;
	struct dm9000x *dm9x = (struct dm9000x *)dev->priv;
	int tx_status = iord(dm9x, 0x01);		//Got TX status
	
	if (tx_status & 0xc) {			//One packet sent complete
		dm9x->tx_pkt_cnt--;
		dev->trans_start = 0;
	
		if (dm9x->tx_pkt_cnt > 0) {		//Queue packet check & send
			//Set TX length to DM9000
			iowt(dm9x, 0xfc, dm9x->queue_pkt_len & 0xff);
			iowt(dm9x, 0xfd, (dm9x->queue_pkt_len >> 8) & 0xff);

			//Issue TX polling command
			iowt(dm9x, 0x2, 0x1);		//Cleared after TX complete
			dev->trans_start = jiffies;	//saved the time stamp
		}
		netif_wake_queue(dev);
	}
	x_debug("xnet_tx_done the xnet dev! \n");
}

static void do_xnet_rx(void)
{
	struct net_device *dev = xnet_dev;
	struct dm9000x *dm9x = (struct dm9000x *)dev->priv;
	struct sk_buff *skb;
	u8 rxbyte, *rdptr;
	u16 i, RxStatus, RxLen, GoodPacket, tmplen;
	
	do {
		iord(dm9x, 0xf0);		//Dummy read
		rxbyte = inb(dm9x->iodata);	//Got most updated data
		if (rxbyte == DM9000_PKT_RDY) { // packet ready to receive check
			GoodPacket = 1;
			outb(0xf2, dm9x->ioaddr);
			RxStatus = RxLen = (u16) 0;
			RxStatus = inw(dm9x->iodata);
			outb(0xf2, dm9x->ioaddr);
			RxLen = inw(dm9x->iodata);

			/* Packet Status check */
			if (RxLen < 0x40) {
				GoodPacket = 0;
			} else if (RxLen > DM9000_PKT_MAX) {
				x_debug("<DM9000> RST: RX Len:%x(%x)\n", RxLen, RxStatus);
				dm9x->device_wait_reset = 1;
			}
			if (RxStatus & 0xbf00) GoodPacket = 0;
			if (!dm9x->device_wait_reset){
				if(GoodPacket && ((skb = dev_alloc_skb(RxLen + 4)) != NULL)){
					skb->dev = dev;
					skb_reserve(skb, 2);
					rdptr = (u8 *) skb_put(skb, RxLen - 4);
					tmplen = (RxLen + 1) / 2;
					for (i = 0; i < tmplen; i++){
						((u16 *) rdptr)[i] = inw(dm9x->iodata);
						//x_debug("%x ",((u16 *) rdptr)[i]);
					}
					 /* Pass to upper layer */
					skb->protocol = eth_type_trans(skb, dev);
					netif_rx(skb);
				} else {	    /* Without buffer or error packet */
					tmplen = (RxLen + 1) / 2;
					for (i = 0; i < tmplen; i++)
						inw(dm9x->iodata);
				} 
			}			
		} else if (rxbyte > DM9000_PKT_RDY) {
			// Status check: this byte must be 0 or 1
			x_debug("RX SRAM 1st byte(%02x) != 01, must reset.\n", rxbyte);
			iowt(dm9x, 0x05, 0x00);		// Stop Device
			iowt(dm9x, 0xfe, 0x80);		// Stop INT request
			dm9x->device_wait_reset = 1;
			//dm9x->reset_rx_status++;
		}
	} while (rxbyte == DM9000_PKT_RDY && !dm9x->device_wait_reset); 
	x_debug("xnet_packet_receive the xnet dev! %x\n", dm9x->ioaddr);
}

static irqreturn_t xnet_interrupt(int irq, void *dev_id, struct pt_regs *regs)
{
	struct net_device *dev = dev_id;	
	struct dm9000x *dm9x;
	int int_status;
	u8 reg_save;

	dm9x = (struct dm9000x *)dev->priv;
	spin_lock_irq(&dm9x->lock);

	//Save previous register address 
	reg_save = inb(dm9x->ioaddr);
	//Disable all interrupt
	iowt(dm9x, 0xff, 0x80);

	//Got DM9000 interrupt status 
	int_status = iord(dm9x, 0xfe);		//Got ISR 
	iowt(dm9x, 0xfe, int_status);		//Clear ISR status

	if (int_status & 0x02) {	//Trnasmit Interrupt check
		do_xnet_tx();
	}
	if (int_status & 0x01) {	//Received the coming packet
		do_xnet_rx();
	}
	//Re-enable interrupt mask
	iowt(dm9x, 0xff, 0x83);

	//Restore previous register address
	outb(reg_save, dm9x->ioaddr);

	spin_unlock_irq(&dm9x->lock);

	x_debug("interrupt the xnet dev!  %x\n", dm9x->ioaddr);
	return IRQ_HANDLED;
}

static int xnet_open(struct net_device *dev)
{
	//struct dm9000x *dm9x = (struct dm9000x *)dev->priv;
	
	if(request_irq(dev->irq, &xnet_interrupt, SA_SHIRQ, dev->name, dev))
		return -EAGAIN;
		
	do_init_dm9000x(dev);

	netif_start_queue(dev);
	enable_irq(dev->irq);

	x_debug("open the xnet dev! \n");
	return 0;
}

static int xnet_stop(struct net_device *dev)
{
	struct dm9000x *dm9x = (struct dm9000x *)dev->priv;

	netif_stop_queue(dev);
	free_irq(dev->irq, dev);

	/* RESET devie */
	phy_write(dm9x, 0x00, 0x8000);	/* PHY RESET */
	iowt(dm9x, 0x1f, 0x01);	/* Power-Down PHY */
	iowt(dm9x, 0xff, 0x80);	/* Disable all interrupt */
	iowt(dm9x, 0x05, 0x00);	/* Disable RX */

	x_debug("stop the xnet dev! \n");
	return 0;
}

static int xnet_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct dm9000x *dm9x = (struct dm9000x *)dev->priv;
	char *data;
	int i, len;

	if (dm9x->tx_pkt_cnt > 1) return 1;

	netif_stop_queue(dev);
	iowt(dm9x, 0xff, 0x80);	//Disable all interrupt
	data = (char *)skb->data;
	outb(0xf8, dm9x->ioaddr);

	len = (skb->len + 1) / 2;
	for (i = 0; i < len; i++){
		outw(((u16 *) data)[i], dm9x->iodata);
		//x_debug("%x ",((u16 *) data)[i]);
	}

	// TX control: First packet immediately send, second packet queue 
	if (dm9x->tx_pkt_cnt == 0){			// First Packet
		dm9x->tx_pkt_cnt++;
		// Set TX length to DM9000
		iowt(dm9x, 0xfc, skb->len & 0xff);
		iowt(dm9x, 0xfd, (skb->len >> 8) & 0xff);
		// Issue TX polling command
		iowt(dm9x, 0x2, 0x1);			//Cleared after TX complete
		// saved the time stamp
		dev->trans_start = jiffies;
	} else {					//Second packet
		dm9x->tx_pkt_cnt++;
		dm9x->queue_pkt_len = skb->len;
	}

	dev_kfree_skb(skb);

	if (dm9x->tx_pkt_cnt == 1) netif_wake_queue(dev);
	//Re-enable interrupt
	iowt(dm9x, 0xff, 0x83);

	x_debug("hard start xmit the xnet dev! \n");	
	return 0;
}

static void iowt(struct dm9000x *dm9x, int reg, u8 value)
{
	outb(reg, dm9x->ioaddr);
	outb(value, dm9x->iodata);
}

static u8 iord(struct dm9000x *dm9x, int reg)
{
	outb(reg, dm9x->ioaddr);
	return inb(dm9x->iodata);
}

//Read a word from phyxcer
static u16 phy_read(struct dm9000x *dm9x, int reg)
{
	//Fill the phyxcer register into REG_0C
	//0x0C EEPROM & PHY Address Register bit[7:6]:01 select PHY
	//bit[5:0]:address of PHY or EEPROM
	iowt(dm9x, 0xc, 0x40 | reg);
	iowt(dm9x, 0xb, 0xc);	//Issue phyxcer read command
	udelay(100);			//Wait read complete
	iowt(dm9x, 0xb, 0x0);	//Clear phyxcer read command

	//The read data keeps on REG_0D(L) & REG_0E(H)
	return (iord(dm9x, 0xe) << 8) | iord(dm9x, 0xd);
}

static void phy_write(struct dm9000x *dm9x, int reg, u16 value)
{
	//Fill the phyxcer register into REG_0C
	iowt(dm9x, 0xc, 0x40 | reg);

	//Fill the written data into REG_0D(L) & REG_0E(H)
	iowt(dm9x, 0xd, (value & 0xff));
	iowt(dm9x, 0xe, ((value >> 8) & 0xff));

	iowt(dm9x, 0xb, 0xa);	//Issue phyxcer write command 
	udelay(500);			//Wait write complete 
	iowt(dm9x, 0xb, 0x0);	//Clear phyxcer write command
}

static int __init xnet_dev_init(void)
{
	int err = 0;

	xnet_dev = alloc_etherdev(sizeof(struct net_device));
	xnet_dev->init = xnet_probe;
	
	err = dev_alloc_name(xnet_dev, "eth%d");
	if (err < 0)
		return err;
	err = register_netdev(xnet_dev);
	if (err < 0)
		return err;
	x_debug("init the xnet dev! \n");

	return 0;
}

static void __exit xnet_dev_exit(void)
{
	unregister_netdev(xnet_dev);
	kfree(xnet_dev->priv);
	memset(xnet_dev, 0, sizeof (*xnet_dev));
	x_debug("xnet dev exit!! \n");
}

module_init(xnet_dev_init);
module_exit(xnet_dev_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Helight.Xu");
