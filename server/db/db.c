#define _GNU_SOURCE
#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/file.h>
#include <errno.h>

// helper create data dir
static void ensure_data_dir(const char *data_dir) {
    char cmd[512]; snprintf(cmd,sizeof(cmd),"mkdir -p %s", data_dir);
    system(cmd);
}

int db_init(const char *data_dir) {
    ensure_data_dir(data_dir);
    char path[512];
    // users
    snprintf(path,sizeof(path),"%s/users.txt", data_dir);
    FILE *f = fopen(path,"a+"); if (f) {
        // seed admin if empty
        fseek(f,0,SEEK_END);
        if (ftell(f) == 0) fprintf(f,"admin|admin123|admin|Administrator\n");
        fclose(f);
    }
    // doctors
    snprintf(path,sizeof(path),"%s/doctors.txt", data_dir);
    f = fopen(path,"a+"); if (f) {
        fseek(f,0,SEEK_END);
        if (ftell(f) == 0) {
            fprintf(f,"1|Dr. Hung|Cardiology\n");
            fprintf(f,"2|Dr. Lan|Neurology\n");
        }
        fclose(f);
    }
    // appointments
    snprintf(path,sizeof(path),"%s/appointments.txt", data_dir);
    f = fopen(path,"a+"); if (f) fclose(f);
    return 0;
}

// helper lock file
static int lock_file(FILE *f) {
    int fd = fileno(f);
    if (flock(fd, LOCK_EX) != 0) return -1;
    return 0;
}
static void unlock_file(FILE *f) {
    int fd = fileno(f);
    flock(fd, LOCK_UN);
}

// register user
int db_register_user(const char *data_dir, const char *username, const char *password, const char *role, const char *fullname) {
    char path[512]; snprintf(path,sizeof(path), "%s/users.txt", data_dir);
    FILE *f = fopen(path, "r+");
    if (!f) return -1;
    lock_file(f);
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char u[128], p[128], r[32], name[256];
        sscanf(line, "%127[^|]|%127[^|]|%31[^|]|%255[^\n]", u,p,r,name);
        if (strcmp(u, username) == 0) { unlock_file(f); fclose(f); return -2; }
    }
    fprintf(f, "%s|%s|%s|%s\n", username, password, role, fullname ? fullname : "");
    fflush(f);
    unlock_file(f); fclose(f);
    return 0;
}

int db_auth_user(const char *data_dir, const char *username, const char *password, char *role_out) {
    char path[512]; snprintf(path,sizeof(path), "%s/users.txt", data_dir);
    FILE *f = fopen(path,"r");
    if (!f) return -1;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        char u[128], p[128], r[32], name[256];
        sscanf(line, "%127[^|]|%127[^|]|%31[^|]|%255[^\n]", u,p,r,name);
        if (strcmp(u, username) == 0 && strcmp(p, password) == 0) {
            if (role_out) strncpy(role_out, r, 31);
            fclose(f); return 0;
        }
    }
    fclose(f); return -1;
}

int db_list_doctors(const char *data_dir, char *outbuf, size_t outsz) {
    char path[512]; snprintf(path,sizeof(path), "%s/doctors.txt", data_dir);
    FILE *f = fopen(path,"r");
    if (!f) return -1;
    outbuf[0] = '\0';
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        strncat(outbuf, line, outsz - strlen(outbuf) - 1);
    }
    fclose(f);
    return 0;
}

int db_admin_add_doctor(const char *data_dir, const char *name, const char *specialty, char *out, size_t outsz) {
    char path[512]; snprintf(path,sizeof(path), "%s/doctors.txt", data_dir);
    FILE *f = fopen(path,"r+");
    if (!f) { snprintf(out, outsz, "ERR|DB"); return -1; }
    lock_file(f);
    int maxid = 0;
    char line[512];
    while (fgets(line,sizeof(line),f)) {
        int id; char nm[256], sp[256];
        if (sscanf(line, "%d|%255[^|]|%255[^\n]", &id, nm, sp) == 3) {
            if (id > maxid) maxid = id;
        }
    }
    int nid = maxid + 1;
    fprintf(f, "%d|%s|%s\n", nid, name, specialty);
    fflush(f);
    unlock_file(f); fclose(f);
    snprintf(out, outsz, "OK|DOCTOR_ADDED|id=%d", nid);
    return 0;
}

