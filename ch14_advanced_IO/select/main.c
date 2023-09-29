#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

int main() {
    int totalFds;
    fd_set readFds, writeFds, errorFds;
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    FD_ZERO(&readFds);
    FD_ZERO(&writeFds);
    FD_ZERO(&errorFds);
    FD_SET(0, &readFds);
    FD_SET(1, &readFds);
    FD_SET(1, &writeFds);
    totalFds = select(2, &readFds, &writeFds, &errorFds, &timeout);
    fprintf(stderr, "%d\n", totalFds);
    fprintf(stderr, "%d\n", FD_ISSET(STDIN_FILENO, &readFds));
    fprintf(stderr, "%d\n", FD_ISSET(STDOUT_FILENO, &readFds));
    fprintf(stderr, "%d\n", FD_ISSET(STDOUT_FILENO, &writeFds));
    return 0;
}