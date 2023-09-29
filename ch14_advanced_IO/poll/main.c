#include <stdio.h>
#include <poll.h>
#include <unistd.h>

int main() {
    int totalFds;
    struct pollfd fdArray[2];
    fdArray[0].fd = STDIN_FILENO;
    fdArray[0].events = POLLIN;
    fdArray[1].fd = STDOUT_FILENO;
    fdArray[1].events = POLLOUT;
    totalFds = poll(fdArray, 2, 5);
    printf("%d\n", totalFds);
    printf("%d\n", fdArray[0].revents);
    return 0;
}