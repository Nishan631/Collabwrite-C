#include "text_buffer.h"
#include "editoperation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Helper: create a line node */
static LineNode* createLineNode(const char *text) {
    LineNode *newNode = (LineNode*)malloc(sizeof(LineNode));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed for LineNode\n");
        exit(1);
    }
    newNode->line = strdup(text ? text : "");
    newNode->prev = newNode->next = NULL;
    return newNode;
}

TextBuffer* createBuffer() {
    TextBuffer *buffer = (TextBuffer*)malloc(sizeof(TextBuffer));
    if (!buffer) { fprintf(stderr, "Memory allocation failed for TextBuffer\n"); exit(1); }
    buffer->head = buffer->tail = NULL;
    buffer->line_count = 0;
    buffer->undoStack = createStack();
    buffer->redoStack = createStack();
    return buffer;
}

/* internal helpers to traverse to a node; returns pointer to node at position,
   or NULL if position == line_count (insertion at end) or out-of-range */
static LineNode* node_at(TextBuffer *buffer, int position) {
    if (position < 0 || position > buffer->line_count) return NULL;
    LineNode *cur = buffer->head;
    int idx = 0;
    while (cur && idx < position) {
        cur = cur->next;
        idx++;
    }
    return cur; /* if position == line_count, cur == NULL (end) */
}

/* No-record versions: used by undo/redo to avoid pushing operations onto the stacks */
void insertLine_no_record(TextBuffer *buffer, int position, const char *text) {
    if (position < 0 || position > buffer->line_count) return;
    LineNode *newNode = createLineNode(text);
    if (buffer->line_count == 0) {
        buffer->head = buffer->tail = newNode;
    } else if (position == 0) {
        newNode->next = buffer->head;
        buffer->head->prev = newNode;
        buffer->head = newNode;
    } else if (position == buffer->line_count) {
        buffer->tail->next = newNode;
        newNode->prev = buffer->tail;
        buffer->tail = newNode;
    } else {
        LineNode *cur = node_at(buffer, position);
        LineNode *prev = cur->prev;
        prev->next = newNode;
        newNode->prev = prev;
        newNode->next = cur;
        cur->prev = newNode;
    }
    buffer->line_count++;
}

void deleteLine_no_record(TextBuffer *buffer, int position) {
    if (position < 0 || position >= buffer->line_count) return;
    LineNode *cur = node_at(buffer, position);
    if (!cur) return;
    if (cur->prev) cur->prev->next = cur->next;
    else buffer->head = cur->next;
    if (cur->next) cur->next->prev = cur->prev;
    else buffer->tail = cur->prev;
    free(cur->line);
    free(cur);
    buffer->line_count--;
}

void updateLine_no_record(TextBuffer *buffer, int position, const char *newText) {
    if (position < 0 || position >= buffer->line_count) return;
    LineNode *cur = node_at(buffer, position);
    if (!cur) return;
    free(cur->line);
    cur->line = strdup(newText);
}

/* Public functions that RECORD operations on undo stack and clear redo stack */
void insertLine(TextBuffer *buffer, int position, const char *text) {
    if (position < 0 || position > buffer->line_count) return;
    /* record operation */
    EditOperation *op = (EditOperation*)malloc(sizeof(EditOperation));
    op->type = INSERT_OP;
    op->position = position;
    op->oldText = NULL;
    op->newText = strdup(text ? text : "");
    op->next = NULL;
    pushOperation(buffer->undoStack, op);
    /* clear redo */
    freeStack(buffer->redoStack);
    buffer->redoStack = createStack();
    /* apply */
    insertLine_no_record(buffer, position, text);
}

