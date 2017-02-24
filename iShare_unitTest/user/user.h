
#include <iostream>
#include <memory>
#include <string>
#include <pthread.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>


#include "../../iShare_source/iShare.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using helloworld::Inf;
using helloworld::Greeter;
using helloworld::User_detail;
using helloworld::Reply_inf;
using helloworld::Repeated_string;
using helloworld::Search_result;
using helloworld::Login_m;
using helloworld::Sign_m;

using namespace std;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assambles the client's payload, sends it and presents the response back
  // from the server.
    User_detail Test_User_inf(Inf request);
    Reply_inf Test_Login(Login_m request);
    Reply_inf Test_Sign_up(Sign_m request);
    Search_result Test_Search_username(Inf request);
    Inf Test_Add_friend(Repeated_string request);
    Inf Test_Delete_friend(Repeated_string request);
    Inf Test_Reset_Status(Inf request);
    Inf Test_Update_user_lastModified(Inf request);
    Inf Test_Send_DeviceToken(Repeated_string request);

 private:
  std::unique_ptr<Greeter::Stub> stub_;
};

User_detail GreeterClient::Test_User_inf(Inf request) {
    ClientContext context;
    User_detail reply;
    Status status = stub_->User_inf(&context, request, &reply);
    return reply;
}

Reply_inf GreeterClient::Test_Login(Login_m request) {
    ClientContext context;
    Reply_inf reply;
    Status status = stub_->Login(&context, request, &reply);
    return reply;
}

Reply_inf GreeterClient::Test_Sign_up(Sign_m request) {
    ClientContext context;
    Reply_inf reply;
    Status status = stub_->Sign_up(&context, request, &reply);
    return reply;
}

Search_result GreeterClient::Test_Search_username(Inf request) {
    ClientContext context;
    Search_result reply;
    Status status = stub_->Search_username(&context, request, &reply);
    return reply;
}

Inf GreeterClient::Test_Add_friend(Repeated_string request) {
    ClientContext context;
    Inf reply;
    Status status = stub_->Add_friend(&context, request, &reply);
    return reply;
}

Inf GreeterClient::Test_Delete_friend(Repeated_string request) {
    ClientContext context;
    Inf reply;
    Status status = stub_->Delete_friend(&context, request, &reply);
    return reply;
}

Inf GreeterClient::Test_Reset_Status(Inf request) {
    ClientContext context;
    Inf reply;
    Status status = stub_->Reset_Status(&context, request, &reply);
    return reply;
}

Inf GreeterClient::Test_Update_user_lastModified(Inf request) {
    ClientContext context;
    Inf reply;
    Status status = stub_->Update_user_lastModified(&context, request, &reply);
    return reply;
}

Inf GreeterClient::Test_Send_DeviceToken(Repeated_string request) {
    ClientContext context;
    Inf reply;
    Status status = stub_->Send_DeviceToken(&context, request, &reply);
    return reply;
}









