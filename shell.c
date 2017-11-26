#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "shellutil.h"

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
void split(char *buf, statement* statement, size_t max) {
    char *token = strtok(buf, TOKEN_DELIMETER);             // Break on any new line, tab, space, or return
    int c = 0;

    while (token != NULL) {
        char commandBroken = isCommandBreak(token);
        if (strlen(token)>0 && commandBroken) { // TODO Allow speech etc. to happen before ;

            if(commandBroken == ';') {
                if(token[0] == ';') {
                    statement->argv[c] = 0;
                } else {
                    token[strlen(token) -1] = 0;
                    addToArgV(&c, statement, token);
                    statement->argv[c + 1] = 0;
                }
            }

            if(c > 0) {
                size_t finalStrLength = strlen(statement->argv[c - 1]);
                if(commandBroken == '&') {
                    statement->blocking = 1;
                    if(finalStrLength == 1) {
                        c--;
                    }
                } else {
                    statement->blocking = 0;
                }
            }

            statement->argc = c;
            statement->terminator = commandBroken;
            c = 0;

            statement->next = createStatement();
            statement = statement->next;

        } else if (strncmp(token, "\"\"", 2) == 0) {               // Ignore `""`
            strncpy(statement->argv[c], "", 1);
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
                addToArgV(&c, statement, comb);
            } else {
                *atSpeechMark = 0;                          // Convert speech mark to null character
                statement->argv[c] = malloc(sizeof(char*));
                addToArgV(&c, statement, comb);
                char *v = (atSpeechMark + 1);
                if (*v != '\0') {
                    atSpeechMark++;
                    addToArgV(&c, statement, atSpeechMark);
                }
            }

        } else {
            addToArgV(&c, statement, token);
        }
        token = strtok(NULL, TOKEN_DELIMETER);              // Move to next token
    }


    statement->argv[c] = 0;                                           // Null terminate final array item
    statement->argc = c;
    statement->next = NULL;

}


/**
 * Executes the given array of commands in a subprocess
 * @param split the array of strings representing a command
 */
void execute(statement *stmt) {
    int block = stmt->blocking;
    char **split = stmt->argv;

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
    size_t inputBufferSize = sizeof(char) * BUFFER_SIZE;

    /*if (argc > 1) {
        execute(argv + 1, 0);                                  // Execute initially provided parameters
    }*/

    while (&free) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s %s", cwd, PROMPT);                   // Show prompt with path
        } else {
            printf("%s", PROMPT);                           // Show prompt without path
        }
        char *buf = malloc(inputBufferSize * sizeof(char *));
        read_line(buf, inputBufferSize);

        statement* temp = createStatement();

        split(buf, temp, MAX_ARG_LENGTH);                   // Split arguments


        for (; temp != NULL; temp = temp->next) {
            if (temp->argv[0] != NULL) {
                if (strncmp(temp->argv[0], "exit", 4) == 0) {         // On `exit`, exit
                    printf("Exiting...\n");
                    return EXIT_SUCCESS;
                } else if (strncmp(temp->argv[0], "cd", 2) == 0) {    // On `cd`, change directory
                    if (chdir(temp->argv[1]) != 0) {
                        perror(PROGRAM_NAME);
                    }
                } else if (strncmp(temp->argv[0], "help", 4) == 0) {  // On `help`, show help
                    printHelp();
                } else {
                    execute(temp);                    // Otherwise, perform standard execute
                }
            }
        }
    }
    return EXIT_FAILURE;
}

