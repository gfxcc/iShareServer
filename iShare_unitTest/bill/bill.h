
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
using helloworld::Share_inf;
using helloworld::Greeter;
using helloworld::Syn_data;
using helloworld::Bill_request;

using namespace std;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assambles the client's payload, sends it and presents the response back
  // from the server.
    vector<Share_inf> Test_Obtain_bills();
    Status Test_Delete_bill(Share_inf request);
    Status Test_Create_share();
 private:
  std::unique_ptr<Greeter::Stub> stub_;
};



vector<Share_inf> GreeterClient::Test_Obtain_bills() {
    vector<Share_inf> res;
    ClientContext context;
    Bill_request bill_request;
    bill_request.set_username("61");
    bill_request.set_amount("all");

    unique_ptr<ClientReader<Share_inf>> reader(stub_->Obtain_bills(&context, bill_request));

    Share_inf dat;
    while (reader->Read(&dat)) {
        res.push_back(dat);
    }

    Status status = reader->Finish();
    return res;
}

Status GreeterClient::Test_Delete_bill(Share_inf request) {
    ClientContext context;
    Inf inf;
    Status status = stub_->Delete_bill(&context, request, &inf);
    return status;
}

Status GreeterClient::Test_Create_share() {

    ClientContext context;
    Share_inf res;
    res.set_creater("61");
    res.set_amount("10");
    res.set_type("ihome");
    res.set_paidby("61");
    res.set_data("2017-02-24 08:42:00");
    res.set_note("hi");
    res.add_members("61");
    for (int i = 0; i < 9; i++)
        res.add_members("");
    res.set_typeicon("ihome");
    Inf inf;
    Status status = stub_->Create_share(&context, res, &inf);
    return status;
}