void deleteLine(TextBuffer *buffer, int position) {
    if (position < 0 || position >= buffer->line_count) return;
    LineNode *cur = node_at(buffer, position);
    if (!cur) return;
    /* record operation (capture old text) */
    EditOperation *op = (EditOperation*)malloc(sizeof(EditOperation));
    op->type = DELETE_OP;
    op->position = position;
    op->oldText = strdup(cur->line);
    op->newText = NULL;
    op->next = NULL;
    pushOperation(buffer->undoStack, op);
    /* clear redo */
    freeStack(buffer->redoStack);
    buffer->redoStack = createStack();
    /* apply delete */
    deleteLine_no_record(buffer, position);
}

void updateLine(TextBuffer *buffer, int position, const char *newText) {
    if (position < 0 || position >= buffer->line_count) return;
    LineNode *cur = node_at(buffer, position);
    if (!cur) return;
    /* record operation (capture old and new text) */
    EditOperation *op = (EditOperation*)malloc(sizeof(EditOperation));
    op->type = UPDATE_OP;
    op->position = position;
    op->oldText = strdup(cur->line);
    op->newText = strdup(newText);
    op->next = NULL;
    pushOperation(buffer->undoStack, op);
    /* clear redo */
    freeStack(buffer->redoStack);
    buffer->redoStack = createStack();
    /* apply update */
    updateLine_no_record(buffer, position, newText);
}

/* Print buffer lines with numbers */
void printBuffer(TextBuffer *buffer) {
    LineNode *curr = buffer->head;
    int lineNum = 0;
    while (curr) {
        printf("%d: %s\n", lineNum, curr->line);
        curr = curr->next;
        lineNum++;
    }
}

/* Return full document as a single dynamically allocated string (caller must free) */
char* buffer_to_string(TextBuffer *buffer) {
    /* estimate required size */
    size_t total = 0;
    LineNode *cur = buffer->head;
    while (cur) {
        total += strlen(cur->line) + 1; /* +1 for newline */
        cur = cur->next;
    }
    char *s = (char*)malloc(total + 1);
    if (!s) return NULL;
    s[0] = '\0';
    cur = buffer->head;
    while (cur) {
        strcat(s, cur->line);
        strcat(s, "\n");
        cur = cur->next;
    }
    return s;
}

int valid_position(TextBuffer *buffer, int position) {
    return (position >= 0 && position <= buffer->line_count);
}

/* Undo / Redo implementations that use no-record internal functions */

void undo(TextBuffer *buffer) {
    if (isStackEmpty(buffer->undoStack)) {
        printf("Nothing to undo.\n");
        return;
    }
    EditOperation *op = popOperation(buffer->undoStack);
    if (!op) return;

    if (op->type == INSERT_OP) {
        /* undo insert => delete the inserted line */
        deleteLine_no_record(buffer, op->position);
    } else if (op->type == DELETE_OP) {
        /* undo delete => insert oldText back */
        insertLine_no_record(buffer, op->position, op->oldText);
    } else if (op->type == UPDATE_OP) {
        /* undo update => restore oldText */
        updateLine_no_record(buffer, op->position, op->oldText);
    }

    /* push op onto redo stack (same op object) */
    pushOperation(buffer->redoStack, op);
}

void redo(TextBuffer *buffer) {
    if (isStackEmpty(buffer->redoStack)) {
        printf("Nothing to redo.\n");
        return;
    }
    EditOperation *op = popOperation(buffer->redoStack);
    if (!op) return;

    if (op->type == INSERT_OP) {
        insertLine_no_record(buffer, op->position, op->newText);
    } else if (op->type == DELETE_OP) {
        deleteLine_no_record(buffer, op->position);
    } else if (op->type == UPDATE_OP) {
        updateLine_no_record(buffer, op->position, op->newText);
    }

    pushOperation(buffer->undoStack, op);
}

void freeBuffer(TextBuffer *buffer) {
    if (!buffer) return;
    LineNode *curr = buffer->head;
    while (curr) {
        LineNode *temp = curr;
        curr = curr->next;
        free(temp->line);
        free(temp);
    }
    /* free stacks */
    freeStack(buffer->undoStack);
    freeStack(buffer->redoStack);
    free(buffer);
}