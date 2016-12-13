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
#include <grpc++/security/server_credentials.h>
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
using helloworld::Search_result;
using namespace std;

bool pushNotificationToDevice (string deviceToken, string message);

// request system

Status GreeterServiceImpl::User_inf (ServerContext* context, const Inf* request, User_detail* reply) {
    log(INFO, "IN User_inf");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;
    //send sql request
    string sql_command = "SELECT * FROM Friends WHERE user1_id = '" + request->information() + "' OR user2_id = '" + request->information() + "'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, mysql_error(conn));
        log(ERROR, sql_command.data());
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL) {
        //            cout << row[0] << ends << row[1] << endl;
        //            reply->set_username(request->information());
        //            reply->add_friends(row[1]);

        string name = row[0];
        if (name == request->information()) {
            reply->add_friends_id(row[1]);
        } else {
            reply->add_friends_id(row[0]);
        }
    }
    mysql_free_result(res);
    //cout << "obtain" << endl;

    // get friends_id
    for (int i = 0; i != reply->friends_id_size(); i++) {
        sql_command = "SELECT username,last_modified FROM User WHERE user_id = " + reply->friends_id(i);
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT User_inf");
            return Status::CANCELLED;
        }
        res = mysql_use_result(conn);
        if ((row = mysql_fetch_row(res)) != NULL) {
            reply->add_friends_name(row[0]);
            reply->add_friends_lastmodified(row[1]);
        }
        mysql_free_result(res);
    }

    // get personal information
    sql_command = "SELECT username, email FROM User WHERE user_id = " + request->information();
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT User_inf");
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    if ((row = mysql_fetch_row(res)) != NULL) {
        reply->set_username(row[0]);
        reply->set_email(row[1]);
    }
    mysql_free_result(res);

    //set syn flag = 0
    sql_command = "UPDATE User SET synchronism_friend = 0 WHERE user_id = " + request->information();
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT User_inf");
        return Status::CANCELLED;
    }

    log(INFO, "OUT User_inf");
    release_sock_to_sql_pool(sock_node);
    return Status::OK;
}

Status GreeterServiceImpl::Login (ServerContext* context, const Login_m* request, Reply_inf* reply) {
    log(INFO, "IN Login");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;

    //string sql_command = "SELECT * FROM User WHERE binary username = '" + request->username() + "' AND binary password = '" + request->password() + "'";
    string sql_command = "SELECT password FROM User WHERE binary username = '" + request->username() + "'";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);

        reply->set_status("CANCELLED");
        reply->set_information("Sorry, please try again");
        log(INFO, "OUT Login");
        return Status::OK;
    }
    res = mysql_use_result(conn);
    row = mysql_fetch_row(res);

    if (res->row_count == 0) {
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);

        reply->set_status("CANCELLED");
        reply->set_information("Sorry, this username does not exist.");
        log(INFO, "OUT Login");
        return Status::OK;
    }

    // check password
    if (row[0] != request->password()) {
        reply->set_status("CANCELLED");
        reply->set_information("password incorrect");
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Login");
        return Status::OK;
    }
    mysql_free_result(res);

    // get user_id
    sql_command = "SELECT user_id FROM User WHERE username = '" + request->username() + "'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);

        reply->set_status("CANCELLED");
        reply->set_information("Sorry, please try again");
        log(INFO, "OUT Login");
        return Status::OK;
    }
    res = mysql_use_result(conn);
    if ((row = mysql_fetch_row(res)) != NULL) {
        reply->set_information(row[0]);
    }
    mysql_free_result(res);

    reply->set_status("OK");
    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Login");
    return Status::OK;
}

Status GreeterServiceImpl::Sign_up (ServerContext* context, const Sign_m* request, Reply_inf* reply) {
    log(INFO, "IN Sign_up");

    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;

    string sql_command = "SELECT user_id FROM User WHERE username = '" + request->username() + "'";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, "check username fail");
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);

        reply->set_status("CANCELLED");
        reply->set_information("Sorry, please try again.");
        log(INFO, "OUT Sign_up");
        return Status::OK;
    }
    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL) {
    }

    // usename has been used
    if (res->row_count != 0) {
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);

        reply->set_status("CANCELLED");
        reply->set_information("Sorry, your username has beed used. Please change username.");
        log(INFO,"OUT Sign_up");
        return Status::OK;
    }
    mysql_free_result(res);

    // insert record into User table
    sql_command = "INSERT INTO User (username, password, email) VALUES ('" +
        request->username() + "', '" + request->password() + "', '" +
        request->email() + "')";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, "insert userInf fail");
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        reply->set_status("CANCELLED");
        reply->set_information("Sorry, please create this account again.");

        log(INFO,"OUT Sign_up");
        return Status::OK;
    }
    // get user_id
    sql_command = "SELECT user_id FROM User WHERE username = '" + request->username() + "'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, "get user_id fail");
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        reply->set_status("CANCELLED");
        reply->set_information("Sorry, please reopen this app.");

        log(INFO,"OUT Sign_up");
        return Status::OK;
    }
    string user_id;
    res = mysql_use_result(conn);
    if ((row = mysql_fetch_row(res)) != NULL) {
        user_id = row[0];
    }
    reply->set_status("OK");
    reply->set_information(user_id);
    log(INFO,"OUT Sign_up");
    release_sock_to_sql_pool(sock_node);
    return Status::OK;
}

Status GreeterServiceImpl::Search_username (ServerContext* context, const Inf* request, Search_result* reply) {
    log(INFO, "IN Search");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;


    string sql_command = "SELECT username, user_id FROM User WHERE UPPER(username) like UPPER('%" + request->information() + "%')";
    log(INFO, sql_command.data());
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL) {
        reply->add_username(row[0]);
        reply->add_user_id(row[1]);
    }
    mysql_free_result(res);

    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Search");
    return Status::OK;
}

