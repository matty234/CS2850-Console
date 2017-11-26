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