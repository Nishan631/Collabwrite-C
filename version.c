#include "version.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void vtree_init(VersionTree *vt) {
    vt->root = NULL;
    vt->next_id = 1;
}

VersionNode* vtree_snapshot(VersionTree *vt, TextBuffer *tb, VersionNode *parent) {
    VersionNode *n = (VersionNode*)malloc(sizeof(VersionNode));
    n->id = vt->next_id++;
    char *snap = buffer_to_string(tb); 
    n->snapshot = snap ? snap : strdup("");
    n->parent = parent;
    n->first_child = NULL;
    n->next_sibling = NULL;

    if (!vt->root) vt->root = n;
    else if (parent) {
        if (!parent->first_child) parent->first_child = n;
        else {
            VersionNode *s = parent->first_child;
            while (s->next_sibling) s = s->next_sibling;
            s->next_sibling = n;
        }
    } else {
        VersionNode *s = vt->root;
        while (s->next_sibling) s = s->next_sibling;
        s->next_sibling = n;
    }
    return n;
}

void vtree_free(VersionNode *node) {
    if (!node) return;
    vtree_free(node->first_child);
    vtree_free(node->next_sibling);
    if (node->snapshot) free(node->snapshot);
    free(node);
}

void print_versions(VersionNode *node, int depth) {
    if (!node) return;
    for (int i=0;i<depth;i++) printf("  ");
    printf("v%d: %.40s%s\n", node->id, node->snapshot, strlen(node->snapshot) > 40 ? "..." : "");
    print_versions(node->first_child, depth+1);
    print_versions(node->next_sibling, depth);
}

VersionNode* vtree_find(VersionNode *node, int id) {
    if (!node) return NULL;
    if (node->id == id) return node;
    VersionNode *r = vtree_find(node->first_child, id);
    if (r) return r;
    return vtree_find(node->next_sibling, id);
}

int vtree_restore(VersionTree *vt, TextBuffer *tb, int id) {
    VersionNode *n = vtree_find(vt->root, id);
    if (!n) return -1;
    tb_free(tb);
    tb_init(tb);
    if (n->snapshot && n->snapshot[0] != '\0') {
        tb_insert(tb, 0, n->snapshot);
    }
    return 0;
}