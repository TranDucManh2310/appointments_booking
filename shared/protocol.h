#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MAX_LINE 2048
#define MAX_PAYLOAD 1024

// Command strings (text-based protocol)
 // Common:
 // REGISTER|username|password|role|full_name
 // LOGIN|username|password
 // QUIT

 // Patient:
 // LIST_DOCTORS
 // LIST_SLOTS|doctorId|YYYY-MM-DD
 // BOOK|doctorId|YYYY-MM-DD|HH:MM
 // VIEW_MY_APPTS
 // CANCEL|appointmentId

 // Doctor:
 // DOCTOR_VIEW_BOOKINGS|doctorId|YYYY-MM-DD
 // DOCTOR_ADD_SLOT|doctorId|YYYY-MM-DD|start|end|slot_min
 // DOCTOR_UPDATE_STATUS|appointmentId|status

 // Admin:
 // ADMIN_ADD_DOCTOR|name|specialty
 // ADMIN_LIST_USERS
 // ADMIN_LIST_DOCTORS
 // ADMIN_LIST_ALL_BOOKINGS
 // ADMIN_DELETE_USER|username

// Helpers
char *trim_newline(char *s);

#endif
