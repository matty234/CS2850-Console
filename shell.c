#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"
#include <sys/types.h>
#include <sys/wait.h>
#include "shellutil.h"
#pragma clang diagnostic pop

/*
 * By 6602
 * All questions answered
 */

/**
 * Prints a simple help message
 */
void printHelp() {
    printf("MASH - The Matt Again Shell\n\n");
    printf("Built-Ins: cd, exit, help\n");
}

/**
 * Tokenises a string by the space delimiter and adds a null value to array
 * @param buf a pointer to the string
 * @param split a pointer to the output string array
 * @param max the maximum length of each parameter
 */
void split_statements(char *buf, statement *statement, size_t max) {
    char *token = strtok(buf, TOKEN_DELIMETER);             // Break on any new line, tab, space, or return

    while (token != NULL) {
        char commandBroken = isCommandBreak(token);
        if (strlen(token)>0 && commandBroken) {

            if(commandBroken == ';') {
                if(token[0] == ';') {
                    statement->argv[statement->argc] = 0;
                } else {
                    token[strlen(token) -1] = 0;
                    addToArgV(statement, token);
                }
            }

            if(strlen(token) != 1 && commandBroken == '&') {
                token[strlen(token) -1] = 0;
                addToArgV(statement, token);
            }

            statement->argv[statement->argc] = 0;
            statement->terminator = commandBroken;


            statement->next = createStatement();

            if(commandBroken == '|') {

                FILE *readable;
                FILE *writable;

                fpipe(&readable, &writable);

                statement->output_redir = writable;
                statement = statement->next;
                statement->input_redir = readable;

            } else {
                statement = statement->next;
            }

        } else if (strcmp(token, "\"\"") == 0) {               // Ignore `""`
            addToArgV(statement, "");
        } else if (strlen(token) == 1 && token[0] == '>') {

            token = strtok(NULL, TOKEN_DELIMETER);
            statement->output_redir = fopen(token ,"w");

        } else if (strlen(token) == 2 && token[0] == '>' && token[1] == '>') {

            token = strtok(NULL, TOKEN_DELIMETER);
            statement->output_redir = fopen(token ,"a");

        } else if (strlen(token) == 1 && token[0] == '<') {

            token = strtok(NULL, TOKEN_DELIMETER);
            statement->input_redir = fopen(token ,"r");

        } else if (token[0] == '\"') {                      // Case: on an opening speech mark
            char *comb = malloc(max);                       // Allocate a space for the concatenated strings
            token++;                                        // Ignore the opening mark

            if(strchr(token, '"')) {
                strcpy(comb, token);
            } else {
                while(token != NULL && !strchr(token, '"')) {
                    strcat(comb, token);     // Add the string to 'comb'
                    token = strtok(NULL, TOKEN_DELIMETER);   // Move to next token
                    if (token != NULL) {
                        strcat(comb, " ");                   // Add space to parameters
                        if (strchr(token, '"')) {
                            strcat(comb, token); // Add the string to 'comb'
                        }
                    }

                }
            }
            // HANDLE "a edge case
            char *atSpeechMark = strchr(comb, '"');
            if(atSpeechMark == NULL) {
                comb[strlen(comb)] = '\0';
                addToArgV(statement, comb);
            } else {
                *atSpeechMark = 0;                          // Convert speech mark to null character
                statement->argv[statement->argc] = malloc(sizeof(char*));
                addToArgV(statement, comb);
                char *v = (atSpeechMark + 1);
                if (*v != '\0') {
                    atSpeechMark++;
                    addToArgV(statement, atSpeechMark);
                }
            }

        } else {
            addToArgV(statement, token);
        }
        token = strtok(NULL, TOKEN_DELIMETER);              // Move to next token
    }


    statement->argv[statement->argc] = 0;                                           // Null terminate final array item
    statement->next = NULL;

}


/**
 * Executes the given array of commands in a subprocess
 * @param split the array of strings representing a command
 */
void execute_statement(statement *stmt) {
    char **split = stmt->argv;
    int stdoutCopy = 0;
    int stdinCopy = 0;

    if(stmt->output_redir) {
        stdoutCopy = dup(STDOUT_FILENO);
        int outputFile = fileno(stmt->output_redir);
        if(dup2(outputFile, STDOUT_FILENO) < 0) return;
        close(outputFile);
    }

    if(stmt->input_redir) {
        stdinCopy = dup(STDIN_FILENO);
        int inputFile = fileno(stmt->input_redir);
        if(dup2(inputFile, STDIN_FILENO) < 0) return;
        close(inputFile);
    }

    pid_t pid = fork();
    if (pid == 0) {
        execvp(split[0], split);                            // Execute and look in PATH for binaries
        if (errno) {
            perror(PROGRAM_NAME);                           // Show error from subprocess
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else {
        waitpid(pid, NULL, (stmt->terminator == '&') ? WNOHANG : 0);                               // Wait for subprocess to finish
        if(stmt->terminator == '&') {
            printf("[%d]\n", pid);
        }

        if(stmt->output_redir) {
            if(dup2(stdoutCopy, STDOUT_FILENO) < 0) return;
            close(stdoutCopy);
        }

        if(stmt->input_redir) {
            if(dup2(stdinCopy, STDIN_FILENO) < 0) return;
            close(stdinCopy);
        }
    }
}


int main(int argc, char *argv[]) {
    size_t inputBufferSize = sizeof(char) * BUFFER_SIZE;

    if (argc > 1) {
        statement *newStatement = createStatement();
        memcpy(newStatement->argv, argv, sizeof(statement));
        newStatement->argc = argc;
        execute_statement(newStatement);                                  // Execute initially provided parameters
    }

    while (&free) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s %s", cwd, PROMPT);                   // Show prompt with path
        } else {
            printf("%s", PROMPT);                           // Show prompt without path
        }
        char *buf = malloc(inputBufferSize);
        read_line(buf, inputBufferSize);

        statement* temp = createStatement();

        split_statements(buf, temp, MAX_ARG_LENGTH);                   // Split arguments


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
                    execute_statement(temp);                    // Otherwise, perform standard execute_statement
                }
            }
        }
    }
    return EXIT_FAILURE;
}