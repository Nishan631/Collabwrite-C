#ifndef VERSION_H
#define VERSION_H

#include "text_buffer.h"

typedef struct VersionNode {
    char *snapshot; 
    int id;
    struct VersionNode *parent;
    struct VersionNode *first_child;
    struct VersionNode *next_sibling;
} VersionNode;

typedef struct {
    VersionNode *root;
    int next_id;
} VersionTree;

void vtree_init(VersionTree *vt);
VersionNode* vtree_snapshot(VersionTree *vt, TextBuffer *tb, VersionNode *parent);
void vtree_free(VersionNode *node);
void print_versions(VersionNode *node, int depth);
VersionNode* vtree_find(VersionNode *node, int id);
int vtree_restore(VersionTree *vt, TextBuffer *tb, int id);

#endif