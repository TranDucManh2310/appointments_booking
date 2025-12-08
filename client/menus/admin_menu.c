#include "admin_menu.h"
#include "../../shared/socket.h"
#include <stdio.h>
#include <string.h>
#include "../../shared/constants.h"

void admin_menu(int sock, const char *username) {
    int choice; char buf[MAX_LINE];
    while (1) {
        printf("\n=== ADMIN MENU ===\n");
        printf("1. Add doctor account\n2. List users\n3. List doctors\n4. List all bookings\n5. Delete user\n0. Back\nChoice: ");
        if (scanf("%d",&choice)!=1) { while(getchar()!='\n'); continue; }
        getchar();
        if (choice==0) break;
        if (choice==1) {
            char user[64], pass[64], name[128], spec[128];
            printf("Doctor username: "); scanf("%63s", user); getchar();
            printf("Password: "); scanf("%63s", pass); getchar();
            printf("Full name: "); fgets(name,sizeof(name),stdin); name[strcspn(name,"\n")] = 0;
            printf("Specialty: "); fgets(spec,sizeof(spec),stdin); spec[strcspn(spec,"\n")] = 0;
            char cmd[512]; snprintf(cmd,sizeof(cmd),"ADMIN_ADD_DOCTOR_ACCOUNT|%s|%s|%s|%s", user, pass, name, spec);
            send_line(sock, cmd);
            if (recv_line(sock, buf, sizeof(buf)) <= 0) return;
            printf("[SERVER] %s\n", buf);
        } else if (choice==2) {
            send_line(sock,"ADMIN_LIST_USERS");
            while (1) {
                if (recv_line(sock, buf, sizeof(buf)) <=0) return;
                printf("[SERVER] %s\n", buf);
                if (strncmp(buf,"OK|USERS_END",12)==0 || strncmp(buf,"ERR|",4)==0) break;
            }
        } else if (choice==3) {
            send_line(sock,"ADMIN_LIST_DOCTORS");
            while (1) {
                if (recv_line(sock, buf, sizeof(buf)) <=0) return;
                printf("[SERVER] %s\n", buf);
                if (strncmp(buf,"OK|DOCTORS_END",14)==0 || strncmp(buf,"ERR|",4)==0) break;
            }
        } else if (choice==4) {
            send_line(sock,"ADMIN_LIST_ALL_BOOKINGS");
            while (1) {
                if (recv_line(sock, buf, sizeof(buf)) <=0) return;
                printf("[SERVER] %s\n", buf);
                if (strncmp(buf,"OK|BOOKINGS_END",15)==0 || strncmp(buf,"ERR|",4)==0) break;
            }
        } else if (choice==5) {
            char user[64]; printf("Username to delete: "); scanf("%63s", user); getchar();
            char cmd[128]; snprintf(cmd,sizeof(cmd),"ADMIN_DELETE_USER|%s", user);
            send_line(sock, cmd);
            if (recv_line(sock, buf, sizeof(buf)) <=0) return;
            printf("[SERVER] %s\n", buf);
        }
    }
}
