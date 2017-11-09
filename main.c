#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void readLine(char* buf, int sz) {
    fgets(buf, sz, stdin);
}

void split(char* buf, char* split[], size_t max) { //TODO use max
    char* token = strtok(buf, " ");
    int c = 0;
    while (token != NULL)
    {
        split[c] = (char*) malloc(100);
        strcpy(split[c], token);
        c ++;
        token = strtok(NULL, " ");
    }
    split[c+1] = "\0";
}


void execute(char* split[]) {
    pid_t pid = fork();
    if(pid == 0) {
        if(execv(split[0], split) == -1) {
            perror(split[0]);
            exit(0);
        }
    } else {
        wait(NULL);
    }
}


int main(int argc, char* argv[]) {
    int MAX_ARGS = 100;
    while(&free) {
        printf("> ");
        char buf[100];
        readLine(buf, 100);

        char newBuf[100];
        strncpy(newBuf, buf, strlen(buf)-1);
        char* args[100];

        split(newBuf, args, 100);
        if(strncmp(args[0], "exit", 4) == 0) {
            printf("Exiting...");
            return EXIT_SUCCESS;
        } else {
            execute(args);
        }
    }
    return EXIT_FAILURE;
}
