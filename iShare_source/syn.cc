/*
Status GreeterServiceImpl::Syn (ServerContext* context, ServerReaderWriter<Syn_data, Inf>* stream) {
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
*/
