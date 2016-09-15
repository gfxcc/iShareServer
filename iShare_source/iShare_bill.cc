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


    Status GreeterServiceImpl::Obtain_bills (ServerContext *context, const Bill_request *request, ServerWriter<Share_inf> *reply) {
        log(INFO, "IN Obtain_bills");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;
        string sql_command;
        if (request->amount() == "all")
        {
            sql_command = "SELECT * FROM Bills WHERE member_0 = " + request->username() + " OR member_1 = " +
                request->username() + " OR member_2 = " + request->username() + " OR member_3 = " + request->username() +
                " OR member_4 = " + request->username() + " OR member_5 = " + request->username() + " OR member_6 = " +
                request->username() + " OR member_7 = " + request->username() + " OR member_8 = " + request->username() +
                " OR member_9 = " + request->username() + " OR paidBy = " + request->username() + " order by bill_id desc";
        } else  {
            sql_command = "SELECT * FROM Bills WHERE member_0 = " + request->username() +
                " OR member_1 = " + request->username() + " OR member_2 = " +
                request->username() + " OR member_3 = " + request->username() +
                " OR member_4 = " + request->username() + " OR member_5 = " +
                request->username() + " OR member_6 = " + request->username() +
                " OR member_7 = " + request->username() + " OR member_8 = " +
                request->username() + " OR member_9 = " + request->username() +
                " OR paidBy = " + request->username() + " order by bill_id desc LIMIT " +
                request->amount();
        }

        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }
        res = mysql_use_result(conn);

        while ((row = mysql_fetch_row(res)) != NULL) {
            Share_inf bill;
            bill.set_bill_id(row[0]);
            bill.set_creater(row[1]);
            bill.set_amount(row[2]);
            bill.set_type(row[3]);
            bill.set_data(row[5]);
            bill.set_note(row[6]);
            if (row[7])
                bill.set_image(row[7]);
            else
                bill.set_image("");
            bill.set_paidby(row[8]);
            if (row[9])
                bill.add_members(row[9]);
            else
                bill.add_members("");
            if (row[10])
                bill.add_members(row[10]);
            else
                bill.add_members("");
            if (row[11])
                bill.add_members(row[11]);
            else
                bill.add_members("");
            if (row[12])
                bill.add_members(row[12]);
            else
                bill.add_members("");
            if (row[13])
                bill.add_members(row[13]);
            else
                bill.add_members("");
            if (row[14])
                bill.add_members(row[14]);
            else
                bill.add_members("");
            if (row[15])
                bill.add_members(row[15]);
            else
                bill.add_members("");
            if (row[16])
                bill.add_members(row[16]);
            else
                bill.add_members("");
            if (row[17])
                bill.add_members(row[17]);
            else
                bill.add_members("");
            if (row[18])
                bill.add_members(row[18]);
            else
                bill.add_members("");
            bill.set_paidstatus(row[19]);
            if (row[20]) {
                bill.set_typeicon(row[20]);
            } else {
                bill.set_typeicon("");
            }
            //            //cout << "amout " << row[2] << endl;
            //printf("one result\n");
            reply->Write(bill);
        }
        mysql_free_result(res);
        sql_command = "UPDATE User SET synchronism_bill = 0 WHERE user_id = " + request->username();
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT Obtain_bills");
            return Status::OK;
        }
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Obtain_bills");
        return Status::OK;
    }

    Status GreeterServiceImpl::Delete_bill (ServerContext *context, const Share_inf* request, Inf* reply) {
        log(INFO, "IN Delete_bill");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        //MYSQL_RES *res;
        //MYSQL_ROW row;

        string sql_command = "DELETE FROM Bills WHERE bill_id = " + request->bill_id();
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));
            //printf("%s\n", );
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT Delete_bill");
            return Status::CANCELLED;
        }


        sql_command = "UPDATE User SET synchronism_delete = 1 WHERE user_id = '" + request->members(0) + "' OR user_id = '" +
            request->members(1) + "' OR user_id = '" + request->members(2) + "' OR user_id = '" + request->members(3) +
            "' OR user_id = '" + request->members(4) + "' OR user_id = '" + request->members(5) + "' OR user_id = '" +
            request->members(6) + "' OR user_id = '" + request->members(7) + "' OR user_id = '" + request->members(8) +
            "' OR user_id = '" + request->members(9) + "'";
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, mysql_error(conn));
            log(ERROR, sql_command.data());

            reply->set_information("delete update process fail");
            //printf("%s\n", );
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT Delete_bill");
            return Status::OK;
        }

        // push notification
        // push notification
        /*
           sql_command = "SELECT deviceToken FROM User WHERE (username = '" + request->members(0) +
           "' OR username = '" + request->members(1) + "' OR username = '" + request->members(2) +
           "' OR username = '" + request->members(3) + "' OR username = '" + request->members(4) +
           "' OR username = '" + request->members(5) + "' OR username = '" + request->members(6) +
           "' OR username = '" + request->members(7) + "' OR username = '" + request->members(8) +
           "' OR username = '" + request->members(9) + "') AND N_newBill = 1";
           log(INFO, sql_command.data());
           if (mysql_query(conn, sql_command.data())) {
           log(ERROR, "error in Create_share process 3");
           log(ERROR, sql_command.data());

           reply->set_information("push notification fail");
            //printf("%s\n", );
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
            }
            res = mysql_use_result(conn);
            string message = request->creater() + " share a " + request->amount() + "$ bill with you.";
            while ((row = mysql_fetch_row(res)) != NULL) {
            if (!row[0]) {
            continue;
            }
            log(INFO, row[0]);
            pushNotificationToDevice(row[0], message);
            }
            mysql_free_result(res);
            */

        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Delete_bill");
        return Status::OK;
    }

    Status GreeterServiceImpl::Create_share (ServerContext *context, const Share_inf* request, Inf* reply) {
        log(INFO, "IN Create_share");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;

        string note_str = request->note();
        for (unsigned int i = 0; i < note_str.length(); i++) {
            if (note_str[i] == '\n') {
                note_str[i] = ' ';
            }
        }

        string sql_command = "INSERT INTO Bills (creater, amount, type, paidBy, date, note, image, member_0, member_1, member_2, member_3, member_4, member_5, member_6, member_7, member_8, member_9, typeIcon) VALUES (" +
            request->creater() + ", " + request->amount() + " , '" + request->type() + "' , " +
            request->paidby() + " , '" + request->data() + "' , '" + note_str + "' , '" +
            request->image() + "' , " + request->members(0) + " , " + request->members(1) + " , " +
            request->members(2) + " , " + request->members(3) + " , " + request->members(4) +
            " , " + request->members(5) + " , " + request->members(6) + " , " +
            request->members(7) + " , " + request->members(8) + " , " + request->members(9) + ", '" +
            request->typeicon() + "')";

        log(INFO, sql_command.data());

        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));

            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT Create_share");
            return Status::CANCELLED;
        }

        // update synchronism_bill
        sql_command = "UPDATE User SET synchronism_bill = 1 WHERE user_id = " + request->members(0) +
            " OR user_id = " + request->members(1) + " OR user_id = " + request->members(2) +
            " OR user_id = " + request->members(3) + " OR user_id = " + request->members(4) +
            " OR user_id = " + request->members(5) + " OR user_id = " + request->members(6) +
            " OR user_id = " + request->members(7) + " OR user_id = " + request->members(8) +
            " OR user_id = " + request->members(9);

        log(INFO, sql_command.data());
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));

            reply->set_information("OK");
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT Create_share");
            return Status::OK;
        }

        // get creater username
        string creater_username;
        sql_command = "SELECT username FROM User WHERE user_id =" + request->creater();
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));

            reply->set_information("OK");
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT Create_share");
            return Status::OK;
        }
        res = mysql_use_result(conn);
        if ((row = mysql_fetch_row(res)) != NULL) {
            creater_username = row[0];
        }
        mysql_free_result(res);

        // push notification
        sql_command = "SELECT deviceToken FROM User WHERE (user_id = " + request->members(0) +
            " OR user_id = " + request->members(1) + " OR user_id = " + request->members(2) +
            " OR user_id = " + request->members(3) + " OR user_id = " + request->members(4) +
            " OR user_id = " + request->members(5) + " OR user_id = " + request->members(6) +
            " OR user_id = " + request->members(7) + " OR user_id = " + request->members(8) +
            " OR user_id = " + request->members(9) + ") AND N_newBill = 1";
        log(INFO, sql_command.data());
        if (mysql_query(conn, sql_command.data())) {
            log(ERROR, sql_command.data());
            log(ERROR, mysql_error(conn));

            reply->set_information("OK");
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            log(INFO, "OUT Create_share");
            return Status::OK;
        }
        res = mysql_use_result(conn);
        string message = creater_username + " share a " + request->amount() + "$ bill with you.";
        while ((row = mysql_fetch_row(res)) != NULL) {
            if (!row[0]) {
                continue;
            }
            log(INFO, row[0]);
            pushNotificationToDevice(row[0], message);
        }
        mysql_free_result(res);

        reply->set_information("OK");
        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Create_share");
        return Status::OK;
    }
