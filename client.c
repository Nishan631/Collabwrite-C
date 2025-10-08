#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "network.h"

int sockfd;

void *recv_thread(void *arg) {
    char buf[BUFSIZE*2];
    ssize_t n;
    while ((n = recv(sockfd, buf, sizeof(buf)-1, 0)) > 0) {
        buf[n] = '\0';
        printf("[SERVER] %s", buf);
        fflush(stdout);
    }
    printf("Disconnected from server.\n");
    exit(0);
    return NULL;
}

int main(int argc, char **argv) {
    const char *server_ip = "127.0.0.1";
    if (argc >= 2) server_ip = argv[1];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); return 1; }

    struct sockaddr_in serv = {0};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(PORT);
    inet_pton(AF_INET, server_ip, &serv.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&serv, sizeof(serv)) < 0) {
        perror("connect"); return 1;
    }
    printf("Connected to server %s:%d\n", server_ip, PORT);

    pthread_t tid;
    pthread_create(&tid, NULL, recv_thread, NULL);

    char line[BUFSIZE];
    printf("Commands:\n");
    printf("INS <pos> <text>      - insert line at pos (0-based)\n");
    printf("DEL <pos>             - delete line at pos\n");
    printf("UPD <pos> <text>      - update line at pos\n");
    printf("UNDO / REDO / SNAP / GET / PRINT / QUIT\n");

    while (1) {
        printf(">> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        if (strncmp(line, "QUIT", 4) == 0) {
            close(sockfd);
            break;
        }
        send(sockfd, line, strlen(line), 0);
    }
    return 0;
}