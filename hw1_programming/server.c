#include "hw1.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <stdbool.h>

#define ERR_EXIT(a) do { perror(a); exit(1); } while(0)
#define RECORD_LEN FROM_LEN + CONTENT_LEN
#define BUFFER_SIZE 10 * RECORD_LEN

typedef struct {
    char hostname[512];  // server's hostname
    unsigned short port;  // port to listen
    int listen_fd;  // fd to wait for a new connection
} server;

typedef struct {
    char host[512];  // client's host
    int conn_fd;  // fd to talk with client
    char buf[BUFFER_SIZE];  // data sent by/to client
    size_t buf_len;  // bytes used by buf
    int id;
    int status;
} request;

enum status {
    WAITING,
    POST_FROM,
    POST_CONTENT,
    PULL,
};

server svr;  // server
request* requestP = NULL;  // point to a list of requests
int maxfd;  // size of open file descriptor table, size of request list
int last = 0;

// initailize a server, exit for error
static void init_server(unsigned short port);

// initailize a request instance
static void init_request(request* reqP);

// free resources used by a request instance
static void free_request(request* reqP);

int check_listen_fd();

void handleWaiting(int curFd, int BulletinFd);

void handlePostFrom(int curFd, int BulletinFd);

void handlePostContent(int curFd, int BulletinFd);

void handlePull(int curFd, int BulletinFd);

int main(int argc, char** argv) {

    // Parse args.
    // if (argc != 2) {
    //     ERR_EXIT("usage: [port]");
    //     exit(1);
    // }

    struct sockaddr_in cliaddr;  // used by accept()
    int clilen;

    int conn_fd;  // fd for a new connection with client
    int file_fd;  // fd for file that we open for reading
    char buf[BUFFER_SIZE];
    int buf_len;

    // Initialize server
    // init_server((unsigned short) atoi(argv[1]));
    init_server(8888);

    int BulletinFd = open("./BulletinBoard.txt", O_RDWR | O_CREAT, 0666);
    int largestFd  = BulletinFd + 1;

    struct pollfd *readFdArray = (struct pollfd *)malloc(sizeof(struct pollfd) * maxfd);

    // Loop for handling connections
    fprintf(stderr, "\nstarting on %.80s, port %d, fd %d, maxconn %d...\n", svr.hostname, svr.port, svr.listen_fd, maxfd);

    while (1) {
        // TODO: Add IO multiplexing
        if (check_listen_fd()) {
            // Check new connection
            clilen = sizeof(cliaddr);
            conn_fd = accept(svr.listen_fd, (struct sockaddr*)&cliaddr, (socklen_t*)&clilen);
            if (conn_fd < 0) {
                if (errno == EINTR || errno == EAGAIN) continue;  // try again
                if (errno == ENFILE) {
                    (void) fprintf(stderr, "out of file descriptor table ... (maxconn %d)\n", maxfd);
                    continue;
                }
                ERR_EXIT("accept");
            }
            requestP[conn_fd].conn_fd = conn_fd;
            requestP[conn_fd].status = WAITING;
            readFdArray[conn_fd - BulletinFd - 1].fd = conn_fd;
            readFdArray[conn_fd - BulletinFd - 1].events = POLLIN;
            if (conn_fd > largestFd) largestFd = conn_fd;
            strcpy(requestP[conn_fd].host, inet_ntoa(cliaddr.sin_addr));
            fprintf(stderr, "getting a new request... fd %d from %s\n", conn_fd, requestP[conn_fd].host);
        }

        // TODO: handle requests from clients
        int totalFds = poll(readFdArray, largestFd  - BulletinFd, 5);
        // if (totalFds) fprintf(stderr, "%d\n", totalFds);
        if (totalFds) {
            int curFd = BulletinFd + 1;
            while (totalFds) {
                // fprintf(stderr, "%d\n", totalFds);
                if (readFdArray[curFd - BulletinFd - 1].revents) {
                    recv(requestP[curFd].conn_fd, requestP[curFd].buf, BUFFER_SIZE, 0);
                    switch (requestP[curFd].status) {
                        case WAITING:
                            handleWaiting(curFd, BulletinFd);
                            break;
                        case POST_FROM:
                            handlePostFrom(curFd, BulletinFd);
                            break;
                        case POST_CONTENT:
                            handlePostContent(curFd, BulletinFd);
                            break;
                        case PULL: //check writeFdArray
                            handlePull(curFd, BulletinFd);
                            break;
                    }
                    totalFds--;
                }
                curFd++;
            }
        }
    }
    close(requestP[conn_fd].conn_fd);
    free_request(&requestP[conn_fd]);
    free(requestP);
    return 0;
}

