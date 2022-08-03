/*
 * Copyright (c) 2009-~ Helight.Xu
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License
 *
 * Author:       Helight.Xu<Helight.Xu@gmail.com>
 * Created Time: Fri 11 Sep 2009 06:04:53 PM CST
 * File Name:    spix.h
 *
 * Description:  
 */

#ifndef __H_SPIX
#define __H_SPIX

#include <asm/io.h>
#include <mach/hardware.h>
#include <mach/platform.h>
#include <mach/lpc32xx_gpio.h>

#define        SHIFT_ON     ~(1<<13)
#define        SHIFT_OFF    (1<<13)
#define        RXTX_R        ~(1<<15)
#define        RXTX_T        (1<<15)

#define SPIX_BASE 0x20088000
#define SPIX_IOBASE io_p2v(SPIX_BASE)
                                                                                                
#define spix_gobal      io_p2v(SPIX_BASE + 0x00)
#define spix_con        io_p2v(SPIX_BASE + 0x04)
#define spix_ier        io_p2v(SPIX_BASE + 0x0C)
#define spix_stat       io_p2v(SPIX_BASE + 0x10)
#define spix_data       io_p2v(SPIX_BASE + 0x14)

#define spix_tim        io_p2v(0x20088400)
#define spix_clk        io_p2v(0x400040C4)

#define p2mux           io_p2v(0x4002802C)
#define p2dir           io_p2v(0x40028010)
#define spimux          io_p2v(0x40028104)

#define p2out1          io_p2v(0x40028020)
#define p2out0          io_p2v(0x40028024)
#define p3out1          io_p2v(0x40028004)
#define p3out0          io_p2v(0x40028008)

#define disable_spi_flash()     __raw_writel((1 << 30), p3out1)
#define enable_spi_flash()      __raw_writel((1 << 30), p3out0)	//gpio_05
#define disable_write()         __raw_writel((1 << 7), p3out1)
#define enable_write()          __raw_writel((1 << 7), p3out0)	//gpo_7

int send_byte(unsigned char ch);

int send_byte(unsigned char ch)
{
	unsigned int tmp = 0, tim = 6;

	enable_write();
	//disable_write();
	enable_spi_flash();

	tmp = __raw_readl(spix_con);
	printk("spix: %02x \n", tmp);
	__raw_writel(((tmp & SHIFT_ON) | RXTX_T), spix_con);
	__raw_writel(ch, spix_data);
	do {
		tmp = __raw_readl(spix_stat);
		tim--;
	} while ((tmp & 0x08) && tim);
	
	printk("spix_stat: %02x ", tmp);
	tmp = __raw_readl(spix_data);
	printk("send_byte... tmp: %02x tim: %d\n", tmp, tim);

	tmp = __raw_readl(spix_con);
	__raw_writel((tmp | SHIFT_OFF), spix_con);

	disable_spi_flash();
	disable_write();
	return 0;
}

#endif
