#include <poll.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    // struct pollfd temp[1];
    // temp[0].fd = 2;
    // temp[0].events = 0;
    // poll(temp, 1, 10000);
    char buf[10] = "apple";

    int fd = open("./BulletinBoard.txt", O_WRONLY);
    pwrite(fd, buf, 5, 0);
    struct flock lock1, lock2;
    lock1.l_len = 5;
    lock1.l_start = 0;
    lock1.l_type = F_WRLCK;
    lock1.l_whence = SEEK_SET;
    lock2.l_len = 5;
    lock2.l_start = 0;
    lock2.l_type = F_RDLCK;
    lock2.l_whence = SEEK_SET;
    fcntl(fd, F_GETLK, &lock1);
    printf("%d ", lock1.l_type - F_UNLCK);
    lock1.l_type = F_WRLCK;
    fcntl(fd, F_SETLK, &lock1);
    fcntl(fd, F_GETLK, &lock2);
    printf("%d %d\n", lock1.l_type - F_WRLCK, lock2.l_type - F_UNLCK);
    return 0;
}