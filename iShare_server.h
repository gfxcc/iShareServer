/* iShare_server.h
 *
 * Created by Yong Cao at Jan 26 2016
 *
 * */

enum LOG_LEVEL {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};


void log(LOG_LEVEL level, const char* msg);
