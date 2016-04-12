/*
 *
 * created by YongCao at 2/Mar/2016
 *
 * this tools used to transfer data from
 *      iShare_data ----->>>>> iShare_server
 *
 *
 *
 * */

#include <stdio.h>
#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <mysql/mysql.h>
#include <iostream>
#include <pthread.h>
#include <vector>

#include "mysql_pool.h"

#define CONN_NUM 5

using namespace std;

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


int main() {

    printf("[INFO] open database iShare_data\n");
    const char* hostname = "localhost";
    const char* username = "root";
    const char* passwd = "19920406Cy";
    const char* db = "iShare_data";
    //sql_pool_create(CONN_NUM, hostname, username,
    //            passwd, db, 3306, NULL, Create_sock, Close_sock);

    sql_pool_create(CONN_NUM, hostname, username,
            passwd, db, 3306, NULL, Create_sock, Close_sock);

    SQL_SOCK *sock_s = Create_sock((char*)hostname, (char*)username, (char*)passwd, (char*)"iShare_server", 3306);
    MYSQL *conn_s = sock_s->sock;
    MYSQL_RES *res_s;
    MYSQL_ROW row_s;

    printf("[INFO] start to transfer TABLE Bills\n");
    SQL_SOCK_NODE* sock_node = get_sock_from_pool();
    MYSQL* conn = sock_node->sql_sock->sock;
    MYSQL_RES *res;
    MYSQL_ROW row;

    SQL_SOCK_NODE* sock_node_1 = get_sock_from_pool();
    MYSQL* conn_1 = sock_node_1->sql_sock->sock;
    MYSQL_RES *res_1;
    MYSQL_ROW row_1;

    string sql_command = "SELECT * FROM Bills WHERE bill_id > 423";
    if (mysql_query(conn, sql_command.data())) {
        printf("[ERROR] select Bills fail\n");
        return 0;
    }
    res = mysql_store_result(conn);
    int num_line = mysql_num_rows(res);
    int count = 0;
    while ((row = mysql_fetch_row(res)) != NULL) {

        string creater, paidBy;
        vector<string> member_vec;

        // set vec
        if (row[9])
            member_vec.push_back(row[9]);
        if (row[10])
            member_vec.push_back(row[10]);
        if (row[11])
            member_vec.push_back(row[11]);
        if (row[12])
            member_vec.push_back(row[12]);
        if (row[13])
            member_vec.push_back(row[13]);
        if (row[14])
            member_vec.push_back(row[14]);
        if (row[15])
            member_vec.push_back(row[15]);
        if (row[16])
            member_vec.push_back(row[16]);
        if (row[17])
            member_vec.push_back(row[17]);
        if (row[18])
            member_vec.push_back(row[18]);

        sql_command = "SELECT user_id FROM User WHERE username = '" + string(row[1]) + "'";
        if (mysql_query(conn_1, sql_command.data())) {
            printf("[ERROR] select user_id for creater fail\n");
            return 0;
        }
        res_1 = mysql_use_result(conn_1);
        if ((row_1 = mysql_fetch_row(res_1)) != NULL) {
            creater = row_1[0];
        }
        mysql_free_result(res_1);

        sql_command = "SELECT user_id FROM User WHERE username = '" + string(row[8]) + "'";
        if (mysql_query(conn_1, sql_command.data())) {
            printf("[ERROR] select user_id for paidBy fail\n");
            return 0;
        }
        res_1 = mysql_use_result(conn_1);
        if ((row_1 = mysql_fetch_row(res_1)) != NULL) {
            paidBy = row_1[0];
        }
        mysql_free_result(res_1);

        // get member_0 ~ member_9
        for (int i = 0; i != member_vec.size(); i++) {
            sql_command = "SELECT user_id FROM User WHERE username = '" + member_vec[i] + "'";
            if (mysql_query(conn_1, sql_command.data())) {
                printf("[ERROR] select user_id for member fail\n");
                return 0;
            }
            res_1 = mysql_use_result(conn_1);
            if ((row_1 = mysql_fetch_row(res_1)) != NULL) {
                member_vec[i] = string(row_1[0]);
            }
            mysql_free_result(res_1);
        }

        for (int i = 0; i != member_vec.size(); i++) {
            if (member_vec[i] == "")
                member_vec[i] = "NULL";
        }

        for (int i = 0; i != (10 - member_vec.size()); i++) {
            member_vec.push_back("NULL");
        }
        // insert new content to iShare_server
        if (row[20]) {
            sql_command = "INSERT INTO Bills (creater, amount, type, date, note, paidBy, member_0, member_1, member_2, member_3, member_4, member_5, member_6, member_7, member_8, member_9, paidStatus, typeIcon) VALUES (" +
                        creater + ", " + row[2] + ", '" + row[3] + "', '" + row[5] + "', '" + row[6] +
                       "', " + paidBy + ", " + member_vec[0] + ", " + member_vec[1] + ", " + member_vec[2] + ", " +
                       member_vec[3] + ", " + member_vec[4] + ", " + member_vec[5] + ", " + member_vec[6] + ", " +
                       member_vec[7] + ", " + member_vec[8] + ", " + member_vec[9] + ", '" + row[19] + "', '" +
                       row[20] + "')";
        } else {
            sql_command = "INSERT INTO Bills (creater, amount, type, date, note, paidBy, member_0, member_1, member_2, member_3, member_4, member_5, member_6, member_7, member_8, member_9, paidStatus) VALUES (" +
                        creater + ", " + row[2] + ", '" + row[3] + "', '" + row[5] + "', '" + row[6] +
                       "', " + paidBy + ", " + member_vec[0] + ", " + member_vec[1] + ", " + member_vec[2] + ", " +
                       member_vec[3] + ", " + member_vec[4] + ", " + member_vec[5] + ", " + member_vec[6] + ", " +
                       member_vec[7] + ", " + member_vec[8] + ", " + member_vec[9] + ", '" + row[19] + "')";
        }

        if (mysql_query(conn_s, sql_command.data())) {
            printf("[ERROR] insert new row fail\n");
            printf("[ERROR] %s\n", mysql_error(conn_s));
            return 0;
        }

        count++;
        // show indicator
        int block = (count * 20)/num_line;
        printf("\r[");
        for (int i = 0; i != 20; i++) {
            if (i <= block)
                printf("#");
            else
                printf(" ");
        }
        printf("]");
        fflush(stdout);
    }
    printf("\n");
    return 0;
}
