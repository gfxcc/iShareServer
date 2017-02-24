/*
 *
 * Copyright 2015, Google Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <pthread.h>
#include <thread>
#include <unistd.h>

#include <grpc++/grpc++.h>
#include <grpc/grpc.h>
#include <grpc++/channel.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>


#include "../iShare_source/iShare.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReader;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Synchronism;
using helloworld::Inf;
using helloworld::Syn_data;
using namespace std;
class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Synchronism::NewStub(channel)) {}

  // Assambles the client's payload, sends it and presents the response back
  // from the server.

  string syn(string name) {
    ClientContext context;
    Inf inf;
    inf.set_information(name);
    cout << "FROM " << name<< 1 << endl;
    unique_ptr<ClientReader<Syn_data>> reader (stub_->Syn(&context, inf));
    cout << "FROM " << name<< 2 << endl;
    Syn_data dat;

    while (reader->Read(&dat)) {
        cout << "FROM" << name << dat.friend_() << " " << dat.bill() << " " << endl;
    }
    cout << 3 << endl;
    Status status = reader->Finish();
    if (status.ok())
        cout << "OK" << endl;
    else
        cout << "ER" << endl;
    return "";
  }
 private:
  std::unique_ptr<Synchronism::Stub> stub_;
};
//int num;

void task(int num) {
    cout << "new thread " << num  << endl;

    GreeterClient greeter(grpc::CreateChannel(
      "localhost:50053", grpc::InsecureChannelCredentials()));
    greeter.syn(to_string(num));
}
int main(int argc, char** argv) {
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  //GreeterClient greeter(grpc::CreateChannel(
  //    "localhost:50053", grpc::InsecureChannelCredentials()));
  //std::string user("world");
  //std::string reply = greeter.SayHello(user);
  //std::cout << "Greeter received: " << reply << std::endl;
    //greeter.syn("1");
    pthread_t t1;
    vector<thread*> z;
    for (int i = 1; i <= 100; i++) {
        //num = i;
        auto p = new thread(task, i);
        if (p) {
            cout << "create succ" << endl;
            z.push_back(p);
        }
        sleep(0.01);
    }

    for (int i = 0; i < 100; i++) {
        z[i]->join();
        delete z[i];
    }
  return 0;
}
