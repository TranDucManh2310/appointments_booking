#include "protocol.h"
#include <string.h>
char *trim_newline(char *s) {
    if (!s) return NULL;
    char *p;
    if ((p = strchr(s,'\r'))) *p = '\0';
    if ((p = strchr(s,'\n'))) *p = '\0';
    return s;
}
