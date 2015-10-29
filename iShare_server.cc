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
 *
 */

#include <iostream>
#include <memory>
#include <string>
#include <list>

#include <mysql.h>
#include <grpc/grpc.h>
#include <grpc++/server.h>
#include <grpc++/server_builder.h>
#include <grpc++/server_context.h>
#include <grpc++/server_credentials.h>
#include <grpc++/support/status.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sstream>
#include "iShare.grpc.pb.h"
#include "mysql_pool.h"

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
using namespace std;

#define CONN_NUM 10

string convertToString(double d);
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
        printf("*************User_inf IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;
        //send sql request
        string sql_command = "SELECT * FROM Friends WHERE username_1 = '" + request->information() + "' OR username_2 = '" + request->information() + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("Obtain user_inf fail\n");
            printf("%s\n", sql_command.data());
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
                reply->add_friends(row[1]);
            } else {
                reply->add_friends(row[0]);
            }
        }

        //cout << "obtain" << endl;

        //set syn flag = 0
        sql_command = "UPDATE User SET synchronism_friend = 0 WHERE username = '" + request->information() + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("Change syn flag = 0 fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;

        }

        printf("*************User_inf OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Login (ServerContext* context, const Login_m* request, Inf* reply) override {
        printf("*************Login IN*************\n");

        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;

        string sql_command = "SELECT * FROM User WHERE binary username = '" + request->username() + "' AND binary password = '" + request->password() + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("Login fail\n");
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }
        res = mysql_use_result(conn);
        while ((row = mysql_fetch_row(res)) != NULL) {
        }
        if (res->row_count == 0) {
            reply->set_information("WRONG");
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        reply->set_information("OK");

        printf("*************Login OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;

    }

    Status Sign_up (ServerContext* context, const Sign_m* request, Inf* reply) override {
        printf("*************Sign_up IN*************\n");

        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;

        string sql_command = "SELECT user_id FROM User WHERE username = '" + request->username() + "'";

        if (mysql_query(conn, sql_command.data())) {
            printf("error %s\n", mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }
        res = mysql_use_result(conn);
        while ((row = mysql_fetch_row(res)) != NULL) {
        }

        // usename used
        if (res->row_count != 0) {
            reply->set_information("WRONG");
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        // insert record into User table
        mysql_free_result(res);
        sql_command = "INSERT INTO User (username, password) VALUES ('" + request->username() + "', '" + request->password() + "')";

        if (mysql_query(conn, sql_command.data())) {
            printf("error2 %s\n", mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        reply->set_information("OK");

        printf("*************Sign_up OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Search_username (ServerContext* context, const Inf* request, Repeated_string* reply) override {
        printf("*************Search IN*************\n");

        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;


        string sql_command = "SELECT username FROM User WHERE username like '%" + request->information() + "%'";
        if (mysql_query(conn, sql_command.data())) {
            printf("%s\n", sql_command.data());
            printf("error %s\n", mysql_error(conn));
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }
        res = mysql_use_result(conn);
        while ((row = mysql_fetch_row(res)) != NULL) {
            reply->add_content(row[0]);
        }
        printf("%s\n", sql_command.data());

        printf("*************Search OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Add_friend (ServerContext* context, const Repeated_string* request, Inf* reply) override {
        printf("*************Add_friend IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;

        string sql_command = "SELECT * FROM iShare_data.Friends where (username_1 = '" + request->content(0) + "' and username_2 = '" + request->content(1) + "') or (username_1 = '" + request->content(1) + "' and username_2 = '" +request->content(0) + "')";
        if (mysql_query(conn, sql_command.data())) {
            printf("%s\n", mysql_error(conn));
            check_sql_sock_normal(sock_node);

        }
        res = mysql_use_result(conn);
        while ((row = mysql_fetch_row(res)) != NULL) {
        }
        if (res->row_count != 0) {
            reply->set_information("Already be friends");

            release_sock_to_sql_pool(sock_node);
            return Status::OK;
        }
        mysql_free_result(res);

        //
        sql_command = "INSERT INTO Friends (username_1, username_2) VALUES ('" + request->content(0) + "', '" + request->content(1) + "')";
        if (mysql_query(conn, sql_command.data())) {
            printf("INSERT INTO Friends fail %s\n", mysql_error(conn));
            reply->set_information("INSERT WRONG");
            check_sql_sock_normal(sock_node);
        }

        //
        sql_command = "UPDATE User SET synchronism_friend = 1 WHERE username = '" + request->content(0) + "' OR username = '" + request->content(1) + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("UPDATE User fail %s\n", mysql_error(conn));
            printf("%s\n", sql_command.data());
            reply->set_information("UPDATE User WRONG");
            check_sql_sock_normal(sock_node);
        }


        reply->set_information("OK");
        printf("*************Add_friend OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Delete_friend (ServerContext* context, const Repeated_string* request, Inf* reply) override {
        printf("*************Delete_friend IN*************\n");

        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;

        string sql_command = "DELETE FROM Friends WHERE (username_1 = '" + request->content(0) + "' and username_2 = '" + request->content(1) + "') OR (username_1 = '" + request->content(1) + "' and username_2 = '" + request->content(0) + "')";

        if (mysql_query(conn, sql_command.data())) {
            printf("error %s\n", mysql_error(conn));
            printf("%s\n", sql_command.data());

            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }
        // delete result check
        if(mysql_affected_rows(conn) != 1) {
            printf("error happend during delete process\n");
        }

        sql_command = "UPDATE User SET synchronism_friend = 1 WHERE username = '" + request->content(0) + "' OR username = '" + request->content(1) + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("UPDATE User fail %s\n", mysql_error(conn));
            printf("%s\n", sql_command.data());
            reply->set_information("UPDATE User WRONG");
            check_sql_sock_normal(sock_node);
        }

        reply->set_information("OK");
        printf("*************Delete_friend OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Syn (ServerContext* context, ServerReaderWriter<Syn_data, Inf>* stream) override {
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;


        //cout << "Start Syn" << endl;
        printf("Start Syn\n");
        string sql_command;
        Inf request;
        Syn_data reply;

        while (stream->Read(&request)) {

            //            ostringstream ostr;
            //            ostr << i;
            //            string astr = ostr.str();

            sql_command = "SELECT synchronism_friend, synchronism_bill, synchronism_delete, synchronism_request FROM User WHERE username ='" + request.information() + "'";
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
                        printf("error row == NULL\n");
                        printf("fail %s\n", sql_command.data());

                        // get new conn
                        check_sql_sock_normal(sock_node);
                        release_sock_to_sql_pool(sock_node);
                        sock_node = get_sock_from_pool();
                        conn = sock_node->sql_sock->sock;
                    }
                } else {
                    printf("error res == NULL\n");
                    printf("%s fail\n", sql_command.data());

                    check_sql_sock_normal(sock_node);
                    release_sock_to_sql_pool(sock_node);
                    sock_node = get_sock_from_pool();
                    conn = sock_node->sql_sock->sock;
                }
                mysql_free_result(res);
            } else {
                printf("error %s\n", mysql_error(conn));
                printf("%s fail\n", sql_command.data());

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
        printf("END\n");


        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Send_Img (ServerContext *context, ServerReader<Image>* reader, Inf* reply) override {
        printf("*************Send_Img IN*************\n");

        Image image_name;
        Image image_path;
        Image image;
        reader->Read(&image_name);

        // analyze name and path
        string imgName = image_name.data();

        reader->Read(&image_path);
        string path = image_path.data();

        printf("%s %s\n", imgName.data(), path.data());

        // create image file
        FILE* fp;
        path = "./" + path + "/" + imgName + ".png";
        fp = fopen(path.data(),"w+");
        // get image
        reader->Read(&image);
        string str = image.data();
        const char* data = str.data();
        int count = fwrite(data, 1, str.length(),fp);
        printf("count: %d\n", count);
        int r = fclose(fp);

        if (r == EOF) {
            fprintf(stderr, "cannot close file handler\n");
        }

        printf("*************Send_Img OUT*************\n");
        reply->set_information("Get image");
        return Status::OK;
    }

    Status Create_share (ServerContext *context, const Share_inf* request, Inf* reply) override {
        printf("*************Create_share IN*************\n");

        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;

        string sql_command = "INSERT INTO Bills (creater, amount, type, paidBy, date, note, image, member_0, member_1, member_2, member_3, member_4, member_5, member_6, member_7, member_8, member_9, typeIcon) VALUES ('" + request->creater() + "', '" + request->amount() + "' , '" + request->type() + "' , '" + request->paidby() + "' , '" + request->data() + "' , '" + request->note() + "' , '" + request->image() + "' , '" + request->members(0) + "' , '" + request->members(1) + "' , '" + request->members(2) + "' , '" + request->members(3) + "' , '" + request->members(4) + "' , '" + request->members(5) + "' , '" + request->members(6) + "' , '" + request->members(7) + "' , '" + request->members(8) + "' , '" + request->members(9) + "', '" + request->typeicon() + "')";

        printf("%s\n", sql_command.data());

        if (mysql_query(conn, sql_command.data())) {
            printf("error in Create_share Process 1 %s\n", mysql_error(conn));
            printf("%s fail\n", sql_command.data());

            reply->set_information("insert fail");
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        // update synchronism_bill
        sql_command = "UPDATE User SET synchronism_bill = 1 WHERE username = '" + request->members(0) + "' OR username = '" + request->members(1) + "' OR username = '" + request->members(2) + "' OR username = '" + request->members(3) + "' OR username = '" + request->members(4) + "' OR username = '" + request->members(5) + "' OR username = '" + request->members(6) + "' OR username = '" + request->members(7) + "' OR username = '" + request->members(8) + "' OR username = '" + request->members(9) + "'";

        printf("%s\n", sql_command.data());
        if (mysql_query(conn, sql_command.data())) {
            printf("error in Create_share process 2 %s\n", mysql_error(conn));
            printf("%s fail\n", sql_command.data());

            reply->set_information("update fail");
            //printf("%s\n", );
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }


        // push notification
        sql_command = "SELECT deviceToken FROM User WHERE username = '" + request->members(0) + "' OR username = '" + request->members(1) + "' OR username = '" + request->members(2) + "' OR username = '" + request->members(3) + "' OR username = '" + request->members(4) + "' OR username = '" + request->members(5) + "' OR username = '" + request->members(6) + "' OR username = '" + request->members(7) + "' OR username = '" + request->members(8) + "' OR username = '" + request->members(9) + "'";
        printf("%s\n", sql_command.data());
        if (mysql_query(conn, sql_command.data())) {
            printf("error in Create_share process 3 %s\n", mysql_error(conn));
            printf("%s fail\n", sql_command.data());

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
            printf("%s\n", row[0]);
            pushNotificationToDevice(row[0], message);
        }
        mysql_free_result(res);

        reply->set_information("OK");
        printf("*************Create_share OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Delete_bill (ServerContext *context, const Share_inf* request, Inf* reply) override {
        printf("*************Delete_bill IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        //MYSQL_RES *res;
        //MYSQL_ROW row;

        string sql_command = "DELETE FROM Bills WHERE bill_id = " + request->bill_id();
        if (mysql_query(conn, sql_command.data())) {
            printf("delete_bill fail %s\n", mysql_error(conn));
            printf("%s fail\n", sql_command.data());

            reply->set_information("delete fail");
            //printf("%s\n", );
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        sql_command = "UPDATE User SET synchronism_delete = 1 WHERE username = '" + request->members(0) + "' OR username = '" + request->members(1) + "' OR username = '" + request->members(2) + "' OR username = '" + request->members(3) + "' OR username = '" + request->members(4) + "' OR username = '" + request->members(5) + "' OR username = '" + request->members(6) + "' OR username = '" + request->members(7) + "' OR username = '" + request->members(8) + "' OR username = '" + request->members(9) + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("delete_bill update process fail %s\n", mysql_error(conn));
            printf("%s fail\n", sql_command.data());

            reply->set_information("delete update process fail");
            //printf("%s\n", );
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        release_sock_to_sql_pool(sock_node);
        printf("*************Delete_bill OUT*************\n");
        return Status::OK;
    }

    Status Receive_Img (ServerContext *context, const Repeated_string* request, ServerWriter<Image>* reply) override {
        printf("*************Receive_Image IN*************\n");
        Image image;

        for (int i = 1; i != request->content_size(); i++) {

            string path = "./" + request->content(0) + "/" + request->content(i) + ".png";

            FILE *fp = fopen(path.data(), "rb");
            //cout << i << endl;
            if (fp == NULL)
            {
                //fprintf(stderr, "cannot open image \n");
                printf("cannot open image%s\n", path.data());
                image.set_data("");

                //                for (int j = 0; j != request->content_size(); j++) {
                //                    //cout << "!" << request->content(j) << endl;
                //                }

                continue;
            }

            fseek(fp, 0, SEEK_END);

            if (ferror(fp)) {

                fprintf(stderr, "fseek() failed\n");
                int r = fclose(fp);

                if (r == EOF) {
                    fprintf(stderr, "cannot close file handler\n");
                }

                image.set_data("");
                continue;
            }

            int flen = ftell(fp);

            if (flen == -1) {

                perror("error occurred");
                int r = fclose(fp);

                if (r == EOF) {
                    fprintf(stderr, "cannot close file handler\n");
                }

                image.set_data("");
                continue;
            }

            fseek(fp, 0, SEEK_SET);

            if (ferror(fp)) {

                fprintf(stderr, "fseek() failed\n");
                int r = fclose(fp);

                if (r == EOF) {
                    fprintf(stderr, "cannot close file handler\n");
                }

                image.set_data("");
                continue;
            }
            char *data = new char[flen + 1];

            fread(data, 1, flen, fp);
            data[flen] = '\0';


            if (ferror(fp)) {

                fprintf(stderr, "fread() failed\n");
                int r = fclose(fp);

                if (r == EOF) {
                    fprintf(stderr, "cannot close file handler\n");
                }

                image.set_data("");
                continue;
            }

            int r = fclose(fp);

            if (r == EOF) {
                fprintf(stderr, "cannot close file handler\n");
                continue;
            }

            string image_data(data, flen);
            image.set_data(image_data);
            image.set_name(request->content(i));
            reply->Write(image);
            delete []data;
        }

        printf("*************Receive_Image OUT*************\n");
        return Status::OK;
    }

    Status Obtain_bills (ServerContext *context, const Bill_request *request, ServerWriter<Share_inf> *reply) override {
        printf("*************Obtain_bills IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;
        string sql_command;
        if (request->amount() == "all")
        {
            sql_command = "SELECT * FROM Bills WHERE member_0 = '" + request->username() + "' OR member_1 = '" + request->username() + "' OR member_2 = '" + request->username() + "' OR member_3 = '" + request->username() + "' OR member_4 = '" + request->username() + "' OR member_5 = '" + request->username() + "' OR member_6 = '" + request->username() + "' OR member_7 = '" + request->username() + "' OR member_8 = '" + request->username() + "' OR member_9 = '" + request->username() + "' OR paidBy = '" + request->username() + "' order by bill_id desc";
        } else  {
            sql_command = "SELECT * FROM Bills WHERE member_0 = '" + request->username() + "' OR member_1 = '" + request->username() + "' OR member_2 = '" + request->username() + "' OR member_3 = '" + request->username() + "' OR member_4 = '" + request->username() + "' OR member_5 = '" + request->username() + "' OR member_6 = '" + request->username() + "' OR member_7 = '" + request->username() + "' OR member_8 = '" + request->username() + "' OR member_9 = '" + request->username() + "' OR paidBy = '" + request->username() + "' order by bill_id desc LIMIT " + request->amount();
        }

        if (mysql_query(conn, sql_command.data())) {
            printf("error %s\n", mysql_error(conn));
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
            bill.set_image(row[7]);
            bill.set_paidby(row[8]);
            bill.add_members(row[9]);
            bill.add_members(row[10]);
            bill.add_members(row[11]);
            bill.add_members(row[12]);
            bill.add_members(row[13]);
            bill.add_members(row[14]);
            bill.add_members(row[15]);
            bill.add_members(row[16]);
            bill.add_members(row[17]);
            bill.add_members(row[18]);
            bill.set_paidstatus(row[19]);
            if (row[20]) {
                bill.set_typeicon(row[20]);
            } else {
                bill.set_typeicon("");
            }
            //            //cout << "amout " << row[2] << endl;
            printf("one result\n");
            reply->Write(bill);
        }
        mysql_free_result(res);

        sql_command = "UPDATE User SET synchronism_bill = 0 WHERE username = '" + request->username() + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Change syn flag = 0 fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);

        }


        printf("*************Obtain_bills OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Reset_Status (ServerContext *context, const Inf* request, Inf* reply) override {
        printf("*************Reset_Status IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        //MYSQL_RES *res;
        //MYSQL_ROW row;

        string sql_command = "UPDATE User SET synchronism_delete = 0 WHERE username = '" + request->information() + "'";

        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Change synchronism_delete = 0 fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
        }
        printf("*************Reset_Status OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    // request system

    Status Send_request (ServerContext* content, const Request* request, Inf* reply) override {
        printf("*************Send_request IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        //MYSQL_RES *res;
        //MYSQL_ROW row;

        string sql_command = "INSERT INTO Request (sender, receiver, type, content, request_date) VALUES ('" + request->sender() + "', '" + request->receiver() + "', '" + request->type() + "', '" + request->content() + "', '" + request->request_date() + "')";

        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Send_request insert process fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        sql_command = "UPDATE User SET synchronism_request = 1 WHERE username = '" + request->receiver() + "'";

        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Send_request update process fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
        }

        printf("*************Send_request OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Obtain_request (ServerContext* content, const Inf* request, ServerWriter<Request>* reply) override {
        printf("*************Obtain_request IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;

        string sql_command = "SELECT * FROM Request WHERE receiver = '" + request->information() + "' order by request_id desc";

        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Obtain_request fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }
        res = mysql_use_result(conn);
        while ((row = mysql_fetch_row(res)) != NULL) {
            Request req;
            req.set_request_id(row[0]);
            req.set_sender(row[1]);
            req.set_receiver(row[2]);
            req.set_type(row[3]);
            req.set_content(row[4]);

            req.set_request_date(row[6]);
            printf("one request\n");
            reply->Write(req);
        }
        mysql_free_result(res);

        sql_command = "UPDATE User SET synchronism_request = 0 WHERE username = '" + request->information() + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Obtain_request update process fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        printf("*************Obtain_request OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Obtain_requestLog (ServerContext* content, const Inf* request, ServerWriter<Request>* reply) override {
        printf("*************Obtain_requestLog IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;
        string sql_command = "SELECT * FROM RequestLog WHERE (sender = '" + request->information() + "' AND ignore_sender = 'FALSE') OR (receiver = '" + request->information() + "' AND ignore_receiver = 'FALSE') order by requestLog_id desc";

        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Obtain_requestLog fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }
        printf("%s\n", sql_command.data());
        res = mysql_use_result(conn);
        printf("t\n");
        while ((row = mysql_fetch_row(res)) != NULL) {

            Request req;
            req.set_request_id(row[0]);
            req.set_sender(row[1]);
            req.set_receiver(row[2]);
            req.set_type(row[3]);
            req.set_content(row[4]);
            req.set_response(row[5]);
            req.set_request_date(row[6]);
            req.set_response_date(row[7]);

            //printf("hello\n");
            reply->Write(req);
        }
        mysql_free_result(res);
        printf("t\n");
        sql_command = "UPDATE User SET synchronism_request = 0 WHERE username = '" + request->information() + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Obtain_requestLog update process fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        sql_command = "UPDATE RequestLog SET ignore_receiver = 'TRUE' WHERE receiver = '" + request->information() + "' AND ignore_receiver = 'FALSE'";
        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Obtain_requestLog update process fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        sql_command = "UPDATE RequestLog SET ignore_sender = 'TRUE' WHERE sender = '" + request->information() + "' AND ignore_sender = 'FALSE'";
        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Obtain_requestLog update process fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        printf("*************Obtain_requestLog OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Obtain_requestLogHistory (ServerContext* content, const Inf* request, ServerWriter<Request>* reply) override {
        printf("*************Obtain_requestLogHistory IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;
        string sql_command = "SELECT * FROM RequestLog WHERE sender = '" + request->information() + "' OR receiver = '" + request->information() + "' order by requestLog_id desc";

        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Obtain_requestLogHistory fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }
        printf("%s\n", sql_command.data());
        res = mysql_use_result(conn);
        printf("t\n");
        while ((row = mysql_fetch_row(res)) != NULL) {

            Request req;
            req.set_request_id(row[0]);
            req.set_sender(row[1]);
            req.set_receiver(row[2]);
            req.set_type(row[3]);
            req.set_content(row[4]);
            req.set_response(row[5]);
            req.set_request_date(row[6]);
            req.set_response_date(row[7]);

            //printf("hello\n");
            reply->Write(req);
        }
        mysql_free_result(res);

        printf("*************Obtain_requestLogHistory OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Request_response (ServerContext* content, const Response* request, Inf* reply) override {
        printf("*************Request_response IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        MYSQL_RES *res;
        MYSQL_ROW row;
        Request req;
        string type;

        string sql_command = "SELECT * FROM Request WHERE request_id = '" + request->request_id() + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Request_response SELECT process fail\n");
            printf("%s\n", sql_command.data());
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
            sql_command = "INSERT INTO RequestLog (sender, receiver, type, content, response, request_date, response_date) VALUES ('" + req.sender() + "', '" + req.receiver() + "', '" + req.type() + "', '" + req.content() + "', '" + req.response() + "', '" + req.request_date() + "', '" + req.response_date() + "')";
            if (mysql_query(conn, sql_command.data())) {
                printf("ERROR Request_response INSERT process fail\n");
                printf("%s\n", sql_command.data());
                check_sql_sock_normal(sock_node);
                release_sock_to_sql_pool(sock_node);
                return Status::CANCELLED;
            }

            sql_command = "DELETE FROM Request WHERE request_id = '" + request->request_id() + "'";
            if (mysql_query(conn, sql_command.data())) {
                printf("ERROR Request_response DELETE process fail\n");
                printf("%s\n", sql_command.data());
                check_sql_sock_normal(sock_node);
                release_sock_to_sql_pool(sock_node);
                return Status::CANCELLED;
            }

            sql_command = "UPDATE User SET synchronism_request = 2, synchronism_delete = 1 WHERE username = '" + request->sender() + "' OR username = '" + request->receiver() + "'";
            printf("%s\n", sql_command.data());
            if (mysql_query(conn, sql_command.data())) {
                printf("ERROR Request_response update process fail\n");
                printf("%s\n", sql_command.data());
                check_sql_sock_normal(sock_node);
                release_sock_to_sql_pool(sock_node);
                return Status::CANCELLED;
            }

        } else if (type == "friendInvite")
        {

        }




        printf("*************Request_response OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status MakePayment (ServerContext* content, ServerReader<BillPayment>* reader, Inf* reply) override {
        printf("*************MakePayment IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;

        BillPayment request;

        while (reader->Read(&request)) {

            string sql_command = "UPDATE Bills SET paidStatus = '" + request.paidstatus() + "' WHERE bill_id = '" + request.bill_id() + "'";
            if (mysql_query(conn, sql_command.data())) {
                printf("ERROR MakePayment fail\n");
                printf("%s\n", sql_command.data());
                check_sql_sock_normal(sock_node);
                release_sock_to_sql_pool(sock_node);
                return Status::CANCELLED;
            }
            //printf("in");
        }

        printf("*************MakePayment OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Create_requestLog (ServerContext* content, const Request* request, Inf* reply) override {
        printf("*************Create_requestLog IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        //MYSQL_RES *res;
        //MYSQL_ROW row;

        string sql_command = "INSERT INTO RequestLog (sender, receiver, type, content, response, request_date, response_date) VALUES ('" + request->sender() + "', '" + request->receiver() + "', '" + request->type() + "', '" + request->content() + "', '" + request->response() + "', '" + request->request_date() + "', '" + request->request_date() + "')";

        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Send_request insert process fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        sql_command = "UPDATE User SET synchronism_request = 2, synchronism_delete = 1 WHERE username = '" + request->sender() + "' OR username = '" + request->receiver() + "'";

        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Send_request update process fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        printf("*************Create_requestLog OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status IgnoreRequestLog (ServerContext* content, const IgnoreMessage* request, Inf* reply) override {
        printf("*************IgnoreRequestLog IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;
        //MYSQL_RES *res;
        //MYSQL_ROW row;

        string sql_command = "UPDATE RequestLog SET ignore_" + request->user() + " = 'TRUE' WHERE requestLog_id = '" + request->requestlog_id() + "'";
        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR IgnoreRequestLog fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        printf("*************IgnoreRequestLog OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

    Status Send_DeviceToken (ServerContext* content, const Repeated_string* request, Inf* reply) override {
        printf("*************Send_DeviceToken IN*************\n");
        SQL_SOCK_NODE* sock_node = get_sock_from_pool();
        MYSQL* conn = sock_node->sql_sock->sock;

        string sql_command = "UPDATE User SET deviceToken = '" + request->content(1) + "' WHERE username = '" + request->content(0) + "'";
        printf("DeviceToken:%s\n", request->content(1).data());
        if (mysql_query(conn, sql_command.data())) {
            printf("ERROR Send_DeviceToken fail\n");
            printf("%s\n", sql_command.data());
            check_sql_sock_normal(sock_node);
            release_sock_to_sql_pool(sock_node);
            return Status::CANCELLED;
        }

        printf("*************Send_DeviceToken OUT*************\n");
        release_sock_to_sql_pool(sock_node);
        return Status::OK;
    }

};

void RunServer() {
    std::string server_address("0.0.0.0:50051");
    GreeterServiceImpl service;

    ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on : " << server_address << std::endl;
    server->Wait();
}


string convertToString(double d) {
    ostringstream os;
    if (os << d)
        return os.str();
    return "invalid conversion";
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
    const char* db = "iShare_data";
    cout << sql_pool_create(CONN_NUM, hostname, username, passwd, db, 3306, NULL, Create_sock, Close_sock);
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

