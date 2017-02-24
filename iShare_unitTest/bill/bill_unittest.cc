#include <limits.h>
#include "bill.h"
#include "gtest/gtest.h"


TEST(iShare_bill_unittest, ObtainTest) {

    GreeterClient greeter(grpc::CreateChannel(
                "localhost:50056", grpc::InsecureChannelCredentials()));

    vector<Share_inf> res = greeter.Test_Obtain_bills();

    EXPECT_EQ(5, res.size());
}

TEST(iSHare_bill_unittest, DeleteTest) {
    GreeterClient greeter(grpc::CreateChannel(
                "localhost:50056", grpc::InsecureChannelCredentials()));

    vector<Share_inf> res = greeter.Test_Obtain_bills();

    EXPECT_EQ(5, res.size());

    greeter.Test_Delete_bill(res[0]);
    res = greeter.Test_Obtain_bills();

    EXPECT_EQ(4, res.size());
}

TEST(iSHare_bill_unittest, CreateTest) {
    GreeterClient greeter(grpc::CreateChannel(
                "localhost:50056", grpc::InsecureChannelCredentials()));
    vector<Share_inf> res = greeter.Test_Obtain_bills();

    EXPECT_EQ(4, res.size());

    greeter.Test_Create_share();
    res = greeter.Test_Obtain_bills();

    EXPECT_EQ(5, res.size());
}
