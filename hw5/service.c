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
    struct node *next;
} node;

static unsigned long secret;
static char service_name[MAX_SERVICE_NAME_LEN];
char buf[MAX_CMD_LEN];
node *head = NULL;

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

bool killHandler();

bool exchangeHandler();

void parentHandler();

bool childHandler(node *cur);

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
    
    if (strcmp(service_name, "Manager") == 0) {
        while (1) {
            commandHandler();
        }
    } else {
        write(4, "done", 4);
        while (1) {
            parentHandler();
        }
    }
    
    return 0;
}

void commandHandler() {
    memset(buf, 0, MAX_CMD_LEN);
    fgets(buf, sizeof(buf), stdin);
    char command[8], target1[MAX_SERVICE_NAME_LEN], target2[MAX_SERVICE_NAME_LEN];
    sscanf(buf, "%s %s %s", command, target1, target2);
    if (strcmp(command, "spawn") == 0) {
        if (spawnHandler(target1, target2)) {
            print_spawn(target1, target2);
        } else {
            print_not_exist(target1);
        }
    } else if (strcmp(command, "kill") == 0) {
        killHandler();
    } else if (strcmp(command, "exchange") == 0) {
        exchangeHandler();
    }
}

// node *findNode(node *head, char target[]) {
//     node *cur = head;
//     while (cur) {
//         if (strcmp(cur -> service_name, target) == 0) {
//             return cur;
//         }
//         cur = cur -> next;
//     }
//     return NULL;
// }

bool spawnHandler(char target[], char newChild[]) {
    print_receive_command(service_name, "spawn");
    if (strcmp(service_name, target) == 0) {
        node *cur = head;
        while (cur) {
            cur = cur -> next;
        }
        cur = malloc(sizeof(node));
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
    node *cur = head;
    while (cur) {
        memset(buf, 0, MAX_CMD_LEN);
        sprintf(buf, "spawn %s %s\n", target, newChild);
        if (childHandler(cur)) {
            return true;
        }
        cur = cur -> next;
    }
    return false;
}

bool killHandler() {
    
}

bool exchangeHandler() {
    
}

void parentHandler() {
    memset(buf, 0, MAX_CMD_LEN);
    read(3, buf, MAX_CMD_LEN);
    printf("%s\n", buf);
    char command[8], target1[MAX_SERVICE_NAME_LEN], target2[MAX_SERVICE_NAME_LEN];
    sscanf(buf, "%s %s %s", command, target1, target2);
    if (strcmp(command, "spawn") == 0) {
        if (spawnHandler(target1, target2)) {
            print_spawn(target1, target2);
        } else {
            print_not_exist(target1);
        }
    } else if (strcmp(command, "kill") == 0) {
        killHandler();
    } else if (strcmp(command, "exchange") == 0) {
        exchangeHandler();
    }
}

bool childHandler(node *cur) {
    write(cur -> pfd[1], buf, MAX_CMD_LEN);
    memset(buf, 0, MAX_CMD_LEN);
    read(cur -> pfd[0], buf, MAX_CMD_LEN);
    printf("hi\n");
    return (strcmp(buf, "success") == 0) ? true : false;
}