#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/select.h>


int main()
{
	int fd = 0, ret = 0;
	int key_value[2] = {0};
	fd_set rds;

	fd = open ("/dev/xaddev", O_RDWR);
	if(fd < 0){
		perror("open error:");
		return -1;
	}
	for (;;) {
		ret = read(fd, key_value, sizeof(int));
		printf("key_value: %d \n", key_value[0]);
		memset(key_value, '\0', sizeof(key_value));
		sleep(1);
	}
	close(fd);
	return 0;
}
