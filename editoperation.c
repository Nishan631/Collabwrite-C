#include "editoperation.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

OperationStack* createStack() {
    OperationStack *stack = (OperationStack*)malloc(sizeof(OperationStack));
    if (!stack) {
        fprintf(stderr, "Memory allocation failed for OperationStack\n");
        exit(1);
    }
    stack->top = NULL;
    return stack;
}

void pushOperation(OperationStack *stack, EditOperation *op) {
    if (!stack || !op) return;
    op->next = stack->top;
    stack->top = op;
}

EditOperation* popOperation(OperationStack *stack) {
    if (!stack || !stack->top) return NULL;
    EditOperation *op = stack->top;
    stack->top = op->next;
    op->next = NULL;
    return op;
}

int isStackEmpty(OperationStack *stack) {
    return (!stack || stack->top == NULL);
}

void freeStack(OperationStack *stack) {
    if (!stack) return;
    while (!isStackEmpty(stack)) {
        EditOperation *op = popOperation(stack);
        if (op->oldText) free(op->oldText);
        if (op->newText) free(op->newText);
        free(op);
    }
    free(stack);
}