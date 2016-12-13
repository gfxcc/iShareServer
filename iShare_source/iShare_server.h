/* iShare_server.h
 *
 * Created by Yong Cao at Jan 26 2016
 *
 * */
#include <string>
#include <sstream>
#include <thread>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <grpc++/grpc++.h>
#include <grpc/support/log.h>
/*
#include <grpc++/support/async_stream.h>
#include <grpc++/impl/rpc_method.h>
#include <grpc++/impl/proto_utils.h>
#include <grpc++/impl/service_type.h>
#include <grpc++/support/async_unary_call.h>
#include <grpc++/support/status.h>
#include <grpc++/support/stub_options.h>
#include <grpc++/support/sync_stream.h>
*/

#include "iShare.grpc.pb.h"

#define CONN_NUM 50
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
bool pushNotificationToDevice (std::string deviceToken, std::string message);

namespace grpc {
    class CompletionQueue;
    class Channel;
    class RpcService;
    class ServerCompletionQueue;
    class ServerContext;
}  // namespace grpc
using grpc::Server;
using grpc::ServerAsyncResponseWriter;
using grpc::ServerBuilder;
using grpc::ServerCompletionQueue;
using grpc::ServerContext;
using grpc::ServerAsyncWriter;

namespace helloworld {

    class GreeterServiceImpl final : public Greeter::Service {
        ::grpc::Status SayHello(::grpc::ServerContext* context, const ::helloworld::HelloRequest* request, ::helloworld::HelloReply* reply) override;
        ::grpc::Status User_inf(::grpc::ServerContext* context, const ::helloworld::Inf* request, ::helloworld::User_detail* reply) override;
        ::grpc::Status Login (::grpc::ServerContext* context, const ::helloworld::Login_m* request, ::helloworld::Reply_inf* reply) override;
        ::grpc::Status Sign_up (::grpc::ServerContext* context, const ::helloworld::Sign_m* request, ::helloworld::Reply_inf* reply) override;
        ::grpc::Status Search_username (::grpc::ServerContext* context, const ::helloworld::Inf* request, ::helloworld::Search_result* reply) override;
        ::grpc::Status Add_friend (::grpc::ServerContext* context, const ::helloworld::Repeated_string* request, ::helloworld::Inf* reply) override;
        ::grpc::Status Delete_friend (::grpc::ServerContext* context, const ::helloworld::Repeated_string* request, ::helloworld::Inf* reply) override;
        ::grpc::Status Send_Img (::grpc::ServerContext *context, ::grpc::ServerReader<::helloworld::Image>* reader, ::helloworld::Inf* reply) override;
        ::grpc::Status Create_share (::grpc::ServerContext *context, const ::helloworld::Share_inf* request, ::helloworld::Inf* reply) override;
        ::grpc::Status Delete_bill (::grpc::ServerContext *context, const ::helloworld::Share_inf* request, ::helloworld::Inf* reply) override;
        ::grpc::Status Receive_Img (::grpc::ServerContext *context, const ::helloworld::Repeated_string* request, ::grpc::ServerWriter<::helloworld::Image>* reply) override;
        ::grpc::Status Obtain_bills (::grpc::ServerContext *context, const ::helloworld::Bill_request *request, ::grpc::ServerWriter<::helloworld::Share_inf> *reply) override;
        ::grpc::Status Reset_Status (::grpc::ServerContext *context, const ::helloworld::Inf* request, ::helloworld::Inf* reply) override;
        ::grpc::Status Send_request (::grpc::ServerContext* content, const ::helloworld::Request* request, ::helloworld::Inf* reply) override;
        ::grpc::Status Obtain_request (::grpc::ServerContext* content, const ::helloworld::Inf* request, ::grpc::ServerWriter<::helloworld::Request>* reply) override;
        ::grpc::Status Obtain_requestLog (::grpc::ServerContext* content, const ::helloworld::Inf* request, ::grpc::ServerWriter<::helloworld::Request>* reply) override;
        ::grpc::Status Obtain_requestLogHistory (::grpc::ServerContext* content, const ::helloworld::Inf* request, ::grpc::ServerWriter<::helloworld::Request>* reply) override;
        ::grpc::Status Request_response (::grpc::ServerContext* content, const ::helloworld::Response* request, ::helloworld::Inf* reply) override;
        ::grpc::Status MakePayment (::grpc::ServerContext* content, ::grpc::ServerReader<::helloworld::BillPayment>* reader, ::helloworld::Inf* reply) override;
        ::grpc::Status Create_requestLog (::grpc::ServerContext* content, const ::helloworld::Request* request, ::helloworld::Inf* reply) override;
        ::grpc::Status IgnoreRequestLog (::grpc::ServerContext* content, const ::helloworld::IgnoreMessage* request, ::helloworld::Inf* reply) override;
        ::grpc::Status Send_DeviceToken (::grpc::ServerContext* content, const ::helloworld::Repeated_string* request, ::helloworld::Inf* reply) override;
        ::grpc::Status Obtain_setting (::grpc::ServerContext* content, const ::helloworld::Inf* request, ::helloworld::Setting* reply) override;
        ::grpc::Status Reset_setting (::grpc::ServerContext* content, const ::helloworld::Setting* request, ::helloworld::Inf* reply) override;
        ::grpc::Status Reset_userInfo (::grpc::ServerContext* content, const ::helloworld::UserInfo* request, ::helloworld::Inf* reply) override;
        ::grpc::Status Obtain_userInfo (::grpc::ServerContext* content, const ::helloworld::Inf* request, ::helloworld::UserInfo* reply) override;
        ::grpc::Status Update_user_lastModified (::grpc::ServerContext* content, const ::helloworld::Inf* request, ::helloworld::Inf* reply) override;
    };

