/*
 *
 *  opyright 2015, Google Inc.
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
 */

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
#include "MMGAPN/global.hpp"
#include "MMGAPN/MMGAPNSConnection.hpp"
#include "MMGAPN/MMGDevice.hpp"
#include "MMGAPN/MMGIOSPayload.hpp"
#include "MMGAPN/MMGTools.hpp"
#include <vector>
#include <cstdlib>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using grpc::ServerWriter;
using grpc::ServerReader;
using grpc::ServerReaderWriter;
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

class GreeterServiceImpl final : public Greeter::Service {
    Status SayHello(ServerContext* context, const HelloRequest* request,
            HelloReply* reply) override {
        std::string prefix("Hello ");
        reply->set_message(prefix + request->name());
        std::cout << "get request" << std::endl;
        return Status::OK;
    }

    Status User_inf(ServerContext* context, const Inf* request, User_detail* reply) override {
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
            sql_command = "SELECT username FROM User WHERE user_id = " + reply->friends_id(i);
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

    Status Login (ServerContext* context, const Login_m* request, Reply_inf* reply) override {
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

    Status Sign_up (ServerContext* context, const Sign_m* request, Reply_inf* reply) override {
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

    Status Search_username (ServerContext* context, const Inf* request, Repeated_string* reply) override {
        log(INFO, "IN Search");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;


        string sql_command = "SELECT username FROM User WHERE UPPER(username) like '%" + request->information() + "%'";

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
            reply->add_content(row[0]);
        }

        mysql_free_result(res);

        release_sock_to_sql_pool(sock_node);
        log(INFO, "OUT Search");
        return Status::OK;
    }

    Status Add_friend (ServerContext* context, const Repeated_string* request, Inf* reply) override {
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

    Status Delete_friend (ServerContext* context, const Repeated_string* request, Inf* reply) override {
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

    Status Syn (ServerContext* context, ServerReaderWriter<Syn_data, Inf>* stream) override {
        log(INFO, "Start SYN");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;

        //cout << "Start Syn" << endl;
        string sql_command;
        Inf request;
        Syn_data reply;

        while (stream->Read(&request)) {

            //            ostringstream ostr;
            //            ostr << i;
            //            string astr = ostr.str();

            if (request.information() == "")
                continue;

            sql_command = "SELECT synchronism_friend, synchronism_bill, synchronism_delete, synchronism_request FROM User WHERE user_id =" + request.information();
            if (!mysql_query(conn, sql_command.data())) {

                //cout << sql_command << endl;
                res = mysql_use_result(conn);
                if (res != NULL) {
                    row = mysql_fetch_row(res);
                    if (row != NULL) {
                        reply.set_friend_(row[0]);
                        reply.set_bill(row[1]);
                        reply.set_delete_(row[2]);
                        reply.set_request(row[3]);
                    } else {
                        log(WARNING, "row == NULL");
                        log(INFO, sql_command.data());

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



            //check write success or not
            if(!stream->Write(reply)) {

                release_sock_to_sql_pool(sock_node);
                return Status::OK;
            }

        }

        log(INFO, "SYN OUT");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Send_Img (ServerContext *context, ServerReader<Image>* reader, Inf* reply) override {
        log(INFO, "IN Send_Img");
        Image image_name;
        Image image_path;
        Image image;
        reader->Read(&image_name);

        // analyze name and path
        string imgName = image_name.data();

        reader->Read(&image_path);
        string path = image_path.data();

        log(INFO, imgName.data());
        log(INFO, path.data());

        // create image file
        FILE* fp;
        path = "./" + path + "/" + imgName + ".png";
        fp = fopen(path.data(),"w+");
        // get image
        reader->Read(&image);
        string str = image.data();
        const char* data = str.data();
        int count = fwrite(data, 1, str.length(),fp);
        log(INFO, to_string(count).data());
        int r = fclose(fp);

        if (r == EOF) {
            log(ERROR, "cannot close file handler");
        }

        log(INFO, "OUT Send_Img");
        reply->set_information("Get image");
        return Status::OK;
    }

    Status Create_share (ServerContext *context, const Share_inf* request, Inf* reply) override {
        log(INFO, "IN Create_share");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;

        string sql_command = "INSERT INTO Bills (creater, amount, type, paidBy, date, note, image, member_0, member_1, member_2, member_3, member_4, member_5, member_6, member_7, member_8, member_9, typeIcon) VALUES (" +
            request->creater() + ", " + request->amount() + " , '" + request->type() + "' , " +
            request->paidby() + " , '" + request->data() + "' , '" + request->note() + "' , '" +
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

    Status Delete_bill (ServerContext *context, const Share_inf* request, Inf* reply) override {
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


        sql_command = "UPDATE User SET synchronism_delete = 1 WHERE user_id = " + request->members(0) + " OR user_id = " +
            request->members(1) + " OR user_id = " + request->members(2) + " OR user_id = " + request->members(3) +
            " OR user_id = " + request->members(4) + " OR user_id = " + request->members(5) + " OR user_id = " +
            request->members(6) + " OR user_id = " + request->members(7) + " OR user_id = " + request->members(8) +
            " OR user_id = " + request->members(9);
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

    Status Receive_Img(ServerContext *context, const Repeated_string* request, ServerWriter<Image>* reply) override {
        log(INFO, "IN Receive_Img");
        Image image;

        for (int i = 1; i != request->content_size(); i++) {

            string path = "./" + request->content(0) + "/" + request->content(i) + ".png";

            FILE *fp = fopen(path.data(), "rb");
            //cout << i << endl;
            if (fp == NULL)
            {
                //fprintf(stderr, "cannot open image \n");
                //log(WARNING, "cannot open image");
                //log(WARNING, path.data());
                image.set_data("");


                //                for (int j = 0; j != request->content_size(); j++) {
                //                    //cout << "!" << request->content(j) << endl;
                //                }

                continue;
            }

            fseek(fp, 0, SEEK_END);

            if (ferror(fp)) {

                log(ERROR, "fseek() failed");
                int r = fclose(fp);

                if (r == EOF) {
                    log(ERROR, "cannot close file handler");
                }

                image.set_data("");
                continue;
            }

            int flen = ftell(fp);

            if (flen == -1) {

                perror("error occurred");
                int r = fclose(fp);

                if (r == EOF) {
                    log(ERROR, "cannot close file handlen");
                }

                image.set_data("");
                continue;
            }

            fseek(fp, 0, SEEK_SET);

            if (ferror(fp)) {

                log(ERROR, "fseek() failed");
                int r = fclose(fp);

                if (r == EOF) {
                    log(ERROR, "cannot close file handler");
                }

                image.set_data("");
                continue;
            }
            char *data = new char[flen + 1];

            fread(data, 1, flen, fp);
            data[flen] = '\0';


            if (ferror(fp)) {

                log(ERROR, "fread() failed");
                int r = fclose(fp);

                if (r == EOF) {
                    log(ERROR, "cannot close file handler");
                }

                image.set_data("");
                continue;
            }

            int r = fclose(fp);

            if (r == EOF) {
                log(ERROR, "cannot close file handler");
                continue;
            }

            string image_data(data, flen);
            image.set_data(image_data);
            image.set_name(request->content(i));
            reply->Write(image);
            delete []data;
        }

        log(INFO, "OUT Receive_Img");
        return Status::OK;
    }

    Status Obtain_bills (ServerContext *context, const Bill_request *request, ServerWriter<Share_inf> *reply) override {
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

    Status Reset_Status (ServerContext *context, const Inf* request, Inf* reply) override {
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

    // request system

    Status Send_request (ServerContext* content, const Request* request, Inf* reply) override {
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

    Status Obtain_request (ServerContext* content, const Inf* request, ServerWriter<Request>* reply) override {
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

    Status Obtain_requestLog (ServerContext* content, const Inf* request, ServerWriter<Request>* reply) override {
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


    Status Obtain_requestLogHistory (ServerContext* content, const Inf* request, ServerWriter<Request>* reply) override {
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

    Status Request_response (ServerContext* content, const Response* request, Inf* reply) override {
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

    Status MakePayment (ServerContext* content, ServerReader<BillPayment>* reader, Inf* reply) override {
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

    Status Create_requestLog (ServerContext* content, const Request* request, Inf* reply) override {
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

    Status IgnoreRequestLog (ServerContext* content, const IgnoreMessage* request, Inf* reply) override {
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

    Status Send_DeviceToken (ServerContext* content, const Repeated_string* request, Inf* reply) override {
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

    // setting

    Status Obtain_setting (ServerContext* content, const Inf* request, Setting* reply) override {
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

    Status Reset_setting (ServerContext* content, const Setting* request, Inf* reply) override {
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


    Status Reset_userInfo (ServerContext* content, const UserInfo* request, Inf* reply) override {
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


    Status Obtain_userInfo (ServerContext* content, const Inf* request, UserInfo* reply) override {
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
};

void RunServer() {
    std::string server_address("0.0.0.0:50052");
    GreeterServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    log(INFO, "Server Start");
    //printf("Server Start\n");
    //std::cout << "Server listening on : " << server_address << std::endl;
    string outInf = "Server listening on : " + server_address;
    log(INFO, outInf.data());
    server->Wait();
}



bool pushNotificationToDevice (string deviceToken, string message) {
    SSL_load_error_strings();
    SSL_library_init();

    // Get a list of devices
    std::vector<MMGDevice*> devices;
    //get_devices_list(devices);
    MMGDevice* device1 = new MMGDevice(deviceToken.data(), 1);
    devices.push_back(device1);

    // Create a payload object
    MMGIOSPayload payload(message.data(), "Slider label", 1, "sound.caf");

    // Create the APNS connection, empty string if no password for the private key
    MMGAPNSConnection connection(MMG_APNS_CA_PATH, MMG_APNS_CERT_PATH, MMG_APNS_PRIVATEKEY_PATH, "gfxcc", true);
    // Open the connection
    if (connection.OpenConnection() != MMGConnectionError::MMGNoError)
        return EXIT_FAILURE;

    // Send the payload
    uint32_t notifId = 1;
    for (MMGDevice* device : devices)
    {
        // Update payload badge number to reflect device's one
        payload.SetBadgeNumber(device->GetBadge());
        // Send payload to the device
        connection.SendPayloadToDevice(payload, *device, notifId++);
    }

    // Free up memory
    for (MMGDevice* device : devices)
        delete device;

    // Close the connection
    connection.CloseConnection();


    return true;
}


SQL_SOCK* Create_sock(char* db_host, char* db_user, char* db_passwd, char* db_name, unsigned short port) {
    SQL_SOCK* new_sock = NULL;
    new_sock = (SQL_SOCK *)malloc(sizeof(SQL_SOCK));

    // check malloc success or not
    if (NULL == new_sock) {
        cout << "malloc fail when create a new conn" << endl;
        return NULL;
    }

    MYSQL * sock = mysql_init(NULL);
    if (!mysql_real_connect(sock, db_host,
                db_user, db_passwd, db_name, 0, 0, 0)) {
        cout << "mysql_real_connect fail" << endl;
        //cout << db_host << endl << db_user << endl << db_passwd << endl << db_name << endl;
        return NULL;
    }
    new_sock->sock = sock;

    return new_sock;
}

void Close_sock(SQL_SOCK* sql_sock) {
    mysql_close(sql_sock->sock);
}

/*
   void *mySqlHeartBeat(void *ptr)
   {

   while (1)
   {
   for (int i = 0; i != CONN_NUM; i++) {
   SQL_SOCK_NODE* sock_node = get_sock_from_pool();
   MYSQL* conn = sock_node->sql_sock->sock;
   string sql_command = "SELECT * FROM User";
   if (mysql_query(conn, sql_command.data())) {
   printf("%s\n", sql_command.data());
   }
   release_sock_to_sql_pool(sock_node);
   }

   sleep(18000);
   }
   return 0;
   }
   */
    int main(int argc, char** argv) {


        //    mysql_init( &mysql );
        //    conn = mysql_real_connect(  &mysql, "caoyongs-MacBook-Pro.local", "gfxcc", "19920406Cy", "iShare_data", 0, 0, 0 );
        //    if( !conn )
        //    {
        //        cout << "Couldn't connect to MySQL database server!\n" << endl;
        //        cout << "Error: %s\n" << mysql_error( &mysql ) << endl;
        //        return 1;
        //    }
        //
        //
        //

        /*

        // SLL init only once
        SSL_load_error_strings();
        SSL_library_init();

        // Get a list of devices
        std::vector<MMGDevice*> devices;
        //get_devices_list(devices);
        MMGDevice* device1 = new MMGDevice("17a612c5fe84f544ebd0c6aa880a0955ca00084a4488dd633d113ef379292f48", 1);
        devices.push_back(device1);

        // Create a payload object
        MMGIOSPayload payload("Push message", "Slider label", 1, "sound.caf");

        // Create the APNS connection, empty string if no password for the private key
        MMGAPNSConnection connection(MMG_APNS_CA_PATH, MMG_APNS_CERT_PATH, MMG_APNS_PRIVATEKEY_PATH, "gfxcc", true);
        // Open the connection
        if (connection.OpenConnection() != MMGConnectionError::MMGNoError)
        return EXIT_FAILURE;

        // Send the payload
        uint32_t notifId = 1;
        for (MMGDevice* device : devices)
        {
        // Update payload badge number to reflect device's one
        payload.SetBadgeNumber(device->GetBadge());
    // Send payload to the device
    connection.SendPayloadToDevice(payload, *device, notifId++);
    }

        // Free up memory
        for (MMGDevice* device : devices)
        delete device;

        // Close the connection
        connection.CloseConnection();

*/

        const char* hostname = "localhost";
        const char* username = "root";
        const char* passwd = "19920406Cy";
        const char* db = "iShare_server";
        sql_pool_create(CONN_NUM, hostname, username,
                passwd, db, 3306, NULL, Create_sock, Close_sock);
        /*
           pthread_t id;
           int err = pthread_create(&id, NULL, mySqlHeartBeat, NULL);
           if (err != 0)
           {
           printf("HeatBeat create Fail!\n");
           printf("%s\n",strerror(err));
           }*/
        RunServer();
        return 0;
    }

