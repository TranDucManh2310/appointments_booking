#include "patient_handler.h"
#include "../db/db.h"
#include "../../shared/socket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void handle_patient(int client_sock, const char *cmdline, const char *data_dir, char *session_user, char *session_role) {
    // cmdline is like COMMAND|arg1|arg2|...
    char tmp[MAX_LINE];
    strncpy(tmp, cmdline, sizeof(tmp));
    char *tokens[10]; int n=0;
    char *p = strtok(tmp, "|");
    while (p && n < 10) { tokens[n++] = p; p = strtok(NULL, "|"); }
    if (n == 0) { send_line(client_sock, "ERR|BAD_REQ"); return; }
    if (strcmp(tokens[0], "REGISTER") == 0) {
        if (n < 5) { send_line(client_sock, "ERR|USAGE|REGISTER|username|password|role|full_name"); return; }
        int r = db_register_user(data_dir, tokens[1], tokens[2], tokens[3], tokens[4]);
        if (r == -2) send_line(client_sock, "ERR|USER_EXISTS");
        else if (r == 0) send_line(client_sock, "OK|REGISTERED");
        else send_line(client_sock, "ERR|DB");
    } else if (strcmp(tokens[0], "LOGIN") == 0) {
        if (n < 3) { send_line(client_sock, "ERR|USAGE|LOGIN|username|password"); return; }
        char role[32] = {0};
        if (db_auth_user(data_dir, tokens[1], tokens[2], role) == 0) {
            strncpy(session_user, tokens[1], 127);
            strncpy(session_role, role, 31);
            char out[256]; snprintf(out,sizeof(out),"OK|LOGIN|%s", role);
            send_line(client_sock, out);
        } else send_line(client_sock, "ERR|AUTH_FAILED");
    } else if (strcmp(tokens[0], "LIST_DOCTORS") == 0) {
        char out[MAX_LINE]; if (db_list_doctors(data_dir, out, sizeof(out))==0) {
            send_line(client_sock, "OK|DOCTORS_BEGIN");
            // send each line
            char *lp = out;
            char *ln;
            while ((ln = strtok(lp, "\n"))) { lp = NULL; send_line(client_sock, ln); }
            send_line(client_sock, "OK|DOCTORS_END");
        } else send_line(client_sock, "ERR|DB");
    } else if (strcmp(tokens[0], "LIST_SLOTS") == 0) {
        if (n < 3) { send_line(client_sock, "ERR|USAGE|LIST_SLOTS|doctorId|date"); return; }
        int did = atoi(tokens[1]);
        char out[MAX_LINE]; if (db_list_slots(data_dir, did, tokens[2], out, sizeof(out))==0) {
            if (strlen(out)==0) send_line(client_sock, "OK|NO_BOOKINGS");
            else { send_line(client_sock, "OK|SLOTS_BEGIN"); char *lp=out; char *ln; while ((ln=strtok(lp,"\n"))) { lp=NULL; send_line(client_sock, ln);} send_line(client_sock,"OK|SLOTS_END"); }
        } else send_line(client_sock, "ERR|DB");
    } else if (strcmp(tokens[0], "BOOK") == 0) {
        if (n < 4) { send_line(client_sock, "ERR|USAGE|BOOK|doctorId|date|time"); return; }
        if (session_user[0] == '\0') { send_line(client_sock,"ERR|NOT_LOGGED_IN"); return; }
        char out[256]; int did = atoi(tokens[1]);
        int r = db_book_appointment(data_dir, session_user, did, tokens[2], tokens[3], out, sizeof(out));
        send_line(client_sock, out);
    } else if (strcmp(tokens[0], "VIEW_MY_APPTS") == 0) {
        if (session_user[0]=='\0') { send_line(client_sock,"ERR|NOT_LOGGED_IN"); return; }
        char out[MAX_LINE];
        if (db_view_user_appts(data_dir, session_user, out, sizeof(out))==0) {
            if (strlen(out)==0) send_line(client_sock, "OK|NO_APPTS");
            else { send_line(client_sock,"OK|MY_APPTS_BEGIN"); char *lp=out; char *ln; while ((ln=strtok(lp,"\n"))) { lp=NULL; send_line(client_sock, ln);} send_line(client_sock,"OK|MY_APPTS_END"); }
        } else send_line(client_sock, "ERR|DB");
    } else if (strcmp(tokens[0], "CANCEL") == 0) {
        if (n < 2) { send_line(client_sock, "ERR|USAGE|CANCEL|appointmentId"); return; }
        if (session_user[0]=='\0') { send_line(client_sock,"ERR|NOT_LOGGED_IN"); return; }
        int apid = atoi(tokens[1]);
        char out[256];
        int r = db_cancel_appointment(data_dir, session_user, apid, out, sizeof(out));
        send_line(client_sock, out);
    } else {
        send_line(client_sock, "ERR|UNKNOWN_CMD");
    }
}
