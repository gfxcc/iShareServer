/* iShare_server.h
 *
 * Created by Yong Cao at Jan 26 2016
 *
 * */
#include <string>
#include <sstream>

#define CONN_NUM 5
#define LINE_MAX_LENGTH 50

enum LOG_LEVEL {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};


void log(LOG_LEVEL level, const char* msg);
void tabPrint(std::string str);
std::string convertToString(double d);
