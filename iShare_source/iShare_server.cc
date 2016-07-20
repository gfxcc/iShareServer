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


Status GreeterServiceImpl::SayHello(ServerContext* context, const HelloRequest* request,
        HelloReply* reply) {
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());
    std::cout << "get request" << std::endl;
    return Status::OK;
}

Status GreeterServiceImpl::Send_Img (ServerContext *context, ServerReader<Image>* reader, Inf* reply) {
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
    path = "/home/ubuntu/iShare_files/" + path + "/" + imgName + ".png";
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

Status GreeterServiceImpl::Receive_Img(ServerContext *context, const Repeated_string* request, ServerWriter<Image>* reply) {
    log(INFO, "IN Receive_Img");
    Image image;

    for (int i = 1; i != request->content_size(); i++) {

        string path = "/home/ubuntu/iShare_files/" + request->content(0) + "/" + request->content(i) + ".png";

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
        log(DEBUG, request->content(i).c_str());
        delete []data;
    }

    log(INFO, "OUT Receive_Img");
    return Status::OK;
}

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

