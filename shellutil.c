//
// Created by Matthew Brown on 26/11/2017.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "shellutil.h"


/**
 * Create empty statement node
 * @return pointer to empty statement
 */
statement* createStatement(){
    statement *temp;
    temp = malloc(sizeof(statement));
    temp->next = NULL;
    return temp;
}

/**
 * Add to statement argv
 * @param c position in argv
 * @param statement current statement
 * @param text text to add
 */
void addToArgV(int* c, statement* statement, char * text) {
    statement->argv[*c] = malloc(sizeof(char*));
    statement->output_redir = NULL;
    strcpy(statement->argv[*c], text);
    (*c)++;
}

/**
 * Check if current characters is end of command list
 * @param token containing break point
 * @return token string broken on
 */
char isCommandBreak(char* token) {
    if(token[0] == ';' || token[strlen(token)-1] == ';') {
        return ';';
    } else if (token[0] == '&' || token[strlen(token)-1] == '&') {
        return '&';
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