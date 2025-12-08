#ifndef DOCTOR_HANDLER_H
#define DOCTOR_HANDLER_H

void handle_doctor(int client_sock, const char *cmdline, const char *data_dir, char *session_user, char *session_role);

#endif
