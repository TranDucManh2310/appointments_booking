#ifndef ADMIN_HANDLER_H
#define ADMIN_HANDLER_H

void handle_admin(int client_sock, const char *cmdline, const char *data_dir, char *session_user, char *session_role);

#endif
