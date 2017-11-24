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
#define TOKEN_DELIMETER " \t\r\n\a"
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
char *read_line(char *buf, size_t sz) {
    fgets(buf, sz, stdin);

    size_t ln = strlen(buf) - 1;

    if (feof(stdin)) {
        exit(EXIT_SUCCESS);                                 // Exit when CTRL-D is received
    }

    if (*buf && buf[ln] == '\n') {
        buf[ln] = '\0';                                     // Replace new line with null terminator
    } else {
        buf[ln + 1] = '\0';                                 // Add null terminator
    }

    if (strlen(buf) == 0) {                                 // Return null if the string is empty after manipulation
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
void split(char *buf, char *split[], int* blocking, size_t max) {
    char *token = strtok(buf, TOKEN_DELIMETER);             // Break on any new line, tab, space, or return
    int c = 0;
    while (token != NULL) {
        split[c] = (char *) malloc(max);                    // Allocate memory for the next parameter in the array
        if (strncmp(token, "\"\"", 2) == 0) {               // Ignore `""`
            strncpy(split[c], "", 1);
            c++;
        } else if (token[0] == '\"') {                      // Case: on an opening speech mark
            char *comb = malloc(max);                       // Allocate a space for the concatenated strings
            token++;                                        // Ignore the opening mark

            if(strchr(token, '"')) {
                strncpy(comb, token, strlen(token));
            } else {
                while(token != NULL && !strchr(token, '"')) {
                    strncat(comb, token, strlen(token));     // Add the string to 'comb'
                    token = strtok(NULL, TOKEN_DELIMETER);   // Move to next token
                    if (token != NULL) {
                        strcat(comb, " ");                   // Add space to parameters
                        if (strchr(token, '"')) {
                            strncat(comb, token, strlen(token)); // Add the string to 'comb'
                        }
                    }

                }
            }
            // HANDLE "a edge case
            char *atSpeechMark = strchr(comb, '"');
            if(atSpeechMark == NULL) {
                comb[strlen(comb)] = '\0';
                strcpy(split[c], comb);
                c++;
            } else {
                *atSpeechMark = 0;                          // Convert speech mark to null character
                strcpy(split[c], comb);
                c++;
                char *v = (atSpeechMark + 1);
                if (*v != '\0') {
                    split[c] = (char *) malloc(max);
                    atSpeechMark++;
                    strcpy(split[c], atSpeechMark);
                    c++;
                }
            }

        } else {
            strncpy(split[c], token, strlen(token));        // Add token to token array
            c++;
        }
        token = strtok(NULL, TOKEN_DELIMETER);              // Move to next token
    }

    if(c > 0) {
        size_t finalStrLength = strlen(split[c - 1]);
        if(split[c - 1][finalStrLength - 1] == '&') {
            *blocking = 1;
            split[c - 1][finalStrLength - 1] = '\0';
            if(finalStrLength == 1) {
                c--;
            }
        } else {
            *blocking = 0;
        }
    }

    split[c] = 0;                                           // Null terminate final array item

}

/**
 * Executes the given array of commands in a subprocess
 * @param split the array of strings representing a command
 */
void execute(char *split[], int block) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(split[0], split);                            // Execute and look in PATH for binaries
        if (errno) {
            perror(PROGRAM_NAME);                           // Show error from subprocess
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else {
        waitpid(pid, NULL, (block == 1)?WNOHANG:0);                               // Wait for subprocess to finish
        if(block == 1) {
            printf("[%d]\n", pid);
        }
    }
}


int main(int argc, char *argv[]) {
    int inputBufferSize = sizeof(char) * BUFFER_SIZE;

    if (argc > 1) {
        execute(argv + 1, 0);                                  // Execute initially provided parameters
    }

    while (&free) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s %s", cwd, PROMPT);                   // Show prompt with path
        } else {
            printf("%s", PROMPT);                           // Show prompt without path
        }
        char *buf = malloc(inputBufferSize * sizeof(char *));
        read_line(buf, inputBufferSize);
        char *args[sizeof(char *) * MAX_ARG_LENGTH * MAX_ARGS];

        int blocking = 0;
        split(buf, args, &blocking, MAX_ARG_LENGTH);                   // Split arguments

        if (args[0] != NULL) {
            if (strncmp(args[0], "exit", 4) == 0) {         // On `exit`, exit
                printf("Exiting...\n");
                return EXIT_SUCCESS;
            } else if (strncmp(args[0], "cd", 2) == 0) {    // On `cd`, change directory
                if (chdir(args[1]) != 0) {
                    perror(PROGRAM_NAME);
                }
            } else if (strncmp(args[0], "help", 4) == 0) {  // On `help`, show help
                printHelp();
            } else {
                execute(args, blocking);                    // Otherwise, perform standard execute
            }
        }
        free(buf);
    }
    return EXIT_FAILURE;
}

