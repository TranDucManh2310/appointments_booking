#include "doctor_menu.h"
#include "../../shared/socket.h"
#include <stdio.h>
#include <string.h>

void doctor_menu(int sock, const char *username) {
    int choice;
    char buf[MAX_LINE];
    while (1) {
        printf("\n=== DOCTOR MENU ===\n");
        printf("1. View bookings\n2. Add slot (simple)\n3. Update appointment status\n0. Back\nChoice: ");
        if (scanf("%d",&choice)!=1) { while(getchar()!='\n'); continue; }
        getchar();
        if (choice==0) break;
        if (choice==1) {
            int did; char date[32];
            printf("Doctor ID: "); scanf("%d",&did); getchar();
            printf("Date (YYYY-MM-DD): "); scanf("%31s", date); getchar();
            char cmd[128]; snprintf(cmd,sizeof(cmd),"DOCTOR_VIEW_BOOKINGS|%d|%s", did, date);
            send_line(sock, cmd);
            while (1) {
                if (recv_line(sock, buf, sizeof(buf)) <= 0) return;
                printf("[SERVER] %s\n", buf);
                if (strncmp(buf, "OK|BOOKINGS_END",15)==0 || strncmp(buf,"OK|NO_BOOKINGS",14)==0 || strncmp(buf,"ERR|",4)==0) break;
            }
        } else if (choice==2) {
            int did, slotm; char date[32], start[8], end[8];
            printf("Doctor ID: "); scanf("%d",&did); getchar();
            printf("Date: "); scanf("%31s", date); getchar();
            printf("Start HH:MM: "); scanf("%7s", start); getchar();
            printf("End HH:MM: "); scanf("%7s", end); getchar();
            printf("Slot minutes: "); scanf("%d", &slotm); getchar();
            char cmd[256]; snprintf(cmd,sizeof(cmd),"DOCTOR_ADD_SLOT|%d|%s|%s|%s|%d", did, date, start, end, slotm);
            send_line(sock, cmd);
            if (recv_line(sock, buf, sizeof(buf)) <=0) return;
            printf("[SERVER] %s\n", buf);
        } else if (choice==3) {
            int apid; char status[32];
            printf("Appointment ID: "); scanf("%d",&apid); getchar();
            printf("Status (COMPLETED/CANCELLED): "); scanf("%31s", status); getchar();
            char cmd[128]; snprintf(cmd,sizeof(cmd),"DOCTOR_UPDATE_STATUS|%d|%s", apid, status);
            send_line(sock, cmd);
            if (recv_line(sock, buf, sizeof(buf)) <= 0) return;
            printf("[SERVER] %s\n", buf);
        }
    }
}
