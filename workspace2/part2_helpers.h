/* 046266 Compilation Methods, EE Faculty, Technion                        */
/* part2_helpers.h - Helper functions for project part 2 - API definitions */

#ifndef COMMON_H
#define COMMON_H
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char* type;
    char* value;
    struct node *sibling;
    struct node *child;
} ParserNode;

ParserNode *makeNode(char* type, char* value, ParserNode *child);

ParserNode *concatList(ParserNode *listHead, ParserNode *newItem);

void dumpParseTree(void);

  /* DATA STRUCTURES LIST */
#define YYSTYPE ParserNode*	// Tell Bison to use pointer to ParseNode as the stack type

#endif //COMMON_H
