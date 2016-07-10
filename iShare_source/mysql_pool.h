#include <mysql/mysql.h>
#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define DB_MAX_STRLNE    64


using namespace std;

typedef struct _SQL_SOCK{
    MYSQL * sock;
} SQL_SOCK;

typedef int (* IS_DB_CONN_FUNC)(void *);
typedef SQL_SOCK *(* DB_CONN_SINGLE_FUNC)(char *, char *, char *, char *, unsigned short);

typedef void (* DB_SOCK_DESTROY_FUNC)(SQL_SOCK *);

enum _SQL_STATUS
{
    DB_DISCONN = 0,
    DB_CONN = 1
};

typedef struct _SQL_SOCK_NODE{
    struct _SQL_SOCK_NODE * next;
    SQL_SOCK * sql_sock;
    pthread_mutex_t sql_lock;
    _SQL_STATUS sql_state;
} SQL_SOCK_NODE;

typedef struct _POOL_SQL_SOCK{
    SQL_SOCK_NODE * sql_pool;
    char db_host[DB_MAX_STRLNE];
    char db_user[DB_MAX_STRLNE];
    char db_passwd[DB_MAX_STRLNE];
    char db_name[DB_MAX_STRLNE];
    unsigned short db_port;
    IS_DB_CONN_FUNC db_is_connect;
    DB_CONN_SINGLE_FUNC db_conn_single;
    DB_SOCK_DESTROY_FUNC db_sock_destroy;
} POOL_SQL_SOCK;

int sql_pool_create(int c, const char * db_host,
                    const char * db_user, const char * db_passwd,
                    const char * db_name, unsigned short port,
                    IS_DB_CONN_FUNC is_conn,
                    DB_CONN_SINGLE_FUNC db_conn_single,
                    DB_SOCK_DESTROY_FUNC db_sock_close);

void sql_pool_destroy();
SQL_SOCK_NODE * get_sock_from_pool();
void release_sock_to_sql_pool(SQL_SOCK_NODE * n);
void check_sql_sock_normal(SQL_SOCK_NODE * n);
