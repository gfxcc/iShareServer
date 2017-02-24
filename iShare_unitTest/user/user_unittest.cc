#include <limits.h>
#include "user.h"
#include "gtest/gtest.h"

TEST(iShare_user_unittest, user_inf) {
    GreeterClient greeter(grpc::CreateChannel(
                "localhost:50056", grpc::InsecureChannelCredentials()));
    Inf request;
    request.set_information("1");

    User_detail reply = greeter.Test_User_inf(request);

    EXPECT_EQ(5, reply.friends_id_size());
    EXPECT_EQ(5, reply.friends_name_size());
}

TEST(iShare_user_unittest, login) {
    GreeterClient greeter(grpc::CreateChannel(
                "localhost:50056", grpc::InsecureChannelCredentials()));
    Login_m request;
    Reply_inf reply;

    request.set_username("gfxcc");
    request.set_password("19920406");
    reply = greeter.Test_Login(request);
    EXPECT_EQ("OK", reply.status());

    request.set_username("gfxcc");
    request.set_password("");
    reply = greeter.Test_Login(request);
    EXPECT_EQ("CANCELLED", reply.status());

    request.set_username("");
    request.set_password("");
    reply = greeter.Test_Login(request);
    EXPECT_EQ("CANCELLED", reply.status());

}

TEST(iShare_user_unittest, signup) {
    GreeterClient greeter(grpc::CreateChannel(
                "localhost:50056", grpc::InsecureChannelCredentials()));
    Sign_m request;
    Reply_inf reply;

    request.set_username("gfxcc");
    request.set_password("");
    reply = greeter.Test_Sign_up(request);
    EXPECT_EQ("CANCELLED", reply.status());

    request.set_username("unittest");
    request.set_password("unittest");
    reply = greeter.Test_Sign_up(request);
    EXPECT_EQ("OK", reply.status());
}

TEST(iShare_user_unittest, search_username) {
    GreeterClient greeter(grpc::CreateChannel(
                "localhost:50056", grpc::InsecureChannelCredentials()));
    Inf request;
    Search_result reply;

    request.set_information("gfxcc");
    reply = greeter.Test_Search_username(request);
    EXPECT_EQ(1, reply.username_size());
}

TEST(iShare_user_unittest, add_friend) {
    GreeterClient greeter(grpc::CreateChannel(
                "localhost:50056", grpc::InsecureChannelCredentials()));
    EXPECT_EQ(1, 1);
}

