#define _GNU_SOURCE
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#include "util.h"

#define ERR_EXIT(s) perror(s), exit(errno);

typedef struct node {
    // char service_name[MAX_SERVICE_NAME_LEN];
    int pfd[2];
    struct node *last, *next;
} node;

static unsigned long secret;
static char service_name[MAX_SERVICE_NAME_LEN];
char buf[MAX_CMD_LEN];
char fifo1[MAX_FIFO_NAME_LEN], fifo2[MAX_FIFO_NAME_LEN], fifo3[MAX_FIFO_NAME_LEN], fifo4[MAX_FIFO_NAME_LEN];
node *head = NULL;
bool exitFlag = false;

static inline bool is_manager() {
    return strcmp(service_name, "Manager") == 0;
}

void print_not_exist(char *service_name) {
    printf("%s doesn't exist\n", service_name);
}

void print_receive_command(char *service_name, char *cmd) {
    printf("%s has received %s\n", service_name, cmd);
}

void print_spawn(char *parent_name, char *child_name) {
    printf("%s has spawned a new service %s\n", parent_name, child_name);
}

void print_kill(char *target_name, int decendents_num) {
    printf("%s and %d child services are killed\n", target_name, decendents_num);
}

void print_acquire_secret(char *service_a, char *service_b, unsigned long secret) {
    printf("%s has acquired a new secret from %s, value: %lu\n", service_a, service_b, secret);
}

void print_exchange(char *service_a, char *service_b) {
    printf("%s and %s have exchanged their secrets\n", service_a, service_b);
}

void commandHandler();

// node *findNode(node *head, char target[]);

bool spawnHandler(char target[], char newChild[]);

int killHandler(char target[]);

int killRecur();

int exchangeHandler(char target1[], char target2[]);

void parentHandler();

int childHandler(node *cur);

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(int argc, char *argv[]) {
    pid_t pid = getpid();    

    if (argc != 2) {
        fprintf(stderr, "Usage: ./service [service_name]\n");
        return 0;
    }

    srand(pid);
    secret = rand();
    /* 
     * prevent buffered I/O
     * equivalent to fflush() after each stdout
     */
    setvbuf(stdout, NULL, _IONBF, 0);

    strncpy(service_name, argv[1], MAX_SERVICE_NAME_LEN);

    printf("%s has been spawned, pid: %d, secret: %lu\n", service_name, pid, secret);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    head = realloc(head, sizeof(node));

    if (strcmp(service_name, "Manager") == 0) {
        while (1) {
            if (exitFlag) exit(0);
            commandHandler();
        }
    } else {
        write(4, "done", 4);
        while (1) {
            if (exitFlag) exit(0);
            parentHandler();
        }
    }
    
    return 0;
}

void commandHandler() {
    memset(buf, 0, MAX_CMD_LEN);
    fgets(buf, MAX_CMD_LEN, stdin);
    char command[9], target1[MAX_SERVICE_NAME_LEN], target2[MAX_SERVICE_NAME_LEN];
    sscanf(buf, "%s %s %s", command, target1, target2);
    if (strcmp(command, "spawn") == 0) {
        if (spawnHandler(target1, target2)) {
            print_spawn(target1, target2);
        } else {
            print_not_exist(target1);
        }
    } else if (strcmp(command, "kill") == 0) {
        int num = killHandler(target1);
        if (num) {
            print_kill(target1, num - 1);
        } else {
            print_not_exist(target1);
        }
    } else if (strcmp(command, "exchange") == 0) {
        memset(fifo1, 0, MAX_FIFO_NAME_LEN);
        sprintf(fifo1, "%s_to_%s.fifo", target1, target2);
        mkfifo(fifo1, 0600);
        int fifofd1 = open(fifo1, O_RDWR);
        memset(fifo2, 0, MAX_FIFO_NAME_LEN);
        sprintf(fifo2, "%s_to_%s.fifo", target2, target1);
        mkfifo(fifo2, 0600);
        int fifofd2 = open(fifo2, O_RDWR);
        memset(fifo3, 0, MAX_FIFO_NAME_LEN);
        sprintf(fifo3, "%s_and_%s.fifo", target1, target2);
        mkfifo(fifo3, 0600);
        int fifofd3 = open(fifo3, O_RDWR);
        memset(fifo4, 0, MAX_FIFO_NAME_LEN);
        sprintf(fifo4, "%s_and_Manager.fifo", target2);
        mkfifo(fifo4, 0600);
        int fifofd4 = open(fifo4, O_RDWR);
        exchangeHandler(target1, target2);
        if (strcmp(service_name, target1) == 0) {
            print_acquire_secret(target1, target2, secret);
            int fifofd5 = open(fifo3, O_WRONLY);
            write(fifofd5, "done", 4);
            close(fifofd5);
        } else if (strcmp(service_name, target2) == 0) {
            int fifofd5 = open(fifo3, O_RDONLY);
            memset(buf, 0, MAX_CMD_LEN);
            read(fifofd5, buf, MAX_CMD_LEN);
            print_acquire_secret(target2, target1, secret);
            int fifofd6 = open(fifo4, O_WRONLY);
            write(fifofd6, "done", 4);
            close(fifofd5);
            close(fifofd6);
        }
        memset(buf, 0, MAX_CMD_LEN);
        read(fifofd4, buf, 4);
        print_exchange(target1, target2);
        close (fifofd1);
        close (fifofd2);
        close (fifofd3);
        close (fifofd4);
        unlink(fifo1);
        unlink(fifo2);
        unlink(fifo3);
        unlink(fifo4);
    }
}