// appointments operations
// appointments file format: id|doctorId|date|time|username|status
int db_book_appointment(const char *data_dir, const char *username, int doctorId, const char *date, const char *time, char *out, size_t outsz) {
    char path[512]; snprintf(path,sizeof(path), "%s/appointments.txt", data_dir);
    FILE *f = fopen(path,"r+");
    if (!f) { snprintf(out,outsz,"ERR|DB"); return -1; }
    lock_file(f);
    // check conflict
    char line[512];
    int lastid = 0;
    while (fgets(line,sizeof(line),f)) {
        int id, did; char d[32], t[16], u[128], st[32];
        if (sscanf(line,"%d|%d|%31[^|]|%15[^|]|%127[^|]|%31[^\n]", &id,&did,d,t,u,st) >= 5) {
            if (id > lastid) lastid = id;
            if (did == doctorId && strcmp(d,date)==0 && strcmp(t,time)==0 && strcmp(st,"CANCELLED")!=0) {
                unlock_file(f); fclose(f);
                snprintf(out,outsz,"ERR|TIME_SLOT_TAKEN");
                return -2;
            }
        }
    }
    int nid = lastid + 1;
    fprintf(f, "%d|%d|%s|%s|%s|%s\n", nid, doctorId, date, time, username, "PENDING");
    fflush(f);
    unlock_file(f); fclose(f);
    snprintf(out,outsz,"OK|BOOKED|appointmentId=%d", nid);
    return 0;
}

int db_list_slots(const char *data_dir, int doctorId, const char *date, char *out, size_t outsz) {
    // For simplicity: we return booked times (so client can infer availability)
    // read appointments and list times booked for doctor on date
    char path[512]; snprintf(path,sizeof(path), "%s/appointments.txt", data_dir);
    FILE *f = fopen(path,"r");
    if (!f) return -1;
    out[0] = '\0';
    char line[512];
    while (fgets(line,sizeof(line),f)) {
        int id, did; char d[32], t[16], u[128], st[32];
        if (sscanf(line,"%d|%d|%31[^|]|%15[^|]|%127[^|]|%31[^\n]", &id,&did,d,t,u,st) >= 5) {
            if (did == doctorId && strcmp(d,date)==0) {
                char tmp[128];
                snprintf(tmp,sizeof(tmp), "%s|%s|%s\n", t, u, st);
                strncat(out, tmp, outsz - strlen(out) - 1);
            }
        }
    }
    fclose(f);
    return 0;
}

int db_view_user_appts(const char *data_dir, const char *username, char *out, size_t outsz) {
    char path[512]; snprintf(path,sizeof(path), "%s/appointments.txt", data_dir);
    FILE *f = fopen(path,"r");
    if (!f) return -1;
    out[0] = '\0';
    char line[512];
    while (fgets(line,sizeof(line),f)) {
        int id, did; char d[32], t[16], u[128], st[32];
        if (sscanf(line,"%d|%d|%31[^|]|%15[^|]|%127[^|]|%31[^\n]", &id,&did,d,t,u,st) >= 5) {
            if (strcmp(u, username) == 0) {
                char tmp[256];
                snprintf(tmp,sizeof(tmp), "%d|%d|%s|%s|%s\n", id, did, d, t, st);
                strncat(out, tmp, outsz - strlen(out) - 1);
            }
        }
    }
    fclose(f);
    return 0;
}

int db_cancel_appointment(const char *data_dir, const char *username, int apptId, char *out, size_t outsz) {
    char path[512], tmp[512];
    snprintf(path,sizeof(path),"%s/appointments.txt", data_dir);
    snprintf(tmp,sizeof(tmp), "%s/appointments.tmp", data_dir);
    FILE *f = fopen(path,"r");
    FILE *t = fopen(tmp,"w");
    if (!f || !t) { if (f) fclose(f); if (t) fclose(t); snprintf(out,outsz,"ERR|DB"); return -1; }
    int found = 0;
    char line[512];
    while (fgets(line,sizeof(line),f)) {
        int id, did; char d[32], tim[16], u[128], st[32];
        if (sscanf(line,"%d|%d|%31[^|]|%15[^|]|%127[^|]|%31[^\n]", &id,&did,d,tim,u,st) >= 5) {
            if (id == apptId && strcmp(u, username) == 0) { // mark cancelled by skipping or write with CANCELLED
                fprintf(t, "%d|%d|%s|%s|%s|%s\n", id, did, d, tim, u, "CANCELLED");
                found = 1;
            } else {
                fputs(line, t);
            }
        } else {
            fputs(line,t);
        }
    }
    fclose(f); fclose(t);
    if (found) {
        remove(path); rename(tmp, path);
        snprintf(out,outsz,"OK|CANCELLED|%d", apptId);
        return 0;
    } else {
        remove(tmp);
        snprintf(out,outsz,"ERR|NOT_FOUND");
        return -2;
    }
}

