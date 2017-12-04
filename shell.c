#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"

#include <sys/types.h>
#include <sys/wait.h>

#pragma clang diagnostic pop



/*
 * By 6602
 * All questions answered
 */

#define BUFFER_SIZE 1000
#define MAX_ARGS 40
#define MAX_ARG_LENGTH 100
#define PROGRAM_NAME "mash"
#define TOKEN_DELIMETER " \t\r\n\a"
#define PROMPT "\e[1;31mMASH >\033[0m "

typedef struct statement {
    char *argv[MAX_ARGS];
    int argc;
    FILE *input_redir;
    FILE *output_redir;
    FILE *output_err_redir;
    char terminator;
    struct statement *next;
} statement;


/**
 * Create empty statement node
 * @return pointer to empty statement
 */
statement *createStatement() {
    statement *temp;
    temp = malloc(sizeof(statement));
    temp->next = NULL;
    temp->input_redir = NULL;
    temp->output_redir = NULL;
    temp->output_err_redir = NULL;
    temp->argc = 0;
    temp->terminator = 0;
    return temp;
}

/**
 * Add to statement argv
 * @param c position in argv
 * @param statement current statement
 * @param text text to add
 */
void addToArgV(statement *statement, char *text) {
    statement->argv[statement->argc] = malloc(sizeof(char *));
    strcpy(statement->argv[statement->argc], text);
    (statement->argc)++;
}

/**
 * Check if current characters is end of command list
 * @param token containing break point
 * @return token the value of the character broken on
 */
char isCommandBreak(char *token) {
    if (token[0] == ';' || token[strlen(token) - 1] == ';') {
        return ';';
    } else if (token[0] == '&' || token[strlen(token) - 1] == '&') {
        return '&';
    } else if (token[0] == '|' || token[strlen(token) - 1] == '|') {
        return '|';
    } else {
        return 0;
    }
}

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
 * Creates a new pipe in memory and assign two file handlers to read and write to it.
 * @param readable a pointer to the readable end of the pipe
 * @param writable a pointer to the writeable end of the pipe
 * @return the status (always 0)
 */
int fpipe(FILE **readable, FILE **writable) {
    int fd[2];
    pipe(fd);
    *readable = fdopen(fd[0], "r");
    *writable = fdopen(fd[1], "w");
    return 0;
}


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
 * @param statement a pointer to the output statement array
 * @param max the maximum length of each parameter
 */
