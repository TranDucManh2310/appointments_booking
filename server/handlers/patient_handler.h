#ifndef PATIENT_HANDLER_H
#define PATIENT_HANDLER_H

void handle_patient(int client_sock, const char *cmdline, const char *data_dir, char *session_user, char *session_role);

#endif
