/*
 * Created at 28/AUG/2015
 *
 * Copyright(C) 2016, YongCao
 *
 * */

#include <iostream>
#include <memory>
#include <string>
#include <list>

#include <mysql/mysql.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/server_credentials.h>
#include <grpc++/support/status.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include "iShare.grpc.pb.h"
#include "mysql_pool.h"
#include "iShare_server.h"

// include for MMGAPN
#include "../MMGAPN/global.hpp"
#include "../MMGAPN/MMGAPNSConnection.hpp"
#include "../MMGAPN/MMGDevice.hpp"
#include "../MMGAPN/MMGIOSPayload.hpp"
#include "../MMGAPN/MMGTools.hpp"
#include <vector>
#include <cstdlib>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
using helloworld::GreeterServiceImpl;
using helloworld::HelloRequest;
using helloworld::HelloReply;
using helloworld::Greeter;
using helloworld::Inf;
using helloworld::User_detail;
using helloworld::Sign_m;
using helloworld::Login_m;
using helloworld::Repeated_string;
using helloworld::Syn_data;
using helloworld::Image;
using helloworld::Share_inf;
using helloworld::Bill_request;
using helloworld::Syn_data;
using helloworld::Request;
using helloworld::Response;
using helloworld::IgnoreMessage;
using helloworld::BillPayment;
using helloworld::Setting;
using helloworld::UserInfo;
using helloworld::Reply_inf;
using namespace std;

bool pushNotificationToDevice (string deviceToken, string message);

// request system

Status GreeterServiceImpl::Send_request (ServerContext* content, const Request* request, Inf* reply) {
    log(INFO, "IN Send_request");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;

    // get receiver uiser_id
    string receiver_id;
    string sql_command = "SELECT user_id FROM User WHERE username = '" + request->receiver() + "'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Send_request");
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    if ((row = mysql_fetch_row(res)) != NULL) {
        receiver_id = row[0];
    }
    mysql_free_result(res);

    sql_command = "INSERT INTO Request (sender, receiver, type, content, request_date) VALUES (" +
        request->sender() + ", " + receiver_id + ", '" + request->type() + "', '" +
        request->content() + "', '" + request->request_date() + "')";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Send_request");
        return Status::CANCELLED;
    }

    sql_command = "UPDATE User SET synchronism_request = 1 WHERE user_id = " + receiver_id;

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Send_request");
        return Status::CANCELLED;
    }

    // push notification
    if (request->type() == "friendInvite") {
        sql_command = "SELECT deviceToken FROM User WHERE user_id = " + receiver_id + " AND N_friendInvite = 1";
    } else if (request->type() == "payment") {
        sql_command = "SELECT deviceToken FROM User WHERE user_id = " + receiver_id + " AND N_paidNotice = 1";
    }
    log(INFO, sql_command.data());
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, mysql_error(conn));
        log(ERROR, sql_command.data());

        reply->set_information("push notification fail");
        //printf("%s\n", );
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Send_request");
        return Status::OK;
    }
    res = mysql_use_result(conn);
    string message = "New rquest.";
    while ((row = mysql_fetch_row(res)) != NULL) {
        if (!row[0]) {
            continue;
        }
        log(INFO, row[0]);
        pushNotificationToDevice(row[0], message);
    }
    mysql_free_result(res);


    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Send_request");
    return Status::OK;
}

Status GreeterServiceImpl::Obtain_request (ServerContext* content, const Inf* request, ServerWriter<Request>* reply) {
    log(INFO, "IN Obtain_request");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    SQL_SOCK_NODE* sock_node_ = get_sock_from_pool();
    MYSQL* conn_ = sock_node_->sql_sock->sock;

    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL_RES *res_;
    MYSQL_ROW row_;

    string sql_command = "SELECT * FROM Request WHERE receiver = " + request->information() + " order by request_id desc";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        check_sql_sock_normal(sock_node_);
        release_sock_to_sql_pool(sock_node_);
        log(INFO, "OUT Obtain_request");
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL) {
        Request req;
        // conver user_id to username
        sql_command = "SELECT username FROM User WHERE user_id = " + request->information();
        if (mysql_query(conn_, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn_));

            mysql_free_result(res);

            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            check_sql_sock_normal(sock_node_);
            release_sock_to_sql_pool(sock_node_);
            log(INFO, "OUT Obtain_request");
            return Status::CANCELLED;
        }
        res_ = mysql_use_result(conn_);
        if ((row_ = mysql_fetch_row(res_)) != NULL) {
            req.set_receiver(row_[0]);
        }
        mysql_free_result(res_);

        string sender_id(row[1]);
        sql_command = "SELECT username FROM User WHERE user_id = " + sender_id;
        if (mysql_query(conn_, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn_));

            mysql_free_result(res);

            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            check_sql_sock_normal(sock_node_);
            release_sock_to_sql_pool(sock_node_);
            log(INFO, "OUT Obtain_request");
            return Status::CANCELLED;
        }
        res_ = mysql_use_result(conn_);
        if ((row_ = mysql_fetch_row(res_)) != NULL) {
            req.set_sender(row_[0]);
        }
        mysql_free_result(res_);

        req.set_request_id(row[0]);
        //req.set_sender(row[1]);
        //req.set_receiver(row[2]);
        req.set_type(row[3]);
        req.set_content(row[4]);

        req.set_request_date(row[6]);
        log(INFO, "one request");
        reply->Write(req);
    }
    release_sock_to_sql_pool(sock_node_);
    mysql_free_result(res);


    sql_command = "UPDATE User SET synchronism_request = 0 WHERE user_id = " + request->information();
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Obtain_request");
        return Status::OK;
    }


    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Obtain_request");
    return Status::OK;
}