void split_statements(char *buf, statement *statement, size_t max) {
    char *token = strtok(buf, TOKEN_DELIMETER);               // Break on any new line, tab, space, or return

    while (token != NULL) {
        char commandBroken = isCommandBreak(token);
        if (strlen(token) > 0 && commandBroken) {

            if (commandBroken == ';') {
                if (token[0] == ';') {
                    statement->argv[statement->argc] = 0;
                } else {
                    token[strlen(token) - 1] = 0;
                    addToArgV(statement, token);
                }
            }

            if (strlen(token) != 1 && commandBroken == '&') {
                token[strlen(token) - 1] = 0;
                addToArgV(statement, token);
            }

            statement->argv[statement->argc] = 0;
            statement->terminator = commandBroken;


            statement->next = createStatement();             // Creates and initialises new statement

            if (commandBroken == '|') {

                FILE *readable;
                FILE *writable;

                fpipe(&readable, &writable);                 // Creates new pipe to handle | case

                statement->output_redir = writable;
                statement = statement->next;
                statement->input_redir = readable;

            } else {
                statement = statement->next;
            }

        } else if (strcmp(token, "\"\"") == 0) {             // Ignore `""`
            addToArgV(statement, "");
        } else if (strlen(token) == 1 && token[0] == '>') {

            token = strtok(NULL, TOKEN_DELIMETER);
            statement->output_redir = fopen(token, "w");

        } else if (strlen(token) == 2 && token[0] == '>' && token[1] == '>') {

            token = strtok(NULL, TOKEN_DELIMETER);
            statement->output_redir = fopen(token, "a");    // Open (and create) file in append mode

        } else if (strlen(token) == 2 && token[0] == '2' && token[1] == '>') {

            token = strtok(NULL, TOKEN_DELIMETER);
            statement->output_err_redir = fopen(token, "w");// Open (and create) file in append mode

        } else if (strlen(token) == 1 && token[0] == '<') {

            token = strtok(NULL, TOKEN_DELIMETER);
            statement->input_redir = fopen(token, "r");

        } else if (token[0] == '\"') {                      // Case: on an opening speech mark
            char *comb = malloc(max);                       // Allocate a space for the concatenated strings
            token++;                                        // Ignore the opening mark

            if (strchr(token, '"')) {
                strcpy(comb, token);
            } else {
                while (token != NULL && !strchr(token, '"')) {
                    strcat(comb, token);                     // Add the string to 'comb'
                    token = strtok(NULL, TOKEN_DELIMETER);   // Move to next token
                    if (token != NULL) {
                        strcat(comb, " ");                   // Add space to parameters
                        if (strchr(token, '"')) {
                            strcat(comb, token);             // Add the string to 'comb'
                        }
                    }

                }
            }
            // HANDLE "a edge case
            char *atSpeechMark = strchr(comb, '"');
            if (atSpeechMark == NULL) {
                comb[strlen(comb)] = '\0';
                addToArgV(statement, comb);
            } else {
                *atSpeechMark = 0;                          // Convert speech mark to null character
                statement->argv[statement->argc] = malloc(sizeof(char *));
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


    statement->argv[statement->argc] = 0;                   // Null terminate final statements, final array item
    statement->next = NULL;                                 // Sets the next statement to null (indicating the final statement)
    free(token);                                            // Free memory used by the string
}


/**
 * Executes the given array of commands in a subprocess
 * @param stmt the statments representing a command
 */
void execute_statement(statement *stmt) {
    char **split = stmt->argv;
    int stdoutCopy = 0;
    int stderrCopy = 0;
    int stdinCopy = 0;

    if (stmt->output_redir) {
        stdoutCopy = dup(STDOUT_FILENO);
        int outputFile = fileno(stmt->output_redir);
        if (dup2(outputFile, STDOUT_FILENO) < 0) return;
        close(outputFile);
    }

    if (stmt->output_err_redir) {
        stderrCopy = dup(STDERR_FILENO);
        int outputFile = fileno(stmt->output_err_redir);
        if (dup2(outputFile, STDERR_FILENO) < 0) return;
        close(outputFile);
    }

    if (stmt->input_redir) {
        stdinCopy = dup(STDIN_FILENO);
        int inputFile = fileno(stmt->input_redir);
        if (dup2(inputFile, STDIN_FILENO) < 0) return;
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
        waitpid(pid, NULL,
                (stmt->terminator == '&') ? WNOHANG : 0);                               // Wait for subprocess to finish
        if (stmt->terminator == '&') {
            printf("[%d]\n", pid);
        }

        if (stmt->output_redir) {
            if (dup2(stdoutCopy, STDOUT_FILENO) < 0) return;
            close(stdoutCopy);
        }

        if (stmt->output_err_redir) {
            if (dup2(stderrCopy, STDERR_FILENO) < 0) return;
            close(stderrCopy);
        }

        if (stmt->input_redir) {
            if (dup2(stdinCopy, STDIN_FILENO) < 0) return;
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
        execute_statement(newStatement);
        free(newStatement);
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

        statement *temp = createStatement();

        split_statements(buf, temp, MAX_ARG_LENGTH);                   // Split arguments


        while (temp != NULL) {
            if (temp->argv[0] != NULL) {
                if (strncmp(temp->argv[0], "exit", 4) == 0) {         // On `exit`, exit
                    printf("Exiting...\n");
                    return EXIT_SUCCESS;
                } else if (strncmp(temp->argv[0], "cd", 2) == 0) {    // On `cd`, change directory
                    if (temp->argv[1] != NULL) {
                        if (chdir(temp->argv[1]) != 0) {
                            perror(PROGRAM_NAME);
                        }
                    } else {
                        chdir(getpwuid(getuid())->pw_dir);
                    }
                } else if (strncmp(temp->argv[0], "help", 4) == 0) {  // On `help`, show help
                    printHelp();
                } else {
                    execute_statement(temp);                    // Otherwise, perform standard execute_statement
                }
            }
            struct statement *oldtemp = temp;
            temp = temp->next;
            free(oldtemp);
        }

    }
    return EXIT_FAILURE;
}