bool spawnHandler(char target[], char newChild[]) {
    memset(buf, 0, MAX_CMD_LEN);
    sprintf(buf, "spawn %s %s", target, newChild);
    print_receive_command(service_name, buf);
    if (strcmp(service_name, target) == 0) {
        node *cur = head;
        while (cur -> next) {
            cur = cur -> next;
        }
        cur -> next = malloc(sizeof(node));
        cur -> next -> last = cur;
        cur = cur -> next;
        cur -> next = NULL;
        int pfd1[2], pfd2[2];
        pipe2(pfd1, O_CLOEXEC);
        pipe2(pfd2, O_CLOEXEC);
        if (fork() == 0) {
            if (pfd1[0] != 3) {
                close(pfd1[1]);
                dup2(pfd1[0], 3);
                close(pfd1[0]);
            }
            if (pfd2[1] != 4) {
                close(pfd2[0]);
                dup2(pfd2[1], 4);
                close(pfd2[1]);
            }
            fcntl(3, F_SETFD, 0);
            fcntl(4, F_SETFD, 0);
            
            execlp("./service", "./service", newChild, (char *)0);
        }
        cur -> pfd[0] = pfd2[0];
        cur -> pfd[1] = pfd1[1];
        close(pfd1[0]);
        close(pfd2[1]);
        memset(buf, 0, MAX_CMD_LEN);
        read(cur -> pfd[0], buf, MAX_CMD_LEN);
        // printf("%s\n", buf);
        return true;
    }
    node *cur = head -> next;
    while (cur) {
        memset(buf, 0, MAX_CMD_LEN);
        sprintf(buf, "spawn %s %s", target, newChild);
        if (childHandler(cur)) {
            return true;
        }
        cur = cur -> next;
    }
    return false;
}

int killHandler(char target[]) {
    memset(buf, 0, MAX_CMD_LEN);
    sprintf(buf, "kill %s", target);
    print_receive_command(service_name, buf);
    if (strcmp(service_name, target) == 0) {
        exitFlag = true;
        if (strcmp(service_name, "Manager") != 0) {
            memset(buf, 0, MAX_CMD_LEN);
            write(4, "waitme", 6);
        }
        return killRecur();
    }
    node *cur = head -> next;
    while (cur) {
        memset(buf, 0, MAX_CMD_LEN);
        sprintf(buf, "kill %s", target);
        int num = childHandler(cur);
        if (num) {
            if (cur -> next) {
                cur -> next -> last = cur -> last;
            }
            cur -> last -> next = cur -> next;
            free(cur);
            return num;
        }
        cur = cur -> next;
    }
    return 0;
}

int killRecur() {
    node *cur = head, *cur2;
    int num = 0;
    while (cur -> next) {
        cur2 = cur;
        cur = cur -> next;
        free(cur2);
        write(cur -> pfd[1], "killRecur", 9);
        memset(buf, 0, MAX_CMD_LEN);
        read(cur -> pfd[0], buf, 2);
        int status;
        wait(&status);
        num += atoi(buf);
    }
    free(cur);
    return num + 1;
}

void openFifo(char target1[], char target2[]) {
    memset(fifo1, 0, MAX_FIFO_NAME_LEN);
    sprintf(fifo1, "%s_to_%s.fifo", target1, target2);
    memset(fifo2, 0, MAX_FIFO_NAME_LEN);
    sprintf(fifo2, "%s_to_%s.fifo", target2, target1);
    memset(fifo3, 0, MAX_FIFO_NAME_LEN);
    sprintf(fifo3, "%s_and_%s.fifo", target1, target2);
    memset(fifo4, 0, MAX_FIFO_NAME_LEN);
    sprintf(fifo4, "%s_and_Manager.fifo", target2);
}

