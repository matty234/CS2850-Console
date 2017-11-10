#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 1000
#define MAX_ARGS 20
#define MAX_ARG_LENGTH 100
#define PROGRAM_NAME "mash"
#define PROMPT "\e[1;31mMASH >\033[0m "

/*
 * By 6602
 * All questions answered
 */

/**
 * Reads a line from the console and returns a character array.
 * @param sz the size of the buffer to read
 * @return the entered line
 */
char* readLine(int sz) {
    char* buf = malloc(sz * sizeof(char*));
    fgets(buf, sz, stdin);


    size_t ln = strlen(buf) - 1;

    if (*buf && buf[ln] == '\n') {
        buf[ln] = '\0';
    } else {
        buf[ln+1] = '\0';
    }

    if(strchr(buf, 4)){ // Deal with Control-D (EOL) (incorrectly)
        exit(EXIT_SUCCESS);
    }
    if(strlen(buf) == 0) {
        return NULL;
    } else {
        return buf;
    }
}

/**
 * Prints a simple help message
 */
void printHelp() {
    printf("MASH - The Matt Again Shell\n\n");
    printf("Built-Ins: cd, exit, help\n");
}

/**
 * Tokenises a string by the space delimeter and adds a null value to array
 * @param buf a pointer to the string
 * @param split a pointer to the output string array
 * @param max the maximum length of each parameter
 */
void split(char* buf, char* split[], size_t max) {
    char* token = strtok(buf, " \t\r\n\a");
    int c = 0;
    while (token != NULL)
    {
        split[c] = (char*) malloc(max);
        if(token[0] == '"') {
            char* comb = malloc(max);
            do  {
                strncat(comb, token, strlen(token));
                token = strtok(NULL, " ");
                if(token != NULL && token[strlen(token)-1] == '\"') {
                    strcat(comb, " ");
                    strncat(comb, token, strlen(token));
                    token = NULL;
                }
            } while (token != NULL);
            comb++;
            comb[strlen(comb)-1] = 0;
            strncpy(split[c], comb, strlen(comb));
            c ++;
        } else {
            strncpy(split[c], token, strlen(token));
            c ++;
        }
        token = strtok(NULL, " ");
    }
    split[c] = 0;
}

/**
 * Executes the given array of commands in a subprocess
 * @param split the array of strings representing a command
 */
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
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s %s",cwd, PROMPT);
        } else {
            printf("%s>", PROMPT);
        }

        char* buf = readLine(inputBufferSize);
        char* args[sizeof(char*)*MAX_ARG_LENGTH*MAX_ARGS];
        split(buf, args, MAX_ARG_LENGTH);

        if(args[0] != NULL) {
            if (strncmp(args[0], "exit", 4) == 0) {
                printf("Exiting...");
                return EXIT_SUCCESS;
            } else if (strncmp(args[0], "cd", 2) == 0) {
                chdir(args[1]);
                if (errno) {
                    perror(PROGRAM_NAME);
                }
            } else if (strncmp(args[0], "help", 4) == 0) {
                printHelp();
            } else {
                execute(args);
            }
        }
    }
    return EXIT_FAILURE;
}
