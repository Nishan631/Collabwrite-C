#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LineNode {
    char *line;
    struct LineNode *prev;
    struct LineNode *next;
} LineNode;

typedef struct OperationStack OperationStack; /* forward declaration */

typedef struct {
    LineNode *head;
    LineNode *tail;
    int line_count;

    OperationStack *undoStack;
    OperationStack *redoStack;
} TextBuffer;

/* creation & destruction */
TextBuffer* createBuffer();
void freeBuffer(TextBuffer *buffer);

/* editing (these record operations on undo stack) */
void insertLine(TextBuffer *buffer, int position, const char *text);
void deleteLine(TextBuffer *buffer, int position);
void updateLine(TextBuffer *buffer, int position, const char *newText);

/* internal editing helpers that DO NOT record operations (used by undo/redo) */
void insertLine_no_record(TextBuffer *buffer, int position, const char *text);
void deleteLine_no_record(TextBuffer *buffer, int position);
void updateLine_no_record(TextBuffer *buffer, int position, const char *newText);

/* utility */
void printBuffer(TextBuffer *buffer);
char* buffer_to_string(TextBuffer *buffer); /* caller must free */
int valid_position(TextBuffer *buffer, int position);

/* undo/redo wrappers (operate using the stacks) */
void undo(TextBuffer *buffer);
void redo(TextBuffer *buffer);

#endif