int db_doctor_view_bookings(const char *data_dir, int doctorId, const char *date, char *out, size_t outsz) {
    char path[512]; snprintf(path,sizeof(path),"%s/appointments.txt", data_dir);
    FILE *f = fopen(path,"r");
    if (!f) return -1;
    out[0] = '\0';
    char line[512];
    while (fgets(line,sizeof(line),f)) {
        int id, did; char d[32], tim[16], u[128], st[32];
        if (sscanf(line,"%d|%d|%31[^|]|%15[^|]|%127[^|]|%31[^\n]", &id,&did,d,tim,u,st) >= 5) {
            if (did == doctorId && (date==NULL || strcmp(d,date)==0)) {
                char tmp[256];
                snprintf(tmp,sizeof(tmp), "%d|%s|%s|%s\n", id, d, tim, u);
                strncat(out, tmp, outsz - strlen(out) - 1);
            }
        }
    }
    fclose(f);
    return 0;
}

int db_doctor_update_status(const char *data_dir, int apptId, const char *status, char *out, size_t outsz) {
    char path[512], tmp[512];
    snprintf(path,sizeof(path),"%s/appointments.txt", data_dir);
    snprintf(tmp,sizeof(tmp), "%s/appointments.tmp", data_dir);
    FILE *f = fopen(path,"r");
    FILE *t = fopen(tmp,"w");
    if (!f || !t) { if (f) fclose(f); if (t) fclose(t); snprintf(out,outsz,"ERR|DB"); return -1; }
    int found = 0;
    char line[512];
    while (fgets(line,sizeof(line),f)) {
        int id, did; char d[32], tim[16], u[128], st[32];
        if (sscanf(line,"%d|%d|%31[^|]|%15[^|]|%127[^|]|%31[^\n]", &id,&did,d,tim,u,st) >= 5) {
            if (id == apptId) {
                fprintf(t, "%d|%d|%s|%s|%s|%s\n", id, did, d, tim, u, status);
                found = 1;
            } else {
                fputs(line, t);
            }
        } else {
            fputs(line,t);
        }
    }
    fclose(f); fclose(t);
    if (found) { remove(path); rename(tmp, path); snprintf(out,outsz,"OK|UPDATED|%d", apptId); return 0; }
    else { remove(tmp); snprintf(out,outsz,"ERR|NOT_FOUND"); return -2; }
}

// admin operations
int db_admin_list_users(const char *data_dir, char *out, size_t outsz) {
    char path[512]; snprintf(path,sizeof(path), "%s/users.txt", data_dir);
    FILE *f = fopen(path,"r");
    if (!f) return -1;
    out[0] = '\0';
    char line[512];
    while (fgets(line,sizeof(line),f)) strncat(out, line, outsz - strlen(out) - 1);
    fclose(f); return 0;
}

int db_admin_list_bookings(const char *data_dir, char *out, size_t outsz) {
    char path[512]; snprintf(path,sizeof(path), "%s/appointments.txt", data_dir);
    FILE *f = fopen(path,"r");
    if (!f) return -1;
    out[0] = '\0'; char line[512];
    while (fgets(line,sizeof(line),f)) strncat(out, line, outsz - strlen(out) - 1);
    fclose(f); return 0;
}

int db_admin_delete_user(const char *data_dir, const char *username, char *out, size_t outsz) {
    char path[512], tmp[512];
    snprintf(path,sizeof(path), "%s/users.txt", data_dir);
    snprintf(tmp,sizeof(tmp), "%s/users.tmp", data_dir);
    FILE *f = fopen(path,"r");
    FILE *t = fopen(tmp,"w");
    if (!f || !t) { if (f) fclose(f); if (t) fclose(t); snprintf(out,outsz,"ERR|DB"); return -1; }
    int found = 0; char line[512];
    while (fgets(line,sizeof(line),f)) {
        char u[128], p[128], r[32], name[256];
        if (sscanf(line,"%127[^|]|%127[^|]|%31[^|]|%255[^\n]", u,p,r,name) >= 1) {
            if (strcmp(u, username) == 0) { found = 1; continue; }
        }
        fputs(line, t);
    }
    fclose(f); fclose(t);
    if (found) { remove(path); rename(tmp, path); snprintf(out,outsz,"OK|DELETED|%s", username); return 0; }
    remove(tmp); snprintf(out,outsz,"ERR|NOT_FOUND"); return -2;
}
