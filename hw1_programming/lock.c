#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int fd = open("./BulletinBoard", O_WRONLY | O_CREAT, 0666);
    struct flock lock;
    lock.l_len = 25;
    lock.l_start = 0;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    int num = fcntl(fd, F_SETLK, &lock);
    sleep(-1);
    return 0;
}