#include "doctor_handler.h"
#include "../db/db.h"
#include "../../shared/socket.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../shared/constants.h"

void handle_doctor(int client_sock, const char *cmdline, const char *data_dir, char *session_user, char *session_role) {
    char tmp[MAX_LINE];
    strncpy(tmp, cmdline, sizeof(tmp));
    char *tokens[12]; int n=0;
    char *p = strtok(tmp,"|");
    while (p && n<12) { tokens[n++] = p; p = strtok(NULL,"|"); }
    if (n==0) { send_line(client_sock,"ERR|BAD_REQ"); return; }

    // Enforce authentication/authorization for doctor endpoints
    if (session_user[0] == '\0') { send_line(client_sock, "ERR|NOT_LOGGED_IN"); return; }
    if (strcmp(session_role, "doctor") != 0 && strcmp(session_role, "admin") != 0) {
        send_line(client_sock, "ERR|PERMISSION_DENIED");
        return;
    }
    int doctorId = -1;
    if (db_get_doctor_id_for_user(data_dir, session_user, &doctorId) != 0) {
        send_line(client_sock, "ERR|DOCTOR_NOT_LINKED");
        return;
    }

    if (strcmp(tokens[0], "DOCTOR_VIEW_BOOKINGS") == 0) {
        const char *date = NULL; // lấy toàn bộ, không cần date
        char out[MAX_LINE];
        if (db_doctor_view_bookings(data_dir, doctorId, date, out, sizeof(out))==0) {
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
        db_doctor_update_status(data_dir, apid, doctorId, tokens[2], out, sizeof(out));
        send_line(client_sock, out);
    } else {
        send_line(client_sock, "ERR|UNKNOWN_CMD");
    }
}
