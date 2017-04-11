#include <string.h>

class Mail {
public:
    explicit Mail(std::string des, std::string subject, std::string body) : des_(des), subject_(subject), body_(body) {}
    explicit Mail(std::string subject, std::string body) : des_("gfxccc@gmail.com"), subject_(subject), body_(body) {}
    bool send();
    void set_des(std::string des) {
        des_ = des;
    }
    void set_subject(std::string subject) {
        subject_ = subject;
    }
    void set_body(std::string body) {
        body_ = body;
    }

private:
    std::string des_, subject_, body_;
};