int check_listen_fd() {
    struct pollfd fdArray[1];
    fdArray[0].fd = svr.listen_fd;
    fdArray[0].events = POLLIN;
    return poll(fdArray, 1, 5);
}

void handleWaiting(int curFd, int BulletinFd) {
    if (strcmp(requestP[curFd].buf, "post") == 0) {
        requestP[curFd].status = POST_FROM;
        last++;
        memset(requestP[curFd].buf, 0, sizeof(requestP[curFd].buf));
    } else if (strcmp(requestP[curFd].buf, "pull") == 0) {
        lseek(BulletinFd, 0, SEEK_SET);
        // read(BulletinFd, requestP[curFd].buf, 20);
        read(BulletinFd, requestP[curFd].buf, FROM_LEN);
        printf("%s", requestP[curFd].buf);
        char str[10] = "abc";
        strcat(str, requestP[curFd].buf);
        fprintf(stderr, "%s", requestP[curFd].buf);
        send(curFd, requestP[curFd].buf, BUFFER_SIZE, 0);
        memset(requestP[curFd].buf, 0, sizeof(requestP[curFd].buf));
        // requestP[curFd].status = PULL;
        // memset(requestP[curFd].buf, 0, sizeof(requestP[curFd].buf));
    } else if (strcmp(requestP[curFd].buf, "exit") == 0) {
        close(requestP[curFd].conn_fd);
        memset(requestP[curFd].buf, 0, sizeof(requestP[curFd].buf));
    }
}

void handlePostFrom(int curFd, int BulletinFd) {
    lseek(BulletinFd, RECORD_LEN * last, SEEK_SET);
    write(BulletinFd, requestP[curFd].buf, FROM_LEN);
    requestP[curFd].status = POST_CONTENT;
    memset(requestP[curFd].buf, 0, sizeof(requestP[curFd].buf));
}

void handlePostContent(int curFd, int BulletinFd) {
    lseek(BulletinFd, RECORD_LEN * last + FROM_LEN, SEEK_SET);
    write(BulletinFd, requestP[curFd].buf, CONTENT_LEN);
    memset(requestP[curFd].buf, 0, sizeof(requestP[curFd].buf));
    requestP[curFd].status = WAITING;
    lseek(BulletinFd, RECORD_LEN * last, SEEK_SET);
    read(BulletinFd, requestP[curFd].buf, FROM_LEN);
    printf("[Log] Receive post from %s", requestP[curFd].buf);
    memset(requestP[curFd].buf, 0, sizeof(requestP[curFd].buf));
}

void handlePull(int curFd, int BulletinFd) {
    lseek(BulletinFd, 0, SEEK_SET);
    // read(BulletinFd, requestP[curFd].buf, 20);
    read(BulletinFd, requestP[curFd].buf, FROM_LEN);
    printf("%s", requestP[curFd].buf);
    char str[10] = "abc";
    strcat(str, requestP[curFd].buf);
    fprintf(stderr, "%s", requestP[curFd].buf);
    send(curFd, requestP[curFd].buf, BUFFER_SIZE, 0);
    memset(requestP[curFd].buf, 0, sizeof(requestP[curFd].buf));
}


// ======================================================================================================
// You don't need to know how the following codes are working

static void init_request(request* reqP) {
    reqP->conn_fd = -1;
    reqP->buf_len = 0;
    reqP->id = 0;
}

static void free_request(request* reqP) {
    init_request(reqP);
}

static void init_server(unsigned short port) {
    struct sockaddr_in servaddr;
    int tmp;

    gethostname(svr.hostname, sizeof(svr.hostname));
    svr.port = port;

    svr.listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (svr.listen_fd < 0) ERR_EXIT("socket");

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    tmp = 1;
    if (setsockopt(svr.listen_fd, SOL_SOCKET, SO_REUSEADDR, (void*)&tmp, sizeof(tmp)) < 0) {
        ERR_EXIT("setsockopt");
    }
    if (bind(svr.listen_fd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ERR_EXIT("bind");
    }
    if (listen(svr.listen_fd, 1024) < 0) {
        ERR_EXIT("listen");
    }

    // Get file descripter table size and initialize request table
    maxfd = getdtablesize();
    requestP = (request*) malloc(sizeof(request) * maxfd);
    if (requestP == NULL) {
        ERR_EXIT("out of memory allocating all requests");
    }
    for (int i = 0; i < maxfd; i++) {
        init_request(&requestP[i]);
    }
    requestP[svr.listen_fd].conn_fd = svr.listen_fd;
    strcpy(requestP[svr.listen_fd].host, svr.hostname);

    return;
}
