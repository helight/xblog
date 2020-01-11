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
	char key_value[2] = {0};
	fd_set rds;

	fd = open ("/dev/xkeydev", O_RDWR);
	if(fd < 0){
		perror("open error:");
		return -1;
	}
	for (;;) {
/*		FD_ZERO(&rds);
		FD_SET(fd, &rds);

		ret = select(fd + 1, &rds, NULL, NULL, NULL);
		switch(ret) {
			case -1:
				perror("select");
				break;
			case 0:
				printf("Timeout.\n");
				break;
			default:
				if (FD_ISSET(fd, &rds)) {
				ret = read(fd, key_value, 1);
				printf("key_value: %02x \n", key_value);
				memset(key_value, '\0', sizeof(key_value));
			}
		}
*/
	ret = read(fd, key_value, 1);
	printf("key_value: %02x \n", key_value[0]);
	memset(key_value, '\0', sizeof(key_value));
	}
	close(fd);
	return 0;
}
