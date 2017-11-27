//
// Created by Matthew Brown on 26/11/2017.
//

#ifndef CONSOLE_SHELLUTIL_H
#define CONSOLE_SHELLUTIL_H

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
    char terminator;
    struct statement *next;
} statement;

statement *createStatement();

void addToArgV(statement *statement, char *text);

char isCommandBreak(char *token);

char *read_line(char *buf, size_t sz);

int fpipe(FILE **readable, FILE **writable);

#endif //CONSOLE_SHELLUTIL_H
