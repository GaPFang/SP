#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    int fd = open("./BulletinBoard", O_WRONLY | O_CREAT, 0666);
    struct flock *lock = malloc(sizeof(struct flock) * 10);
    for (int i = 0; i < 10; i++) {
        lock[i].l_len = 25;
        lock[i].l_start = 25 * i;
        lock[i].l_type = F_WRLCK;
        lock[i].l_whence = SEEK_SET;
    }
    int arr[9] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
    for (int i = 0; i < 9; i++) {
        fcntl(fd, F_SETLK, &lock[arr[i]]);
    }
    // for (int i = 0; i < 10; i++) {
    //     fcntl(fd, F_GETLK, &lock[i]);
    //     printf("%d ", lock[i].l_type);
    // }
    // printf("\n");
    sleep(-1);
    return 0;
}