Status GreeterServiceImpl::Obtain_requestLog (ServerContext* content, const Inf* request, ServerWriter<Request>* reply) {
    log(INFO, "IN Obtain_requestLog");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    SQL_SOCK_NODE* sock_node_ = get_sock_from_pool();
    MYSQL* conn_ = sock_node_->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL_RES *res_;
    MYSQL_ROW row_;
    string sql_command = "SELECT * FROM RequestLog WHERE (sender = " + request->information() +
        " AND ignore_sender = 'FALSE') OR (receiver = " + request->information() +
        " AND ignore_receiver = 'FALSE') order by requestLog_id desc";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Obtain_requestLog");
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL) {

        Request req;
        // convert user_id to username
        string receiver_id(row[2]);
        sql_command = "SELECT username FROM User WHERE user_id = " + receiver_id;
        if (mysql_query(conn_, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn_));

            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            check_sql_sock_normal(sock_node_);
            release_sock_to_sql_pool(sock_node_);
            log(INFO, "OUT Obtain_requestLog");
            return Status::CANCELLED;
        }
        res_ = mysql_use_result(conn_);
        if ((row_ = mysql_fetch_row(res_)) != NULL) {
            req.set_receiver(row_[0]);
        }
        mysql_free_result(res_);

        string sender_id(row[1]);
        sql_command = "SELECT username FROM User WHERE user_id = " + sender_id;
        if (mysql_query(conn_, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn_));

            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            check_sql_sock_normal(sock_node_);
            release_sock_to_sql_pool(sock_node_);
            log(INFO, "OUT Obtain_requestLog");
            return Status::CANCELLED;
        }
        res_ = mysql_use_result(conn_);
        if ((row_ = mysql_fetch_row(res_)) != NULL) {
            req.set_sender(row_[0]);
        }
        mysql_free_result(res_);

        req.set_request_id(row[0]);
        //req.set_sender(row[1]);
        //req.set_receiver(row[2]);
        req.set_type(row[3]);
        req.set_content(row[4]);
        req.set_response(row[5]);
        req.set_request_date(row[6]);
        req.set_response_date(row[7]);

        reply->Write(req);
    }
    release_sock_to_sql_pool(sock_node_);

    mysql_free_result(res);
    sql_command = "UPDATE User SET synchronism_request = 0 WHERE user_id = " + request->information();
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Obtain_requestLog");
        return Status::CANCELLED;
    }

    sql_command = "UPDATE RequestLog SET ignore_receiver = 'TRUE' WHERE receiver = " + request->information() + " AND ignore_receiver = 'FALSE'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Obtain_requestLog");
        return Status::CANCELLED;
    }

    sql_command = "UPDATE RequestLog SET ignore_sender = 'TRUE' WHERE sender = " + request->information() + " AND ignore_sender = 'FALSE'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR,  sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Obtain_requestLog");
        return Status::CANCELLED;
    }

    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Obtain_requestLog");
    return Status::OK;
}

Status GreeterServiceImpl::Obtain_requestLogHistory (ServerContext* content, const Inf* request, ServerWriter<Request>* reply) {
    log(INFO, "IN Obtain_requestLogHistory");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    SQL_SOCK_NODE* sock_node_ = get_sock_from_pool();
    MYSQL* conn_ = sock_node_->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;
    MYSQL_RES *res_;
    MYSQL_ROW row_;

    //string sql_command = "SELECT * FROM RequestLog WHERE sender = '" + request->information() + "' OR receiver = '" + request->information() + "' order by requestLog_id desc";
    string sql_command = "SELECT * FROM RequestLog WHERE receiver = " + request->information() + " order by requestLog_id desc";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        check_sql_sock_normal(sock_node_);
        release_sock_to_sql_pool(sock_node_);
        log(INFO, "OUT Obtain_requestLogHistory");
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL) {

        Request req;
        // convert user_id to username
        string receiver_id(row[2]);
        sql_command = "SELECT username FROM User WHERE user_id = " + receiver_id;
        if (mysql_query(conn_, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn_));

            mysql_free_result(res);

            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            check_sql_sock_normal(sock_node_);
            release_sock_to_sql_pool(sock_node_);
            log(INFO, "OUT Obtain_requestLogHistory");
            return Status::CANCELLED;
        }
        res_ = mysql_use_result(conn_);
        if ((row_ = mysql_fetch_row(res_)) != NULL) {
            req.set_receiver(row_[0]);
        }
        mysql_free_result(res_);

        string sender_id(row[1]);
        sql_command = "SELECT username FROM User WHERE user_id = " + sender_id;
        if (mysql_query(conn_, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn_));

            mysql_free_result(res);

            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            check_sql_sock_normal(sock_node_);
            release_sock_to_sql_pool(sock_node_);
            log(INFO, "OUT Obtain_requestLogHistory");
            return Status::CANCELLED;
        }
        res_ = mysql_use_result(conn_);
        if ((row_ = mysql_fetch_row(res_)) != NULL) {
            req.set_sender(row_[0]);
        }
        mysql_free_result(res_);
        req.set_request_id(row[0]);
        //req.set_sender(row[1]);
        //req.set_receiver(row[2]);
        req.set_type(row[3]);
        req.set_content(row[4]);
        req.set_response(row[5]);
        req.set_request_date(row[6]);
        req.set_response_date(row[7]);

        reply->Write(req);
    }
    release_sock_to_sql_pool(sock_node_);

    mysql_free_result(res);

    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Obtain_requestLogHistory");
    return Status::OK;
}

