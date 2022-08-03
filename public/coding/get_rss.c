/*
 * rss_read.c
 *
 *  Created on: Mar 21, 2009
 *      Author: helight
目前在我自己写的 Xnotebook文本资料管理软件  中使用。
*/

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/ioctl.h>
#include <dirent.h>

#define rss_file_q  "GET %s HTTP/1.1\nHost: %s:%s\n%s%s"
#define rss_accept "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\n"
#define rss_connet "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\nKeep-Alive: 300\nConnection: keep-alive\n\n"

#define sport 80
char rss_path[] = "/home/helight/.xnote/.rss";

void get_rss(char *rss_url, char *rss_file);


static inline void debug_p(const char * format, ...)
{                                                                               
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
}


int main()
{
    int fd;
    char rss_file[1024];
    char rss_url[1024];
    char *ch=NULL;
    DIR *dir;
    struct dirent *ptr;
    
    debug_p("root_path:%s\n",rss_path);
opendir:if((dir = opendir(rss_path)) == NULL){
        perror("cann't open it\n");
        switch(errno){
        case EACCES:
            perror("Permission denied.\n");
            exit(1);
        default:
            printf("Directory does not exist, or name is an empty string.\n");
            if(mkdir(rss_path, 0755) < 0){
                perror("mkdir:\n");
                exit(1);
                }
            goto opendir;
        }
    }
printf("read ok \n");
    while((ptr = readdir(dir)) != NULL){
        ch = ptr->d_name;
        if(*ch != '.'){
            memset(rss_file, '\0', sizeof(rss_file));
            snprintf(rss_file, sizeof(rss_file), "%s/%s", rss_path, ptr->d_name);
            printf("read ok: %s \n", rss_file);
            if ((fd = fork()) < 0) {
                perror("fork:");
                break;
            } else if (fd > 0) {
                continue;
            } else {
                FILE *ffd = NULL;
                int len = 0;
                ffd = fopen(rss_file, "r");
                memset(rss_url, '\0', sizeof(rss_url));
                printf("read ok: %s \n", rss_url);
                fgets(rss_url, sizeof(rss_url), ffd);
                len = strlen(rss_url);
                *(rss_url + len -1) = '\0';
                printf("read ok2: %s \n", rss_url);
                fclose(ffd);
                get_rss(rss_url, rss_file);
               
            }//fork
        }
        //debug_p("file name:%s fname[]:%s\n",ch, fname[0][0]);
    } //while   
    printf("ok");
    closedir(dir);
    return 0;
}

void get_rss(char *rss_url, char *rss_file)
{
    char servername[256];
    char buff[1024*100];
    char *ch = NULL;
    char *ch2 = NULL;
    char *title = NULL;
    char *link = NULL;   
    int ret = 0, rech = 0;
    int rss_socket;
    struct sockaddr_in cliet_add;
    struct hostent *site;   
    struct timeval tv;
    fd_set readfds;
    FILE *fp;
   
    ch = strstr(rss_url, "http://");
    rss_url = ch;
    ch2 = ch + 7;
    ch = strstr(ch2, "/");
    snprintf(servername, ch - ch2 + 1, "%s", ch2);
    printf("servername: %s \n", servername);
    site = gethostbyname (servername);
    printf("Host name : %s\n", site->h_name);
    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)site->h_addr)));
    printf("servername: %s \n", servername);   
    if ((rss_socket = socket(PF_INET, SOCK_STREAM, 0)) > 0)
        printf("The socket has been created\n");
    else {
        perror("Could not create socket");
        exit(1);
    }
     
    memset(&cliet_add, 0, sizeof(struct sockaddr_in));
    cliet_add.sin_family = AF_INET;

    memcpy (&cliet_add.sin_addr, site->h_addr_list[0], site->h_length);
    cliet_add.sin_port = htons(sport);

    if (connect(rss_socket, (struct sockaddr *)&cliet_add, sizeof(cliet_add)) == 0)
        printf("Connected to host %s\n", inet_ntoa(cliet_add.sin_addr));
    else {
        perror("Could not connect");
        exit(2);
    }
    printf("buff: %s \n", "buff");
    snprintf(buff, sizeof(buff), rss_file_q, rss_url, inet_ntoa(*((struct in_addr *)site->h_addr)), "80", rss_accept ,rss_connet);
    printf("buff: %s \n", buff);
   
    if (write(rss_socket, buff, strlen(buff)) == -1)
        printf(" send error\n");
   
    FD_ZERO(&readfds);
    FD_SET(rss_socket, &readfds);
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    select(rss_socket + 1, &readfds, NULL, NULL, &tv);
   
    if (FD_ISSET(rss_socket, &readfds)) {
       
        while ((ret = read(rss_socket, buff + rech, sizeof(buff) - rech)) > 0) {
            rech +=ret;
        }
    //printf("----------------------------------------------------------------\n %s\n", buff);
    }
    if (close(rss_socket) != 0)
        perror("Warning: problem closing our_socket");

    ch = strstr(buff, "<channel>");
    if (ch){
        ch = strstr(ch, "<title>");
        ch2 = strstr(ch, "</title>");
        *ch2 = '\0';
        printf("title: %s \n", ch+7);
    }
    fp = fopen(rss_file, "wb+");
    printf("rss_file : %s rss_url: %s\n", rss_file, rss_url);
    fprintf(fp, "%s\n", rss_url);
    do {
        ch = strstr(ch2 + 1, "<item>");
        if (ch) {
            ch2 = strstr(ch, "</item>");
            if (ch2) {
                ch = strstr(ch, "<title>");
                ch2 = strstr(ch, "</title>");
                *ch2 = '\0';
                title = ch + 7;
                ch = strstr(ch2 + 1, "<link>");
                ch2 = strstr(ch, "</link>");
                *ch2 = '\0';
                link = ch + 6;
                fprintf(fp, "%s %s\n", title, link);
            } else
                continue;
        } else
            continue;
    }while(ch);
    fclose(fp);
    printf("over\n");
    return;
}

