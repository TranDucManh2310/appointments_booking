#include "admin_handler.h"
#include "../db/db.h"
#include "../../shared/socket.h"
#include <stdio.h>
#include <string.h>
#include "../../shared/constants.h"

void handle_admin(int client_sock, const char *cmdline, const char *data_dir, char *session_user, char *session_role) {
    char tmp[MAX_LINE];
    strncpy(tmp, cmdline, sizeof(tmp));
    char *tokens[8]; int n=0;
    char *p = strtok(tmp,"|");
    while (p && n<8) { tokens[n++] = p; p = strtok(NULL,"|"); }
    if (n==0) { send_line(client_sock,"ERR|BAD_REQ"); return; }

    if (strcmp(tokens[0], "ADMIN_ADD_DOCTOR_ACCOUNT") == 0) {
        // ADMIN_ADD_DOCTOR_ACCOUNT|username|password|full_name|specialty
        if (n < 5) { send_line(client_sock,"ERR|USAGE|ADMIN_ADD_DOCTOR_ACCOUNT|username|password|full_name|specialty"); return; }
        // tạo tài khoản bác sĩ (users.txt) rồi thêm vào danh sách bác sĩ
        int reg = db_register_user(data_dir, tokens[1], tokens[2], "doctor", tokens[3]);
        if (reg == -2) { send_line(client_sock, "ERR|USER_EXISTS"); return; }
        if (reg != 0)  { send_line(client_sock, "ERR|DB"); return; }

        char outdoc[256];
        if (db_admin_add_doctor(data_dir, tokens[3], tokens[4], outdoc, sizeof(outdoc)) != 0) {
            // Rollback user entry nếu không thêm được bác sĩ
            char rollback[128];
            db_admin_delete_user(data_dir, tokens[1], rollback, sizeof(rollback));
            send_line(client_sock, "ERR|DB");
            return;
        }

        int did = 0;
        sscanf(outdoc, "OK|DOCTOR_ADDED|id=%d", &did);
        int link = db_link_doctor_user(data_dir, did, tokens[1]);
        if (link != 0) {
            // Rollback user entry nếu không liên kết được
            char rollback[128];
            db_admin_delete_user(data_dir, tokens[1], rollback, sizeof(rollback));
            send_line(client_sock, link == -2 ? "ERR|DOCTOR_ACCOUNT_EXISTS" : "ERR|DB");
            return;
        }

        char resp[128];
        if (did > 0) snprintf(resp, sizeof(resp), "OK|DOCTOR_ACCOUNT_CREATED|id=%d", did);
        else snprintf(resp, sizeof(resp), "OK|DOCTOR_ACCOUNT_CREATED");
        send_line(client_sock, resp);

    } else if (strcmp(tokens[0], "ADMIN_ADD_DOCTOR") == 0) {
        if (n < 3) { send_line(client_sock,"ERR|USAGE|ADMIN_ADD_DOCTOR|name|specialty"); return; }
        char out[256]; db_admin_add_doctor(data_dir, tokens[1], tokens[2], out, sizeof(out));
        send_line(client_sock, out);
    } else if (strcmp(tokens[0], "ADMIN_LIST_USERS") == 0) {
        char out[MAX_LINE];
        if (db_admin_list_users(data_dir, out, sizeof(out))==0) {
            send_line(client_sock,"OK|USERS_BEGIN");
            char *lp=out; char *ln;
            while ((ln=strtok(lp,"\n"))) { lp=NULL; send_line(client_sock, ln); }
            send_line(client_sock,"OK|USERS_END");
        } else send_line(client_sock,"ERR|DB");
    } else if (strcmp(tokens[0], "ADMIN_LIST_DOCTORS") == 0) {
        char out[MAX_LINE]; if (db_list_doctors(data_dir, out, sizeof(out))==0) {
            send_line(client_sock,"OK|DOCTORS_BEGIN");
            char *lp=out; char *ln;
            while ((ln=strtok(lp,"\n"))) { lp=NULL; send_line(client_sock, ln); }
            send_line(client_sock,"OK|DOCTORS_END");
        } else send_line(client_sock,"ERR|DB");
    } else if (strcmp(tokens[0], "ADMIN_LIST_ALL_BOOKINGS") == 0) {
        char out[MAX_LINE]; if (db_admin_list_bookings(data_dir,out,sizeof(out))==0) {
            send_line(client_sock,"OK|BOOKINGS_BEGIN");
            char *lp=out; char *ln;
            while ((ln=strtok(lp,"\n"))) { lp=NULL; send_line(client_sock, ln); }
            send_line(client_sock,"OK|BOOKINGS_END");
        } else send_line(client_sock,"ERR|DB");
    } else if (strcmp(tokens[0], "ADMIN_DELETE_USER") == 0) {
        if (n < 2) { send_line(client_sock,"ERR|USAGE|ADMIN_DELETE_USER|username"); return; }
        char out[256]; db_admin_delete_user(data_dir, tokens[1], out, sizeof(out));
        send_line(client_sock, out);
    } else {
        send_line(client_sock,"ERR|UNKNOWN_CMD");
    }
}
