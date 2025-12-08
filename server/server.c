#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "handlers/patient_handler.h"
#include "handlers/doctor_handler.h"
#include "handlers/admin_handler.h"
#include "db/db.h"
#include "../shared/socket.h"
#include "../shared/protocol.h"
#include <unistd.h>
#include <arpa/inet.h>

#define DATA_DIR "data"

typedef struct {
    int sock;
    char data_dir[256];
} client_ctx_t;

void *client_thread(void *arg) {
    client_ctx_t *ctx = (client_ctx_t*)arg;
    int sock = ctx->sock;
    char data_dir[256]; strncpy(data_dir, ctx->data_dir, sizeof(data_dir)-1);
    free(ctx);

    char session_user[128] = {0};
    char session_role[32] = {0};

    send_line(sock, "OK|WELCOME|Clinic Socket Server");

    char line[MAX_LINE];
    while (1) {
        ssize_t r = recv_line(sock, line, sizeof(line));
        if (r <= 0) break;
        trim_newline(line);
        if (strlen(line) == 0) continue;

        // route command by prefix
        char copy[MAX_LINE]; strncpy(copy, line, sizeof(copy));
        char *cmd = strtok(copy, "|");
        if (!cmd) { send_line(sock, "ERR|BAD_REQ"); continue; }

        // common commands handled by patient handler (REGISTER, LOGIN, LIST_DOCTORS, LIST_SLOTS, BOOK, VIEW_MY_APPTS, CANCEL)
        if (strcmp(cmd, "REGISTER") == 0 || strcmp(cmd, "LOGIN") == 0 ||
            strcmp(cmd, "LIST_DOCTORS") == 0 || strcmp(cmd, "LIST_SLOTS") == 0 ||
            strcmp(cmd, "BOOK") == 0 || strcmp(cmd, "VIEW_MY_APPTS") == 0 ||
            strcmp(cmd, "CANCEL") == 0) {
            handle_patient(sock, line, data_dir, session_user, session_role);
        }
        else if (strcmp(cmd, "DOCTOR_VIEW_BOOKINGS")==0 || strcmp(cmd,"DOCTOR_ADD_SLOT")==0 || strcmp(cmd,"DOCTOR_UPDATE_STATUS")==0) {
            handle_doctor(sock, line, data_dir, session_user, session_role);
        }
        else if (strcmp(cmd,"ADMIN_ADD_DOCTOR")==0 || strcmp(cmd,"ADMIN_LIST_USERS")==0 || strcmp(cmd,"ADMIN_LIST_DOCTORS")==0 ||
                 strcmp(cmd,"ADMIN_LIST_ALL_BOOKINGS")==0 || strcmp(cmd,"ADMIN_DELETE_USER")==0) {
            // allow only admin role for admin commands
            if (strcmp(session_role,"admin")!=0 && strncmp(cmd,"ADMIN_",6)==0) {
                send_line(sock,"ERR|PERMISSION_DENIED");
            } else {
                handle_admin(sock, line, data_dir, session_user, session_role);
            }
        }
        else if (strcmp(cmd,"QUIT")==0) {
            send_line(sock,"OK|BYE");
            break;
        } else {
            send_line(sock,"ERR|UNKNOWN_CMD");
        }
    }

    close(sock);
    return NULL;
}

int main(int argc, char *argv[]) {
    int port = 9000;
    if (argc >= 2) port = atoi(argv[1]);
    db_init(DATA_DIR);
    int sfd = create_server_socket(port);
    if (sfd < 0) return 1;
    printf("Server listening on 0.0.0.0:%d\n", port);

    while (1) {
        struct sockaddr_in cli; socklen_t len = sizeof(cli);
        int c = accept(sfd, (struct sockaddr*)&cli, &len);
        if (c < 0) { perror("accept"); continue; }
        printf("Connection from %s:%d\n", inet_ntoa(cli.sin_addr), ntohs(cli.sin_port));
        client_ctx_t *ctx = malloc(sizeof(client_ctx_t));
        ctx->sock = c;
        strncpy(ctx->data_dir, DATA_DIR, sizeof(ctx->data_dir)-1);
        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, ctx);
        pthread_detach(tid);
    }
    close(sfd);
    return 0;
}
