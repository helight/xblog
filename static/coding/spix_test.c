/*
 * Copyright (c) 2009-~ Helight.Xu
 *
 * This source code is released for free distribution under the terms of the
 * GNU General Public License
 *
 * Author:       Helight.Xu<Helight.Xu@gmail.com>
 * Created Time: Fri 11 Sep 2009 06:04:53 PM CST
 * File Name:    spix_test.h
 *
 * Description:  
 */
 
#include <stdio.h>
#include <stdlib.h>	//system
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEV_NAME "/dev/spix"
#define DELAY 100000
//			L     P     C     3     2     5     0
char disp_data[7] = { 0xC7, 0x8C, 0xC6, 0xB0, 0xA4, 0x92, 0xC0};

int main(int argc, char *argv[])
{
	int fd;
	int i = 0, j = 0;

	fd = open(DEV_NAME, O_RDWR);
	if(fd < 0) {
		perror("can not open device");
		exit(1);
	}

	printf("Test write method......\n");
	for (j = 0; j < 100; j++)
		for (i = 0; i < 7; i++) {
			usleep(1000000);
			write(fd, (char *)&disp_data[i], 1);
		}
	//write(fd, disp_data, 7);
	printf("\nTest write method OK!\n");

	close(fd);
	return 0;
}