int exchangeHandler(char target1[], char target2[]) {
    memset(buf, 0, MAX_CMD_LEN);
    sprintf(buf, "exchange %s %s", target1, target2);
    print_receive_command(service_name, buf);
    int num = 0;
    int fifofd1, fifofd2;
    if (strcmp(service_name, target1) == 0) {
        openFifo(target1, target2);
        fifofd1 = open(fifo1, O_WRONLY);
        memset(buf, 0, MAX_CMD_LEN);
        sprintf(buf, "%ld", secret);
        write(fifofd1, buf, MAX_CMD_LEN);
        close(fifofd1);
        num += 1;
    } else if (strcmp(service_name, target2) == 0) {
        openFifo(target1, target2);
        fifofd2 = open(fifo2, O_WRONLY);
        memset(buf, 0, MAX_CMD_LEN);
        sprintf(buf, "%ld", secret);
        write(fifofd2, buf, MAX_CMD_LEN);
        close(fifofd2);
        num += 2;
    }
    node *cur = head -> next;
    while (cur) {
        memset(buf, 0, MAX_CMD_LEN);
        sprintf(buf, "exchange %s %s", target1, target2);
        num += childHandler(cur);
        if (num == 3) {
            break;
        }
        cur = cur -> next;
    }
    if (strcmp(service_name, target1) == 0) {
        fifofd2 = open(fifo2, O_RDONLY);
        memset(buf, 0, MAX_CMD_LEN);
        read(fifofd2, buf, MAX_CMD_LEN);
        secret = (unsigned long)atol(buf);
        close(fifofd2);
    } else if (strcmp(service_name, target2) == 0) {
        fifofd1 = open(fifo1, O_RDONLY);
        memset(buf, 0, MAX_CMD_LEN);
        read(fifofd1, buf, MAX_CMD_LEN);
        secret = (unsigned long)atol(buf);
        close(fifofd1);
    }
    return num;
}

void parentHandler() {
    memset(buf, 0, MAX_CMD_LEN);
    read(3, buf, MAX_CMD_LEN);
    char command[9], target1[MAX_SERVICE_NAME_LEN], target2[MAX_SERVICE_NAME_LEN];
    sscanf(buf, "%s %s %s", command, target1, target2);
    if (strcmp(command, "spawn") == 0) {
        if (spawnHandler(target1, target2)) {
            write(4, "1", 1);
        } else {
            write(4, "0", 1);
        }
    } else if (strcmp(command, "kill") == 0) {
        int num = killHandler(target1);
        if (num) {
            memset(buf, 0, MAX_CMD_LEN);
            sprintf(buf, "%d", num);
            write(4, buf, 2);
        } else {
            write(4, "0", 1);
        }
    } else if (strcmp(command, "killRecur") == 0) {
        memset(buf, 0, MAX_CMD_LEN);
        sprintf(buf, "%d", killRecur());
        write(4, buf, 2);
        exit(0);
    } else if (strcmp(command, "exchange") == 0) {
        memset(buf, 0, MAX_CMD_LEN);
        sprintf(buf, "%d", exchangeHandler(target1, target2));
        write(4, buf, 1);
        if (strcmp(service_name, target1) == 0) {
            print_acquire_secret(target1, target2, secret);
            int fifofd3 = open(fifo3, O_WRONLY);
            write(fifofd3, "done", 4);
            close(fifofd3);
        } else if (strcmp(service_name, target2) == 0) {
            int fifofd3 = open(fifo3, O_RDONLY);
            memset(buf, 0, MAX_CMD_LEN);
            read(fifofd3, buf, MAX_CMD_LEN);
            print_acquire_secret(target2, target1, secret);
            int fifofd4 = open(fifo4, O_WRONLY);
            write(fifofd4, "done", 4);
            close(fifofd3);
            close(fifofd4);
        }
    }
}

int childHandler(node *cur) {
    write(cur -> pfd[1], buf, MAX_CMD_LEN);
    memset(buf, 0, MAX_CMD_LEN);
    read(cur -> pfd[0], buf, 6);
    if (strcmp(buf, "waitme") == 0) {
        int status;
        wait(&status);
        memset(buf, 0, MAX_CMD_LEN);
        read(cur -> pfd[0], buf, MAX_CMD_LEN);
    }
    return atoi(buf);
}