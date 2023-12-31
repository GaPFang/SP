#include "hw1.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)
#define RECORD_LEN (FROM_LEN + CONTENT_LEN)
// #define BUFFER_SIZE (RECORD_NUM * RECORD_LEN)

typedef struct {
    char* ip; // server's ip
    unsigned short port; // server's port
    int conn_fd; // fd to talk with server
    char buf[RECORD_LEN]; // data sent by/to server
    size_t buf_len; // bytes used by buf
} client;

client cli;

struct pollfd fdArray[1];

static void init_client(int argc, char** argv);

void pull();

void post();

int main(int argc, char** argv){
    
    // Parse args.
    // if(argc!=3){
    //     ERR_EXIT("usage: [ip] [port]");
    // }

    // Handling connection
    init_client(argc, argv);
    fprintf(stderr, "connect to %s %d\n", cli.ip, cli.port);
    memset(cli.buf, 0, RECORD_LEN);

    fdArray[0].fd = cli.conn_fd;
    fdArray[0].events = POLLIN;

    printf("==============================\nWelcome to CSIE Bulletin board\n==============================\n");
    pull();

    while(1){
        // TODO: handle user's input
        printf("Please enter your command (post/pull/exit): ");
        memset(cli.buf, 0, RECORD_LEN);
        scanf("%s", cli.buf);
        if (strcmp(cli.buf, "post") == 0) {
            post();
        } else if (strcmp(cli.buf, "pull") == 0) {
            printf("==============================\n");
            pull();
        } else if (strcmp(cli.buf, "exit") == 0) {
            fdArray[0].events = POLLOUT;
            poll(fdArray, 1, -1);
            send(cli.conn_fd, cli.buf, strlen(cli.buf), 0);
            break;
        }
    }
    close(cli.conn_fd);
    return 0;
}

void post() {
    send(cli.conn_fd, cli.buf, strlen(cli.buf), 0);
    memset(cli.buf, 0, RECORD_LEN);
    poll(fdArray, 1, -1);
    recv(cli.conn_fd, cli.buf, RECORD_LEN, 0);
    if (strcmp(cli.buf, "X") == 0) {
        printf("[Error] Maximum posting limit exceeded\n");
    } else {
        memset(cli.buf, 0, RECORD_LEN);
        printf("FROM: ");
        scanf("%s", cli.buf);
        strcat(cli.buf, "\0");
        send(cli.conn_fd, cli.buf, strlen(cli.buf), 0);
        memset(cli.buf, 0, RECORD_LEN);
        printf("CONTENT:\n");
        scanf("%s", cli.buf);
        strcat(cli.buf, "\0");
        send(cli.conn_fd, cli.buf, strlen(cli.buf), 0);
    }
}

void pull() {
    send(cli.conn_fd, cli.buf, strlen(cli.buf), 0);
    memset(cli.buf, 0, RECORD_LEN);
    fdArray[0].events = POLLIN;
    poll(fdArray, 1, -1);
    recv(cli.conn_fd, cli.buf, FROM_LEN, 0);
    while (strcmp(cli.buf, "end") != 0) {
        printf("FROM: %s\n", cli.buf);
        memset(cli.buf, 0, RECORD_LEN);
        poll(fdArray, 1, -1);
        recv(cli.conn_fd, cli.buf, CONTENT_LEN, 0);
        printf("CONTENT:\n%s\n", cli.buf);
        memset(cli.buf, 0, RECORD_LEN);
        poll(fdArray, 1, -1);
        recv(cli.conn_fd, cli.buf, FROM_LEN, 0);
    }
    printf("==============================\n");
}

static void init_client(int argc, char** argv){
    
    if (argc!=3) {
        cli.ip = "127.0.0.1";
        cli.port = 8888;
    } else {
        cli.ip = argv[1];

        if(atoi(argv[2])==0 || atoi(argv[2])>65536){
            ERR_EXIT("Invalid port");
        }
        cli.port=(unsigned short)atoi(argv[2]);
    }
    

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
