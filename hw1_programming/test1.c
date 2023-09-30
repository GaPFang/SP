#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    // int fd = open("./BulletinBoard.txt", O_WRONLY);
    // struct flock lock;
    // lock.l_len = 5;
    // lock.l_start = 0;
    // lock.l_type = F_WRLCK;
    // printf("%d\n", lock.l_type);
    // lock.l_whence = SEEK_SET;
    // fcntl(fd, F_SETLK, &lock);
    // sleep(20);
    // return 0;

    int fd = open("./BulletinBoard.txt", O_WRONLY);
    struct flock lock;
    lock.l_len = 5;
    lock.l_start = 0;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    fcntl(fd, F_SETLK, &lock);
    sleep(20);
    return 0;
}