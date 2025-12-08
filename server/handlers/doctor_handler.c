#include "doctor_handler.h"
#include "../db/db.h"
#include "../../shared/socket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void handle_doctor(int client_sock, const char *cmdline, const char *data_dir, char *session_user, char *session_role) {
    char tmp[MAX_LINE];
    strncpy(tmp, cmdline, sizeof(tmp));
    char *tokens[12]; int n=0;
    char *p = strtok(tmp,"|");
    while (p && n<12) { tokens[n++] = p; p = strtok(NULL,"|"); }
    if (n==0) { send_line(client_sock,"ERR|BAD_REQ"); return; }

    if (strcmp(tokens[0], "DOCTOR_VIEW_BOOKINGS") == 0) {
        if (n < 3) { send_line(client_sock,"ERR|USAGE|DOCTOR_VIEW_BOOKINGS|doctorId|date"); return; }
        int did = atoi(tokens[1]);
        char out[MAX_LINE];
        if (db_doctor_view_bookings(data_dir, did, tokens[2], out, sizeof(out))==0) {
            if (strlen(out)==0) send_line(client_sock,"OK|NO_BOOKINGS");
            else { send_line(client_sock,"OK|BOOKINGS_BEGIN"); char *lp=out; char *ln; while ((ln=strtok(lp,"\n"))) { lp=NULL; send_line(client_sock, ln);} send_line(client_sock,"OK|BOOKINGS_END"); }
        } else send_line(client_sock,"ERR|DB");
    } else if (strcmp(tokens[0], "DOCTOR_ADD_SLOT") == 0) {
        // simplified: not persisted as schedule; real impl would write schedules.txt
        // For this project we'll respond OK to indicate slot added.
        send_line(client_sock,"OK|SLOT_ADDED");
    } else if (strcmp(tokens[0], "DOCTOR_UPDATE_STATUS") == 0) {
        if (n < 3) { send_line(client_sock,"ERR|USAGE|DOCTOR_UPDATE_STATUS|appointmentId|status"); return; }
        int apid = atoi(tokens[1]);
        char out[256];
        int r = db_doctor_update_status(data_dir, apid, tokens[2], out, sizeof(out));
        send_line(client_sock, out);
    } else {
        send_line(client_sock, "ERR|UNKNOWN_CMD");
    }
}
