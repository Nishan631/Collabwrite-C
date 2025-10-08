#ifndef EDITOPERATION_H
#define EDITOPERATION_H

#include <stdlib.h>


#define INSERT_OP 0
#define DELETE_OP 1
#define UPDATE_OP 2

typedef struct EditOperation {
    int type;               
    int position;           
    char *oldText;          
    char *newText;          
    struct EditOperation *next;
} EditOperation;

typedef struct {
    EditOperation *top;
} OperationStack;


OperationStack* createStack();
void pushOperation(OperationStack *stack, EditOperation *op);
EditOperation* popOperation(OperationStack *stack);
int isStackEmpty(OperationStack *stack);
void freeStack(OperationStack *stack);

#endif