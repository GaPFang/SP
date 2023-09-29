#include "hw1.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)
#define BUFFER_SIZE 10 * (FROM_LEN + CONTENT_LEN + 5)

typedef struct {
    char* ip; // server's ip
    unsigned short port; // server's port
    int conn_fd; // fd to talk with server
    char buf[BUFFER_SIZE]; // data sent by/to server
    size_t buf_len; // bytes used by buf
} client;

client cli;
static void init_client(char** argv);

void pull();

int main(int argc, char** argv){
    
    // Parse args.
    if(argc!=3){
        ERR_EXIT("usage: [ip] [port]");
    }

    // Handling connection
    init_client(argv);
    fprintf(stderr, "connect to %s %d\n", cli.ip, cli.port);

    printf("==============================\nWelcome to CSIE Bulletin board\n==============================\n");
    // recv(cli.conn_fd, cli.buf, BUFFER_SIZE, 0);
    strcpy(cli.buf, "");
    printf("%s\n", cli.buf);
    printf("==============================\n");

    while(1){
        // TODO: handle user's input
        printf("Please enter your command (post/pull/exit): ");
        scanf("%s", cli.buf);
        if (strcmp(cli.buf, "post") == 0) {
            send(cli.conn_fd, cli.buf, strlen(cli.buf), 0);
            printf("FROM: ");
            scanf("%s", cli.buf);
            strcat(cli.buf, "\n");
            send(cli.conn_fd, cli.buf, strlen(cli.buf), 0);
            // recv(cli.conn_fd, cli.buf, BUFFER_SIZE, 0);
            // printf("%s", cli.buf);
            strcpy(cli.buf, "");
            printf("CONTENT:\n");
            scanf("%s", cli.buf);
            strcat(cli.buf, "\n\n");
            send(cli.conn_fd, cli.buf, strlen(cli.buf), 0);
            // recv(cli.conn_fd, cli.buf, BUFFER_SIZE, 0);
            // printf("%s", cli.buf);
            strcpy(cli.buf, "");
        } else if (strcmp(cli.buf, "pull") == 0) {
            // send(cli.conn_fd, cli.buf, strlen(cli.buf), 0);
            // recv(cli.conn_fd, cli.buf, BUFFER_SIZE, 0);
            printf("%s", cli.buf);
            strcpy(cli.buf, "");
        } else if (strcmp(cli.buf, "exit") == 0) {
            send(cli.conn_fd, cli.buf, strlen(cli.buf), 0);
            break;
        }
        printf("==============================\n");
    }
    close(cli.conn_fd);
    return 0;
}

static void init_client(char** argv){
    
    cli.ip = argv[1];

    if(atoi(argv[2])==0 || atoi(argv[2])>65536){
        ERR_EXIT("Invalid port");
    }
    cli.port=(unsigned short)atoi(argv[2]);

    struct sockaddr_in servaddr;
    cli.conn_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(cli.conn_fd<0){
        ERR_EXIT("socket");
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(cli.port);

    if(inet_pton(AF_INET, cli.ip, &servaddr.sin_addr)<=0){
        ERR_EXIT("Invalid IP");
    }

    if(connect(cli.conn_fd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
        ERR_EXIT("connect");
    }

    return;
}

void pull() {
    
}