    class ServerImpl final {
        public:
            ~ServerImpl() {
                server_->Shutdown();
                // Always shutdown the completion queue after the server.
                cq_->Shutdown();
            }

            // There is no shutdown handling in this code.
            void Run();

        private:
            // Class encompasing the state and logic needed to serve a request.
            class CallData {
                public:
                    // Take in the "service" instance (in this case representing an asynchronous
                    // server) and the completion queue "cq" used for asynchronous communication
                    // with the gRPC runtime.
                    CallData(Synchronism::AsyncService* service, ServerCompletionQueue* cq, std::unordered_set<std::string>* users, std::unordered_map<std::string, CallData*>* mp)
                        : service_(service), cq_(cq), responder_(&ctx_), status_(CREATE), users_(users), mp_(mp) {
                            // Invoke the serving logic right away.
                            Proceed();
                        }

                    void Proceed();


                    // The means of communication with the gRPC runtime for an asynchronous
                    // server.
                    Synchronism::AsyncService* service_;
                    // The producer-consumer queue where for asynchronous server notifications.
                    ServerCompletionQueue* cq_;
                    // Context for the rpc, allowing to tweak aspects of it such as the use
                    // of compression, authentication, as well as to send metadata back to the
                    // client.
                    ServerContext ctx_;

                    // What we get from the client.
                    Inf request_;
                    // What we send back to the client.
                    //ServerAsyncWriter<Syn_data> reply_;

                    // The means to get back to the client.
                    ServerAsyncWriter<Syn_data> responder_;

                    // Let's implement a tiny state machine with the following states.
                    enum CallStatus { CREATE, PROCESS, FINISH };
                    CallStatus status_;  // The current serving state.

                    std::unordered_set<std::string> *users_;
                    std::unordered_map<std::string, CallData*> *mp_;


            };

            static void* SynServer(void*);
            void SynServer();
            // This can be run in multiple threads if needed.
            void HandleRpcs();

            std::unique_ptr<ServerCompletionQueue> cq_;
            Synchronism::AsyncService service_;
            std::unique_ptr<Server> server_;
            std::unordered_set<std::string> users_;
            std::unordered_map<std::string, CallData*> mp_;
    };
}
