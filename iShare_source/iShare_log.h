/* iShare_log.h
 *
 * Created by Yong Cao at Jan 26 2016
 *
 *
 *
 *
 *
 * */

#include <string>
#include <iShare_server.h>
#include <time.h>
#include <iShare_mail.h>
#include <iShare_tools.h>

using namespace std;
class Log {
public:
    explicit Log(string inf) : inf_(inf) {
        print(INFO, "In " + inf_);
    }

    static void print(LOG_LEVEL level, string inf) {
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        printf("%d-%d-%d %.2d:%.2d:%.2d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        string msg = "[" + LogLevelToString(level) + "]:" + inf;
        cout << msg << endl;

        // mail stuff
        if (level >= WARNING) {
            string subject = "[iShare Server Report: " + LogLevelToString(level) + "]";
            Mail mail(subject, inf);
            mail.send();
        }
    }

    ~Log() {
        print(INFO, "OUT " + inf_);
    }
private:
    string inf_;
};

