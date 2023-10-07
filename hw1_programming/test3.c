#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int main() {
    int fd = open("in.txt", O_RDWR | O_CREAT, 0666);
    struct pollfd *fdArray = (struct pollfd *)malloc(sizeof(struct pollfd));
    fdArray[0].fd = fd;
    fdArray[0].events = POLLIN;
    poll(fdArray, 1, -1);
    printf("done\n");
    return 0;
}