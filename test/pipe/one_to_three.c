#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int i, pid;
    char buf[128];
    int fd[2];

    pipe(fd);
    // section A
    for (i = 1; i < 4; i++) {
        // section B
        pid = fork();
        if (pid == 0) {
            // section C
            close(fd[0]);
            dup2(fd[1], 1);
            execlp(argv[i], argv[i], (char *) 0);
        }
        // section D
    }
    
    close(fd[1]);

    int status;
    int cnt = 3;
    while (cnt) {
        ssize_t bytesRead;
        while (bytesRead = read(fd[0], buf, sizeof(buf)) > 0) {
            write(1, buf, bytesRead);
        }
        pid = waitpid(-1, &status, WNOHANG);
        if (pid) {
            cnt--;
        }
    }
    exit(0);
}
