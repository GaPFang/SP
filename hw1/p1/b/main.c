#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int fd1, fd2;

    fd1 = open("in.txt", O_RDONLY);
    fd2 = open("out.txt", O_WRONLY | O_CREAT, 0666);

    dup2(fd1, STDIN_FILENO);
    dup2(fd2, STDOUT_FILENO);

    execlp("./a.out", "./a.out", (char *)0);

    return 0;
}