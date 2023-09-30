#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    // int fd = open("./BulletinBoard.txt", O_WRONLY);
    // struct flock lock;
    // lock.l_len = 5;
    // lock.l_start = 0;
    // lock.l_type = F_WRLCK;
    // lock.l_whence = SEEK_SET;
    // int num = fcntl(fd, F_SETLK, &lock);
    // printf("%d\n", lock.l_type);
    // printf("%d\n", num);
    // return 0;

    int fd = open("./BulletinBoard.txt", O_WRONLY);
    struct flock lock;
    lock.l_len = 5;
    lock.l_start = 0;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    int num = fcntl(fd, F_SETLK, &lock);
    printf("%d %d\n", num, lock.l_type);
    return 0;
}