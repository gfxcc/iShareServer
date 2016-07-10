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

Status GreeterServiceImpl::Obtain_setting (ServerContext* content, const Inf* request, Setting* reply) {
    log(INFO, "IN Obtain_setting");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;
    string sql_command = "SELECT N_friendInvite, N_newBill, N_editedDeleteBill, N_commentBill, N_paidNotice FROM User WHERE user_id = " + request->information();

    log(INFO, sql_command.data());

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    if ((row = mysql_fetch_row(res)) != NULL) {
        reply->set_friendinvite(atoi(row[0]));
        reply->set_newbill(atoi(row[1]));
        reply->set_editeddeletebill(atoi(row[2]));
        reply->set_commentbill(atoi(row[3]));
        reply->set_paidnotice(atoi(row[4]));
    }

    mysql_free_result(res);
    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Obtain_setting");
    return Status::OK;
}

Status GreeterServiceImpl::Reset_setting (ServerContext* content, const Setting* request, Inf* reply) {
    log(INFO, "IN Reset_setting");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;

    string v1 = to_string(request->friendinvite());
    string v2 = to_string(request->newbill());
    string v3 = to_string(request->editeddeletebill());
    string v4 = to_string(request->commentbill());
    string v5 = to_string(request->paidnotice());
    string sql_command = "UPDATE User SET N_friendInvite = " + v1 + ", N_newBill = " + v2 +
        ", N_editedDeleteBill = " + v3 + ", N_commentBill = " + v4 + ", N_paidNotice = " +
        v5 + " WHERE user_id = " + request->user_id();
    log(INFO, sql_command.data());
    if (mysql_query(conn, sql_command.data())) {
        //printf("error %s\n", mysql_error(conn));
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Reset_setting");
        return Status::CANCELLED;
    }


    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Reset_setting");
    return Status::OK;
}

Status GreeterServiceImpl::Reset_userInfo (ServerContext* content, const UserInfo* request, Inf* reply) {
    log(INFO, "Reset_userInfo IN");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;

    bool username_changed = true;
    string original_username;
    // check username is avaliable or not
    string sql_command = "SELECT user_id FROM User WHERE username = '" + request->username() + "'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "Reset_userInfo OUT");
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    if ((row = mysql_fetch_row(res)) != NULL) {
        if (string(row[0]) != request->user_id()) {
            mysql_free_result(res);
            reply->set_information("Sorry, this username has been used. Please try another new one.");
            release_sock_to_sql_pool(sock_node);
            mysql_free_result(res);
            log(INFO, "Reset_userInfo OUT");
            return Status::OK;
        } else {
            username_changed = false;
        }
    }
    mysql_free_result(res);

    // get original username if need
    if (username_changed) {
        sql_command = "SELECT username FROM User WHERE user_id =" + request->user_id();
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            mysql_free_result(res);
            log(INFO, "Reset_userInfo OUT");
            return Status::CANCELLED;
        }
        res = mysql_use_result(conn);
        if ((row = mysql_fetch_row(res)) != NULL) {
            original_username = row[0];
        }
        mysql_free_result(res);
    }

    sql_command = "UPDATE User SET password = '" + request->password() + "', email = '" +
        request->email() + "', currency = " + to_string(request->currency()) + ", username = '" +
        request->username() + "' WHERE user_id = " + request->user_id();

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "Reset_userInfo OUT");
        return Status::CANCELLED;
    }

    reply->set_information("OK");

    // if username changed, notice every friends
    if (username_changed) {
        vector<string> friends_id;
        string message = "Your friend " + original_username + " changed username to " + request->username();

        sql_command = "SELECT * FROM Friends WHERE user1_id = '" + request->user_id() + "' OR user2_id = '" + request->user_id() + "'";
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, mysql_error(conn));
            log(ERROR, sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::OK;
        }
        res = mysql_use_result(conn);
        while ((row = mysql_fetch_row(res)) != NULL) {
            //            cout << row[0] << ends << row[1] << endl;
            //            reply->set_username(request->information());
            //            reply->add_friends(row[1]);

            string user_id = row[0];
            if (user_id == request->user_id()) {
                friends_id.push_back(row[1]);
            } else {
                friends_id.push_back(row[0]);
            }
        }
        mysql_free_result(res);

        for (unsigned int i = 0; i != friends_id.size(); i++) {
            sql_command = "UPDATE User SET synchronism_friend = 1 WHERE user_id = " + friends_id[i];
            if (mysql_query(conn, sql_command.data())) {
                log(ERROR, mysql_error(conn));
                log(ERROR, sql_command.data());
                check_sql_sock_normal(sock_node);
                release_sock_to_sql_pool(sock_node);
                return Status::OK;
            }
        }

        for (unsigned int i = 0; i != friends_id.size(); i++) {
            sql_command = "SELECT deviceToken FROM User WHERE user_id = " + friends_id[i];
            if (mysql_query(conn, sql_command.data())) {
                log(ERROR, mysql_error(conn));
                log(ERROR, sql_command.data());
                check_sql_sock_normal(sock_node);
                release_sock_to_sql_pool(sock_node);
                return Status::OK;
            }
            res = mysql_use_result(conn);
            if ((row = mysql_fetch_row(res)) != NULL) {
                pushNotificationToDevice(row[0], message);
            }
            mysql_free_result(res);
        }
    }




    release_sock_to_sql_pool(sock_node);
    log(INFO, "Reset_userInfo OUT");
    return Status::OK;
}

Status GreeterServiceImpl::Obtain_userInfo (ServerContext* content, const Inf* request, UserInfo* reply) {
    log(INFO, "IN Obtain_userInfo");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;

    string sql_command = "SELECT password, email, currency, username FROM User WHERE user_id = " +
        request->information();
    log(INFO, sql_command.data());
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Obatin_userInfo");
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    if((row = mysql_fetch_row(res)) != NULL) {
        reply->set_password(row[0]);
        reply->set_email(row[1]);
        reply->set_currency(atoi(row[2]));
        reply->set_username(row[3]);

    }

    mysql_free_result(res);
    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Obatin_userInfo");
    return Status::OK;
}
