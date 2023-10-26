#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    int i, pid;
    int pfd[3];
    char buf[128];

    // section A
    for (i = 1; i < 4; i++) {
        // section B
        int fd[2];
        pipe(fd);
        pid = fork();
        if (pid == 0) {
            // section C
            close(fd[0]);
            dup2(fd[1], 1);
            close(fd[1]);
            execlp(argv[i], argv[i], (char *) 0);
        }
        // section D
        close(fd[1]);
        pfd[i - 1] = fd[0];
    }

    int status;
    int cnt = 3;
    while (cnt) {
        ssize_t bytesRead;
        for (int i = 0; i < 3; i++) {
            bytesRead = read(pfd[i], buf, sizeof(buf));
            if (bytesRead > 0) write(1, buf, bytesRead);
        }
        pid = waitpid(-1, &status, WNOHANG);
        if (pid) {
            cnt--;
        }
    }
    exit(0);
}
