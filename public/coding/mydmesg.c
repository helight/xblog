#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/klog.h>

int main()
{
    unsigned char buf[1024*4];
    int relog=0;

    relog=klogctl(3,buf,4096);
    if(relog<0)
    {
      perror("klogctl");
      exit(1);
    }
      printf("return:[%d] \n Receive len: [ %d ] \n Ring buf: %s\
					\n",relog,strlen(buf),buf);
    return 0;
} 
