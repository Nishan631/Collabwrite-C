#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>

#include "text_buffer.h"
#include "version.h"
#include "network.h"
#include "editoperation.h"

TextBuffer *g_buffer;
VersionTree g_vtree;

typedef struct Client {
    int sock;
    struct Client *next;
} Client;

Client *clients = NULL;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buf_mutex = PTHREAD_MUTEX_INITIALIZER;

void add_client(int sock) {
    Client *c = (Client*)malloc(sizeof(Client));
    c->sock = sock;
    pthread_mutex_lock(&clients_mutex);
    c->next = clients;
    clients = c;
    pthread_mutex_unlock(&clients_mutex);
}
void remove_client(int sock) {
    pthread_mutex_lock(&clients_mutex);
    Client **pc = &clients;
    while (*pc) {
        if ((*pc)->sock == sock) {
            Client *tmp = *pc;
            *pc = tmp->next;
            free(tmp);
            break;
        }
        pc = &(*pc)->next;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void broadcast(const char *msg) {
    pthread_mutex_lock(&clients_mutex);
    Client *c = clients;
    while (c) {
        send(c->sock, msg, strlen(msg), 0);
        c = c->next;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void handle_command(char *line, int sock) {
    if (!line) return;
    size_t L = strlen(line);
    if (L && line[L-1] == '\n') line[L-1] = '\0';

    if (strncmp(line, "INS ", 4) == 0) {
        char *p = line + 4;
        int pos = (int)strtol(p, &p, 10);
        while (*p == ' ') p++;
        char *text = p;
        if (!valid_position(g_buffer, pos)) {
            char err[256];
            snprintf(err, sizeof(err), "ERR invalid position %d\n", pos);
            send(sock, err, strlen(err), 0);
            return;
        }
        pthread_mutex_lock(&buf_mutex);
        insertLine(g_buffer, pos, text);
        pthread_mutex_unlock(&buf_mutex);
        char msg[BUFSIZE];
        snprintf(msg, sizeof(msg), "APPLY INS %d %s\n", pos, text);
        broadcast(msg);
    } else if (strncmp(line, "DEL ", 4) == 0) {
        char *p = line + 4;
        int pos = (int)strtol(p, NULL, 10);
        if (!valid_position(g_buffer, pos) || pos >= g_buffer->line_count) {
            char err[256];
            snprintf(err, sizeof(err), "ERR invalid position %d\n", pos);
            send(sock, err, strlen(err), 0);
            return;
        }
        pthread_mutex_lock(&buf_mutex);
        deleteLine(g_buffer, pos);
        pthread_mutex_unlock(&buf_mutex);
        char msg[BUFSIZE];
        snprintf(msg, sizeof(msg), "APPLY DEL %d\n", pos);
        broadcast(msg);
    } else if (strncmp(line, "UPD ", 4) == 0) {
        char *p = line + 4;
        int pos = (int)strtol(p, &p, 10);
        while (*p == ' ') p++;
        char *text = p;
        if (!valid_position(g_buffer, pos) || pos >= g_buffer->line_count) {
            char err[256];
            snprintf(err, sizeof(err), "ERR invalid position %d\n", pos);
            send(sock, err, strlen(err), 0);
            return;
        }
        pthread_mutex_lock(&buf_mutex);
        updateLine(g_buffer, pos, text);
        pthread_mutex_unlock(&buf_mutex);
        char msg[BUFSIZE];
        snprintf(msg, sizeof(msg), "APPLY UPD %d %s\n", pos, text);
        broadcast(msg);
    } else if (strcmp(line, "UNDO") == 0) {
        pthread_mutex_lock(&buf_mutex);
        undo(g_buffer);
        pthread_mutex_unlock(&buf_mutex);
        broadcast("APPLY UNDO\n");
    } else if (strcmp(line, "REDO") == 0) {
        pthread_mutex_lock(&buf_mutex);
        redo(g_buffer);
        pthread_mutex_unlock(&buf_mutex);
        broadcast("APPLY REDO\n");
    } else if (strcmp(line, "SNAP") == 0) {
        pthread_mutex_lock(&buf_mutex);
        VersionNode *v = vtree_snapshot(&g_vtree, g_buffer, g_vtree.root);
        pthread_mutex_unlock(&buf_mutex);
        char msg[256];
        snprintf(msg, sizeof(msg), "SNAPSHOT v%d\n", v->id);
        broadcast(msg);
    } else if (strcmp(line, "GET") == 0) {
        pthread_mutex_lock(&buf_mutex);
        char *doc = buffer_to_string(g_buffer);
        pthread_mutex_unlock(&buf_mutex);
        if (!doc) doc = strdup("");
        char reply[BUFSIZE*2];
        snprintf(reply, sizeof(reply), "DOC %d\n%s", g_buffer->line_count, doc);
        send(sock, reply, strlen(reply), 0);
        free(doc);
    } else if (strcmp(line, "PRINT") == 0) {
        pthread_mutex_lock(&buf_mutex);
        printBuffer(g_buffer);
        pthread_mutex_unlock(&buf_mutex);
    } else {
        char err[256];
        snprintf(err, sizeof(err), "ERR unknown command\n");
        send(sock, err, strlen(err), 0);
    }
}

void *client_thread(void *arg) {
    int sock = *(int*)arg;
    free(arg);
    add_client(sock);
    char buf[BUFSIZE];
    ssize_t n;
    while ((n = recv(sock, buf, sizeof(buf)-1, 0)) > 0) {
        buf[n] = '\0';
        char *saveptr = NULL;
        char *line = strtok_r(buf, "\n", &saveptr);
        while (line) {
            char cmdline[BUFSIZE];
            snprintf(cmdline, sizeof(cmdline), "%s\n", line);
            handle_command(cmdline, sock);
            line = strtok_r(NULL, "\n", &saveptr);
        }
    }
    remove_client(sock);
    close(sock);
    return NULL;
}

int main() {
    g_buffer = createBuffer();
    vtree_init(&g_vtree);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); exit(1); }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(server_fd, 16) < 0) { perror("listen"); exit(1); }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int csock = accept(server_fd, (struct sockaddr*)&caddr, &clen);
        if (csock < 0) { perror("accept"); continue; }
        printf("Client connected: %s:%d\n", inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
        pthread_t tid;
        int *p = malloc(sizeof(int));
        *p = csock;
        pthread_create(&tid, NULL, client_thread, p);
        pthread_detach(tid);
    }

    freeBuffer(g_buffer);
    if (g_vtree.root) vtree_free(g_vtree.root);
    close(server_fd);
    return 0;
}