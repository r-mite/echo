#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>

#define PORT "5000"
#define BUF_LEN 256

#define BACKLOG 5
//#define PORT "12345"

// 戻り値 <0   エラー
//        >=0  listenしているソケット
int tcp_listen(const char* service) { // サービス名 or ポート番号（の文字列）
//int tcp_listen(int service){
int err;
struct addrinfo hints;
struct addrinfo* res = NULL;
struct addrinfo* ai;
int sockfd;

memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_INET6;    // AF_INET6は、IPv4/IPv6両方を受け付ける。
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE;   // bind()する場合は指定する。

// node = NULLのとき、INADDR_ANY, IN6ADDR_ANY_INIT に相当。
err = getaddrinfo(NULL, service, &hints, &res);
if (err != 0) {
printf("getaddrinfo(): %s\n", gai_strerror(err));
return -1;
}

ai = res;
//sock_print("create socket", ai->ai_family, ai->ai_socktype);
sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
if (sockfd < 0)
return -1;
int on = 1;
if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
return -1;
else
printf("set SO_REUSEADDR\n");

if (bind(sockfd, ai->ai_addr, ai->ai_addrlen) < 0)
return -1;

if (listen(sockfd, BACKLOG) < 0)
return -1;
//else
//sockaddr_print("listen succeeded", ai->ai_addr, ai->ai_addrlen);

freeaddrinfo(res);
return sockfd;
}


int read_line(int socket, char *p){
int len = 0;
while(1){
int ret;
ret = read(socket, p, 1);
if(ret == -1){
perror("read");
exit(1);
}else if(ret == 0){
break;
}
if(*p == '\n'){
p++;
len++;
break;
}
p++;
len++;
}
*p = '\0';
return len;
}


void test_server() {
int sockfd;

sockfd = tcp_listen(PORT);
if (sockfd < 0) {
perror("server");
exit(1);
}

printf("wait...\n");

while (1) {
int cs;
struct sockaddr_storage sa;  // sockaddr_in 型ではない。
socklen_t len = sizeof(sa);  // クライアントの情報を得る場合
cs = accept(sockfd, (struct sockaddr*) &sa, &len);
if (cs < 0) {
//if (errno == EINTR)
//continue;
perror("accept");
exit(1);
}

printf("accept.\n");
//sockaddr_print("peer", (struct sockaddr*) &sa, len);
/*if (fork() == 0) {
// 子プロセス
char ch;
close(sockfd);

read(cs, &ch, 1);
ch++;
write(cs, &ch, 1);

close(cs);
exit(0);
}*/

while(1){
int read_size;
char buf[BUF_LEN];
read_size = read_line(cs, buf);
if(read_size == 0)break;
printf("message: %s", buf);
write(cs, buf, strlen(buf));
}
printf("reject.\n");

close(cs);
}
}

int main() {
test_server();
return 0;
}

