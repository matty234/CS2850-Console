#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 1000
#define MAX_ARGS 20
#define MAX_ARG_LENGTH 100
#define PROGRAM_NAME "mash"
#define PROMPT "> "

char* readLine( int sz) {
    char* buf = malloc(sz * sizeof(char*));
    fgets(buf, sz, stdin);
    size_t ln = strlen(buf) - 1;

    if (*buf && buf[ln] == '\n') {
        buf[ln] = '\0';
    } else {
        buf[ln+1] = '\0';
    }

    char* l = strchr(buf, 4);
    if(l){
        exit(EXIT_SUCCESS);
    }
    return buf;
}

void printHelp() {
    printf("MASH - The Matt Shell\n\n");
    printf("Built-Ins: cd, exit, help\n");
}

void split(char* buf, char* split[], size_t max) { //TODO use max
    char* token = strtok(buf, " \t\r\n\a");
    int c = 0;
    while (token != NULL)
    {
        split[c] = (char*) malloc(max);
        strncpy(split[c], token, strlen(token));
        c ++;
        token = strtok(NULL, " ");
    }
    split[c] = 0;
}


void execute(char* split[]) {

    pid_t pid = fork();
    if(pid == 0) {
        execvp(split[0], split);
        if(errno) {
                perror(PROGRAM_NAME);
                exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else {
        wait(NULL);
    }
}


int main(int argc, char* argv[]) {
    int inputBufferSize = sizeof(char)*BUFFER_SIZE;
    while(&free) {
        printf(PROMPT);

        char* buf = readLine(inputBufferSize);
        char* args[sizeof(char*)*MAX_ARG_LENGTH*MAX_ARGS];
        split(buf, args, MAX_ARG_LENGTH);

        if(strncmp(args[0], "exit", 4) == 0) {
            printf("Exiting...");
            return EXIT_SUCCESS;
        } else if(strncmp(args[0], "cd", 2) == 0) {
            chdir(args[1]);
            if(errno) {
                perror(PROGRAM_NAME);
            }
        } else if(strncmp(args[0], "help", 4) == 0) {
            printHelp();
        } else {
            execute(args);
        }
    }
    return EXIT_FAILURE;
}
