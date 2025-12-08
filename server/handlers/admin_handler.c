#include "admin_handler.h"
#include "../db/db.h"
#include "../../shared/socket.h"
#include <stdio.h>
#include <string.h>

void handle_admin(int client_sock, const char *cmdline, const char *data_dir, char *session_user, char *session_role) {
    char tmp[MAX_LINE];
    strncpy(tmp, cmdline, sizeof(tmp));
    char *tokens[8]; int n=0;
    char *p = strtok(tmp,"|");
    while (p && n<8) { tokens[n++] = p; p = strtok(NULL,"|"); }
    if (n==0) { send_line(client_sock,"ERR|BAD_REQ"); return; }

    if (strcmp(tokens[0], "ADMIN_ADD_DOCTOR") == 0) {
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