Status GreeterServiceImpl::Add_friend (ServerContext* context, const Repeated_string* request, Inf* reply) {
    log(INFO, "IN Add_friend");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;

    // get user_id
    string user1_id, user2_id;
    string sql_command = "SELECT user_id FROM User WHERE username = '" + request->content(0) + "' OR username = '" + request->content(1) + "'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);

        log(INFO, "OUT Add_friend");
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    if ((row = mysql_fetch_row(res)) != NULL) {
        user1_id = row[0];
    }
    if ((row = mysql_fetch_row(res)) != NULL) {
        user2_id = row[0];
    }
    mysql_free_result(res);

    // check for relation exist or not
    sql_command = "SELECT * FROM Friends where (user1_id = " + user1_id + " and user2_id = " + user2_id + ") OR (user1_id = " + user2_id + " and user2_id = " + user1_id + ")";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Add_friend");
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL) {
    }
    if (res->row_count != 0) {
        reply->set_information("Already be friends");
        release_sock_to_sql_pool(sock_node);

        log(INFO, "OUT Add_friend");
        return Status::OK;
    }
    mysql_free_result(res);

    // start to add friend
    sql_command = "INSERT INTO Friends (user1_id, user2_id) VALUES (" + user1_id + ", " + user2_id + ")";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);

        log(INFO, "OUT Add_friend");
        return Status::CANCELLED;
    }

    //
    sql_command = "UPDATE User SET synchronism_friend = 1 WHERE user_id = " + user1_id + " OR user_id = " + user2_id;
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));

        reply->set_information("Sorry, please again later.");
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);

        log(INFO, "OUT Add_friend");
        return Status::OK;
    }


    reply->set_information("OK");
    log(INFO, "OUT Add_friend");
    release_sock_to_sql_pool(sock_node);
    return Status::OK;
}

Status GreeterServiceImpl::Delete_friend (ServerContext* context, const Repeated_string* request, Inf* reply) {
    log(INFO, "IN Delete_friend");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;

    string sql_command = "DELETE FROM Friends WHERE (user1_id = " + request->content(0) + " and user2_id = " +
        request->content(1) + ") OR (user1_id = " + request->content(1) + " and user2_id = " + request->content(0) + ")";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));

        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Delete_friend");
        return Status::CANCELLED;
    }
    // delete result check
    if(mysql_affected_rows(conn) != 1) {
        log(ERROR, "error happend during delete process");
    }

    sql_command = "UPDATE User SET synchronism_friend = 1, synchronism_delete = 1 WHERE user_id = " + request->content(0) + " OR user_id = " + request->content(1);
    if (mysql_query(conn, sql_command.data())) {

        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        reply->set_information("UPDATE User WRONG");
        check_sql_sock_normal(sock_node);

        log(INFO, "OUT Delete_friend");
        return Status::OK;
    }

    reply->set_information("OK");
    log(INFO, "OUT Delete_friend");
    release_sock_to_sql_pool(sock_node);
    return Status::OK;
}



Status GreeterServiceImpl::Reset_Status (ServerContext *context, const Inf* request, Inf* reply) {
    log(INFO, "IN Reset_Status");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    //MYSQL_RES *res;
    //MYSQL_ROW row;

    string sql_command = "UPDATE User SET synchronism_delete = 0 WHERE user_id = '" + request->information() + "'";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
    }
    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Reset_Status");
    return Status::OK;
}

Status GreeterServiceImpl::Update_user_lastModified (ServerContext *context, const Inf* request, Inf* reply) {
    log(INFO, "IN Update_user_lastModified");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;

    string sql_command = "UPDATE User SET last_modified = NOW() WHERE user_id = '" + request->information() + "'";

    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
    }

    // update synchronism_friend
    vector<string> friend_id_list;
    sql_command = "SELECT * FROM Friends WHERE user1_id = '" + request->information() + "' OR user2_id = '" + request->information() + "'";
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, mysql_error(conn));
        log(ERROR, sql_command.data());
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        return Status::CANCELLED;
    }
    res = mysql_use_result(conn);
    while ((row = mysql_fetch_row(res)) != NULL) {
        //            cout << row[0] << ends << row[1] << endl;
        //            reply->set_username(request->information());
        //            reply->add_friends(row[1]);

        string user_id = row[0];
        if (user_id == request->information()) {
            friend_id_list.push_back(row[1]);
        } else {
            friend_id_list.push_back(row[0]);
        }
    }
    mysql_free_result(res);

    for (unsigned int i = 0; i != friend_id_list.size(); i++) {
        sql_command = "UPDATE User SET synchronism_friend = 1 WHERE user_id = '" + friend_id_list[i] + "'";
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, mysql_error(conn));
            log(ERROR, sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }
    }


    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Update_user_lastModified");

    return Status::OK;
}

Status GreeterServiceImpl::Send_DeviceToken (ServerContext* content, const Repeated_string* request, Inf* reply) {
    log(INFO, "IN Send_DeviceToken");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;

    string sql_command = "UPDATE User SET deviceToken = '" + request->content(1) + "' WHERE user_id = " + request->content(0);
    log(INFO, request->content(1).data());
    if (mysql_query(conn, sql_command.data())) {
        log(ERROR, sql_command.data());
        log(ERROR, mysql_error(conn));
        check_sql_sock_normal(sock_node);
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Send_DeviceToken");
        return Status::CANCELLED;
    }

    release_sock_to_sql_pool(sock_node);
    log(INFO, "OUT Send_DeviceToken");
    return Status::OK;
}

