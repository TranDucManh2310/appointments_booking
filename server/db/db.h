#ifndef SERVER_DB_H
#define SERVER_DB_H

// initialize data dir & files
int db_init(const char *data_dir);

// users: username|password|role|full_name
int db_register_user(const char *data_dir, const char *username, const char *password, const char *role, const char *fullname);
int db_auth_user(const char *data_dir, const char *username, const char *password, char *role_out);

// doctors: id|name|specialty
int db_list_doctors(const char *data_dir, char *outbuf, size_t outsz);
int db_admin_add_doctor(const char *data_dir, const char *name, const char *specialty, char *out, size_t outsz);

// appointments: id|doctorId|date|time|username|status
int db_book_appointment(const char *data_dir, const char *username, int doctorId, const char *date, const char *time, char *out, size_t outsz);
int db_list_slots(const char *data_dir, int doctorId, const char *date, char *out, size_t outsz);
int db_view_user_appts(const char *data_dir, const char *username, char *out, size_t outsz);
int db_cancel_appointment(const char *data_dir, const char *username, int apptId, char *out, size_t outsz);
int db_doctor_view_bookings(const char *data_dir, int doctorId, const char *date, char *out, size_t outsz);
int db_doctor_update_status(const char *data_dir, int apptId, const char *status, char *out, size_t outsz);

int db_admin_list_users(const char *data_dir, char *out, size_t outsz);
int db_admin_list_bookings(const char *data_dir, char *out, size_t outsz);
int db_admin_delete_user(const char *data_dir, const char *username, char *out, size_t outsz);

#endif
