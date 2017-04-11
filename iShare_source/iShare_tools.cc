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
#include <time.h>
#include "iShare_server.h"
#include "iShare_mail.h"

using namespace std;

void log (LOG_LEVEL level, const char* msg);
void tabPrint(std::string str);
string convertToString(double d);
string LogLevelToString(LOG_LEVEL level);

void log (LOG_LEVEL level, const char* msg) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    printf("%d-%d-%d %.2d:%.2d:%.2d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
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
        case ERROR:
            printf("[ERROR]:%s\n", msg);
        default :
            break;
    }

    if (level >= WARNING) {
        string subject = "[iShare Server Report: " + LogLevelToString(level) + "]";
        Mail mail(subject, msg);
        mail.send();
    }
}

// tools function
void tabPrint(std::string str) {
    int length = str.length();
    int pre = (LINE_MAX_LENGTH - length) / 2;
    for (int i = 0; i != pre; i++) {
        fprintf(stdout, "-");
    }
    fprintf(stdout, "%s", str.data());
    for (int i = 0; i != pre; i++) {
        fprintf(stdout, "-");
    }

    if (((pre * 2) + length) < LINE_MAX_LENGTH) {
        fprintf(stdout, "-");
    }

    fprintf(stdout,"\n");
}

string convertToString(double d) {
    ostringstream os;
    if (os << d)
        return os.str();
    return "invalid conversion";
}

string LogLevelToString(LOG_LEVEL level) {
    switch (level) {
        case DEBUG:
            return "DEBUG";
        case INFO:
            return "INFO";
        case WARNING:
            return "WARNING";
        case ERROR:
            return "ERROR";
        default:
            return "UNDEFINED";
    }
}
