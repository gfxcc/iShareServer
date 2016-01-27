/* tools.cc
 *
 * Created by Yong Cao at Jan 26 2016
 *
 * tools functions, log function
 *
 *
 *
 * */
#include <stdio.h>
#include "iShare_server.h"

void log (LOG_LEVEL level, const char* msg) {
    switch (level) {
        case DEBUG:
            printf("[DEBUG]:%s\n", msg);
            break;
        case INFO:
            printf("[INFO]:%s\n", msg);
            break;
        case WARNING:
            printf("[WARNING]:%s\n", msg);
            break;
        default :
            break;
    }

}