Status GreeterServiceImpl::Request_response (ServerContext* content, const Response* request, Inf* reply) {
    log(INFO, "IN Request_response");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;
    Request req;
    string type;

    string sql_command = "SELECT * FROM Request WHERE request_id = '" + request->request_id() + "'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, "ERROR Request_response SELECT process fail");
        log(ERROR, sql_command.data());
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    row = mysql_fetch_row(res);
    //char *t = row[3];
    type = row[3];
    req.set_request_id(row[0]);
    req.set_sender(row[1]);
    req.set_receiver(row[2]);
    req.set_type(row[3]);
    req.set_content(row[4]);
    req.set_response(request->response());
    req.set_request_date(row[6]);
    req.set_response_date(request->response_date());

    mysql_free_result(res);
    if (type == "payment" || type == "friendInvite")
    {
        sql_command = "INSERT INTO RequestLog (sender, receiver, type, content, response, request_date, response_date) VALUES (" +
            req.sender() + ", " + req.receiver() + ", '" + req.type() + "', '" + req.content() + "', '" + req.response() + "', '" +
            req.request_date() + "', '" + req.response_date() + "')";
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT Request_response");
            return Status::CANCELLED;
        }

        sql_command = "DELETE FROM Request WHERE request_id = '" + request->request_id() + "'";
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT Request_response");
            return Status::CANCELLED;
        }

        sql_command = "UPDATE User SET synchronism_request = 2, synchronism_delete = 1 WHERE user_id = " + req.sender() + " OR user_id = " + req.receiver();
        log(INFO, sql_command.data());
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT Request_response");
            return Status::CANCELLED;
        }

    } else if (type == "friendInvite")
    {

    }

    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Request_response");
    return Status::OK;
}

Status GreeterServiceImpl::MakePayment (ServerContext* content, ServerReader<BillPayment>* reader, Inf* reply) {
    log(INFO, "IN MakePayment");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;

    BillPayment request;

    while (reader->Read(&request)) {

        string sql_command = "UPDATE Bills SET paidStatus = '" + request.paidstatus() + "' WHERE bill_id = '" + request.bill_id() + "'";
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT MakePayment");
            return Status::CANCELLED;
        }
        //printf("in");
    }

    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT MakePayment");
    return Status::OK;
}

Status GreeterServiceImpl::Create_requestLog (ServerContext* content, const Request* request, Inf* reply) {
    log(INFO, "IN Create_requestLog");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    //MYSQL_RES *res;
    //MYSQL_ROW row;

    string sql_command = "INSERT INTO RequestLog (sender, receiver, type, content, response, request_date, response_date) VALUES (" +
        request->sender() + ", " + request->receiver() + ", '" + request->type() + "', '" + request->content() + "', '" +
        request->response() + "', '" + request->request_date() + "', '" + request->request_date() + "')";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Create_requestLog");
        return Status::CANCELLED;
    }

    sql_command = "UPDATE User SET synchronism_request = 2, synchronism_delete = 1 WHERE user_id = " + request->sender() +
        " OR user_id = " + request->receiver();

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Create_requestLog");
        return Status::CANCELLED;
    }


    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Create_requestLog");
    return Status::OK;
}

Status GreeterServiceImpl::IgnoreRequestLog (ServerContext* content, const IgnoreMessage* request, Inf* reply) {
    log(INFO, "IN IgnoreRequestLog");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    //MYSQL_RES *res;
    //MYSQL_ROW row;

    string sql_command = "UPDATE RequestLog SET ignore_" + request->user() + " = 'TRUE' WHERE requestLog_id = '" + request->requestlog_id() + "'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT IgnoreRequestLog");
        return Status::CANCELLED;
    }

    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT IgnoreRequestLog");
    return Status::OK;
}
