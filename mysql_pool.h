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

/////////////////////////////////////////////////////
// Create mysql_pool
/////////////////////////////////////////////////////

static POOL_SQL_SOCK * sql_sock_pool = NULL;

int sql_pool_create(int c, const char * db_host,
                    const char * db_user, const char * db_passwd,
                    const char * db_name, unsigned short port,
                    IS_DB_CONN_FUNC is_conn,
                    DB_CONN_SINGLE_FUNC db_conn_single,
                    DB_SOCK_DESTROY_FUNC db_sock_close)
{
    int i = 0;
    SQL_SOCK_NODE * new_node = NULL;

    //参数检查
    if (!(db_host && db_user && db_passwd && db_name))
        return 2;
    
    sql_sock_pool = (POOL_SQL_SOCK *)malloc(sizeof(POOL_SQL_SOCK));
    //内存分配不成功
    if (NULL == sql_sock_pool)
        return 1;

    memset(sql_sock_pool, 0, sizeof(*sql_sock_pool));
    sql_sock_pool->db_conn_single = db_conn_single;
    sql_sock_pool->db_is_connect = is_conn;
    sql_sock_pool->db_sock_destroy = db_sock_close;
    strncpy(sql_sock_pool->db_host, db_host, DB_MAX_STRLNE);
    strncpy(sql_sock_pool->db_user, db_user, DB_MAX_STRLNE);
    strncpy(sql_sock_pool->db_passwd, db_passwd, DB_MAX_STRLNE);
    strncpy(sql_sock_pool->db_name, db_name, DB_MAX_STRLNE);

    for(; i < c; i ++) {
        new_node = (SQL_SOCK_NODE *)malloc(sizeof(SQL_SOCK_NODE));
        if (NULL == new_node) {
            goto POOL_CREATE_FAILED;
        }
        memset(new_node, 0, sizeof(*new_node));
        if(pthread_mutex_init(&new_node->sql_lock, NULL)) {
            free(new_node);
            goto POOL_CREATE_FAILED;
        }
        new_node->sql_sock = db_conn_single(sql_sock_pool->db_host, sql_sock_pool->db_user, sql_sock_pool->db_passwd, sql_sock_pool->db_name, port);
        //cout << sql_sock_pool->db_host << endl << sql_sock_pool->db_user << endl << sql_sock_pool->db_passwd << endl << sql_sock_pool->db_name << endl;
        if (NULL == new_node->sql_sock)
            goto POOL_CREATE_FAILED;
        new_node->sql_state = DB_CONN;

        new_node->next = sql_sock_pool->sql_pool;
        sql_sock_pool->sql_pool = new_node;
    }
    return 0;
POOL_CREATE_FAILED:
    sql_pool_destroy();
    return -1;
}

/////////////////////////////////////////////////////
// free mysql_pool
/////////////////////////////////////////////////////
void sql_pool_destroy()
{
    SQL_SOCK_NODE * tmp, *head = sql_sock_pool->sql_pool;

    while(head) {
        tmp = head;
        head = head->next;

        pthread_mutex_lock(&tmp->sql_lock);
        sql_sock_pool->db_sock_destroy(tmp->sql_sock);
        pthread_mutex_unlock(&tmp->sql_lock);
        pthread_mutex_destroy(&tmp->sql_lock);
        free(tmp);
    }
}


/////////////////////////////////////////////////////
// Get mysql_sock
/////////////////////////////////////////////////////

SQL_SOCK_NODE * get_sock_from_pool()
{
    static SQL_SOCK_NODE * last_used = sql_sock_pool->sql_pool;
    SQL_SOCK_NODE * ret = NULL;

    //防止出现问题
    if (NULL == last_used)
        return NULL;
    
    ret = last_used->next;
    
    while(ret != last_used) {
        if (ret == NULL)
            ret = sql_sock_pool->sql_pool;
        
        if (0 == pthread_mutex_trylock(&ret->sql_lock)) {
            //决定是否重新连接
            if ((DB_DISCONN == ret->sql_state) && 
                (!(ret->sql_sock) || (0 == sql_sock_pool->db_is_connect(ret->sql_sock)))) {
                sql_sock_pool->db_sock_destroy(ret->sql_sock);
                
                ret->sql_sock = sql_sock_pool->db_conn_single(sql_sock_pool->db_host, 
                    sql_sock_pool->db_user, sql_sock_pool->db_passwd, sql_sock_pool->db_name, sql_sock_pool->db_port);

                //数据库不能建立连接，进程应该等待数据库恢复正常
                if (NULL == ret->sql_sock) {
                    pthread_mutex_unlock(&ret->sql_lock);
                    cout << "sql pool used!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<< endl;
                    return NULL;
                }
                
            }

            last_used = ret;
            return ret;
        }

        //获取当前连接不成功
        ret = ret->next;
    } 
    
    cout << "sql pool used!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<< endl;
    //连接池连接被耗尽
    return NULL;
}

/////////////////////////////////////////////////////
// Release mysql_sock
/////////////////////////////////////////////////////
void release_sock_to_sql_pool(SQL_SOCK_NODE * n)
{
    pthread_mutex_unlock(&n->sql_lock);
}

/////////////////////////////////////////////////////
// Check mysql_pool
/////////////////////////////////////////////////////
void check_sql_sock_normal(SQL_SOCK_NODE * n)
{
    n->sql_state = DB_DISCONN;
}


/////////////


SQL_SOCK_NODE * check_sock_from_pool()
{
    static SQL_SOCK_NODE * last_used = sql_sock_pool->sql_pool;
    SQL_SOCK_NODE * ret = NULL;
    
    //防止出现问题
    if (NULL == last_used)
        return NULL;
    
    ret = last_used->next;
    
    while(ret != last_used) {
        if (ret == NULL)
            ret = sql_sock_pool->sql_pool;
        
        if (0 == pthread_mutex_trylock(&ret->sql_lock)) {
            //决定是否重新连接
            if ((DB_DISCONN == ret->sql_state) &&
                (!(ret->sql_sock) || (0 == sql_sock_pool->db_is_connect(ret->sql_sock)))) {
                sql_sock_pool->db_sock_destroy(ret->sql_sock);
                
                ret->sql_sock = sql_sock_pool->db_conn_single(sql_sock_pool->db_host,
                                                              sql_sock_pool->db_user, sql_sock_pool->db_passwd, sql_sock_pool->db_name, sql_sock_pool->db_port);
                
                //数据库不能建立连接，进程应该等待数据库恢复正常
                if (NULL == ret->sql_sock) {
                    pthread_mutex_unlock(&ret->sql_lock);
                    cout << "sql pool used!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<< endl;
                    return NULL;
                }
                
            }
            
            last_used = ret;
            return ret;
        }
        
        //获取当前连接不成功
        ret = ret->next;
    }
    
    cout << "sql pool used!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<< endl;
    //连接池连接被耗尽
    return NULL;
}


