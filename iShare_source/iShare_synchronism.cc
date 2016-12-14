
#include <memory>
#include <iostream>
#include <string>
#include <pthread.h>

#include <grpc++/grpc++.h>
#include <grpc/support/log.h>

#include "iShare.grpc.pb.h"
#include "iShare_server.h"
#include "mysql_pool.h"
using grpc::Server;
using grpc::ServerAsyncWriter;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerCompletionQueue;
using grpc::Status;
using helloworld::Inf;
using helloworld::Syn_data;
using helloworld::Synchronism;
using helloworld::ServerImpl;

void* ServerImpl::SynServer(void* arg) {
    ServerImpl* self = (ServerImpl*)arg;
    self->SynServer();
    return NULL;
}

void ServerImpl::SynServer() {
    log(INFO, "Start SYN Service");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;

    while (1) {
        string sql_command;
        string msg = to_string(users_.size()) + " user online " + to_string(mp_.size());
        log(DEBUG, msg.c_str());
        std::vector<std::string> cancel_users;
        for (auto& i : mp_) {
            if (i.second->ctx_.IsCancelled()) {
                cancel_users.push_back(i.first);
            }
        }
        for (auto& i : cancel_users) {
            delete mp_[i];
            users_.erase(i);
            mp_.erase(i);

        }

        for (auto user_id : users_) {
            string note = "check user " + user_id;
            //log(DEBUG, note.c_str());
            sql_command = "SELECT synchronism_friend, synchronism_bill, synchronism_delete, synchronism_request FROM User WHERE user_id =" + user_id;
            if (!mysql_query(conn, sql_command.data())) {

                //cout << sql_command << endl;
                res = mysql_use_result(conn);
                if (res != NULL) {
                    row = mysql_fetch_row(res);
                    if (row != NULL) {
                        string friend_(row[0]), bill_(row[1]), delete_(row[2]), request_(row[3]);
                        if (friend_ != "0" || bill_ != "0" || delete_ != "0" || request_ != "0") {
                            //std::cout << row[0] << " " << row[1] << " " << row[2] << " " << row[3] << std::endl;
                            Syn_data reply;
                            reply.set_friend_(row[0]);
                            reply.set_bill(row[1]);
                            reply.set_delete_(row[2]);
                            reply.set_request(row[3]);
                            CallData *call_data = mp_[user_id];
                            if (call_data) {
                                call_data->responder_.Write(reply, call_data);
                            }
                        }
                    } else {
                        log(WARNING, "row == NULL");
                        log(INFO, sql_command.data());

                        // invalid user, cancel service
                        CallData *call_data = mp_[user_id];
                        if (call_data) {
                            call_data->responder_.Finish(Status::CANCELLED, call_data);
                            std::cout << "tried to cancel" << std::endl;
                        }
                        // get new conn
                        check_sql_sock_normal(sock_node);
                        release_sock_to_sql_pool(sock_node);
                        sock_node = get_sock_from_pool();
                        conn = sock_node->sql_sock->sock;
                    }
                } else {
                    log(WARNING, "error res == NULL");
                    log(WARNING, sql_command.data());

                    check_sql_sock_normal(sock_node);
                    release_sock_to_sql_pool(sock_node);
                    sock_node = get_sock_from_pool();
                    conn = sock_node->sql_sock->sock;
                }
                mysql_free_result(res);
            } else {
                log(ERROR, sql_command.data());
                log(ERROR, mysql_error(conn));

                check_sql_sock_normal(sock_node);
                release_sock_to_sql_pool(sock_node);
                sock_node = get_sock_from_pool();
                conn = sock_node->sql_sock->sock;
            }

        }

        sleep(1);
    }
    release_sock_to_sql_pool(sock_node);
    log(INFO, "End SYN Service");
}


// There is no shutdown handling in this code.
void ServerImpl::Run() {
    std::string server_address("0.0.0.0:50053");
    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service_" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *asynchronous* service.
    builder.RegisterService(&service_);
    GreeterServiceImpl service;
    builder.RegisterService(&service);
    // Get hold of the completion queue used for the asynchronous communication
    // with the gRPC runtime.
    cq_ = builder.AddCompletionQueue();
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;

    pthread_t t1;
    pthread_create(&t1, NULL,ServerImpl::SynServer, this);
    // Proceed to the server's main loop.

    HandleRpcs();
}


void ServerImpl::CallData::Proceed() {
    if (status_ == CREATE) {
        // Make this instance progress to the PROCESS state.
        status_ = PROCESS;

        // As part of the initial CREATE state, we *request* that the system
        // start processing SayHello requests. In this request, "this" acts are
        // the tag uniquely identifying the request (so that different CallData
        // instances can serve different requests concurrently), in this case
        // the memory address of this CallData instance.
        service_->RequestSyn(&ctx_, &request_, &responder_, cq_, cq_,
                this);
    } else if (status_ == PROCESS) {
        // Spawn a new CallData instance to serve new clients while we process
        // the one for this CallData. The instance will deallocate itself as
        // part of its FINISH state.

        if (first_process_) {
            new CallData(service_, cq_, users_, mp_);
            first_process_ = false;
        }

        // The actual processing.
        std::string prefix("Hello ");
        //reply_.set_message(prefix + request_.name());

        // And we are done! Let the gRPC runtime know we've finished, using the
        // memory address of this instance as the uniquely identifying tag for
        // the event.

/*
        for (int i = 0; i < 10; i++) {
            Syn_data dat;
            dat.set_friend_("hi");
            responder_.Write(dat, this);
        }
        std::cout << "end push" << std::endl;
        status_ = FINISH;
        responder_.Finish(Status::OK, this);
        */
        std::cout << "get request" << std::endl;
        if ((*mp_).find(request_.information()) == (*mp_).end()) {
            std::cout << "get a new client" << std::endl;

            users_->insert(request_.information());
            (*mp_)[request_.information()] = this;
        }
    } else {
        GPR_ASSERT(status_ == FINISH);
        std::cout << "finish one request" << std::endl;
        // Once in the FINISH state, deallocate ourselves (CallData).
        users_->erase(request_.information());
        mp_->erase(request_.information());
        delete this;
    }
}

// This can be run in multiple threads if needed.
void ServerImpl::HandleRpcs() {
    // Spawn a new CallData instance to serve new clients.
    new CallData(&service_, cq_.get(), &users_, &mp_);
    void* tag;  // uniquely identifies a request.
    bool ok;
    while (true) {
        // Block waiting to read the next event from the completion queue. The
        // event is uniquely identified by its tag, which in this case is the
        // memory address of a CallData instance.
        // The return value of Next should always be checked. This return value
        // tells us whether there is any kind of event or cq_ is shutting down.
        GPR_ASSERT(cq_->Next(&tag, &ok));
        GPR_ASSERT(ok);
        static_cast<CallData*>(tag)->Proceed();
    }
}

