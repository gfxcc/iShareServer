#include <fstream>
#include <stdlib.h>
#include "iShare_mail.h"

using namespace std;

bool Mail::send() {
    string content = "Subject:" + subject_ + " \n\n " + body_ +  "\n";
    string cmd = "echo \"" + content + "\" | " + "sendmail -f admin@ishare.com " + des_;
    if (system(cmd.c_str()) != 0)
        return false;
    return true;
}

