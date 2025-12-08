#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../shared/socket.h"
#include "../shared/protocol.h"
#include "menus/patient_menu.h"
#include "menus/doctor_menu.h"
#include "menus/admin_menu.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
    if (argc < 3) { printf("Usage: %s <server_ip> <server_port>\n", argv[0]); return 1; }
    const char *host = argv[1]; int port = atoi(argv[2]);

    int sock = create_client_socket(host, port);
    if (sock < 0) { printf("Connect failed\n"); return 1; }

    char buf[MAX_LINE];
    if (recv_line(sock, buf, sizeof(buf)) > 0) printf("[SERVER] %s\n", buf);

    char username[128] = {0};
    char role[32] = {0};

    while (1) {
        printf("\n=== MAIN MENU ===\n1) Register\n2) Login\n3) Quit\nChoice: ");
        int c; if (scanf("%d",&c)!=1) { while(getchar()!='\n'); continue; } getchar();
        if (c == 3) { send_line(sock, "QUIT"); break; }
        if (c == 1) {
            char u[64], p[64], name[128];
            printf("Username: "); scanf("%63s", u); getchar();
            printf("Password: "); scanf("%63s", p); getchar();
            printf("Full name (no pipe char): "); fgets(name,sizeof(name),stdin); name[strcspn(name,"\n")] = 0;
            char cmd[512]; snprintf(cmd,sizeof(cmd),"REGISTER|%s|%s|%s|%s", u,p,"patient",name);
            send_line(sock, cmd);
            if (recv_line(sock, buf, sizeof(buf)) <= 0) break;
            printf("[SERVER] %s\n", buf);
        } else if (c == 2) {
            char u[64], p[64];
            printf("Username: "); scanf("%63s", u); getchar();
            printf("Password: "); scanf("%63s", p); getchar();
            char cmd[256]; snprintf(cmd,sizeof(cmd),"LOGIN|%s|%s", u,p);
            send_line(sock, cmd);
            if (recv_line(sock, buf, sizeof(buf)) <= 0) break;
            printf("[SERVER] %s\n", buf);
            if (strncmp(buf,"OK|LOGIN|",9)==0) {
                // parse role
                char *roleptr = buf + 9;
                strncpy(role, roleptr, sizeof(role)-1);
                strncpy(username, u, sizeof(username)-1);
                printf("Logged in as %s with role %s\n", username, role);
                if (strcmp(role,"patient")==0) patient_menu(sock, username);
                else if (strcmp(role,"doctor")==0) doctor_menu(sock, username);
                else if (strcmp(role,"admin")==0) admin_menu(sock, username);
                else printf("Unknown role: %s\n", role);
            }
        }
    }

    close(sock);
    return 0;
}
