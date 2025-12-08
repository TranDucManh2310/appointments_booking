#include "patient_menu.h"
#include "../../shared/socket.h"
#include <stdio.h>
#include <string.h>

void patient_menu(int sock, const char *username) {
    int choice;
    char buf[MAX_LINE];
    while (1) {
        printf("\n=== PATIENT MENU ===\n");
        printf("1. List doctors\n2. List slots\n3. Book appointment\n4. View my appointments\n5. Cancel appointment\n0. Back\nChoice: ");
        if (scanf("%d", &choice)!=1) { while(getchar()!='\n'); continue; }
        getchar();
        if (choice == 0) break;
        if (choice == 1) {
            send_line(sock, "LIST_DOCTORS");
            while (1) {
                if (recv_line(sock, buf, sizeof(buf)) <= 0) return;
                printf("[SERVER] %s\n", buf);
                if (strncmp(buf,"OK|DOCTORS_END",14)==0 || strncmp(buf,"ERR|",4)==0) break;
            }
        } else if (choice == 2) {
            int did; char date[32];
            printf("Doctor ID: "); scanf("%d",&did); getchar();
            printf("Date (YYYY-MM-DD): "); scanf("%31s", date); getchar();
            char cmd[128]; snprintf(cmd,sizeof(cmd),"LIST_SLOTS|%d|%s", did, date);
            send_line(sock, cmd);
            while (1) {
                if (recv_line(sock, buf, sizeof(buf)) <= 0) return;
                printf("[SERVER] %s\n", buf);
                if (strncmp(buf,"OK|SLOTS_END",12)==0 || strncmp(buf,"OK|NO_BOOKINGS",14)==0 || strncmp(buf,"ERR|",4)==0) break;
            }
        } else if (choice == 3) {
            int did; char date[32], time[16];
            printf("Doctor ID: "); scanf("%d",&did); getchar();
            printf("Date (YYYY-MM-DD): "); scanf("%31s", date); getchar();
            printf("Time (HH:MM): "); scanf("%15s", time); getchar();
            char cmd[256]; snprintf(cmd,sizeof(cmd),"BOOK|%d|%s|%s", did, date, time);
            send_line(sock, cmd);
            if (recv_line(sock, buf, sizeof(buf)) <= 0) return;
            printf("[SERVER] %s\n", buf);
        } else if (choice == 4) {
            send_line(sock, "VIEW_MY_APPTS");
            while (1) {
                if (recv_line(sock, buf, sizeof(buf)) <= 0) return;
                printf("[SERVER] %s\n", buf);
                if (strncmp(buf,"OK|MY_APPTS_END",15)==0 || strncmp(buf,"OK|NO_APPTS",11)==0 || strncmp(buf,"ERR|",4)==0) break;
            }
        } else if (choice == 5) {
            int apid; printf("Appointment ID to cancel: "); scanf("%d",&apid); getchar();
            char cmd[64]; snprintf(cmd,sizeof(cmd),"CANCEL|%d", apid);
            send_line(sock, cmd);
            if (recv_line(sock, buf, sizeof(buf)) <= 0) return;
            printf("[SERVER] %s\n", buf);
        }
    }
}
