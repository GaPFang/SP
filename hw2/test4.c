#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int main() {
    int fd = open("test.txt", O_WRONLY | O_CREAT, 0666);
    char buf[10] = "12345678";
    memset(buf, 0, 10);
    pwrite(fd, buf, 10, 0);
    pwrite(fd, buf, 10, 0);
    return 0;
}