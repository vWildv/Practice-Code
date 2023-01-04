#include <sql_connection_pool.h>

connection_pool::connection_pool(){
    cur_conn = 0;
    free_conn = 0;
}

connection_pool* connection_pool::get_instance(){
    static connection_pool conn_pool;
    return &conn_pool;
}

void connection_pool::init(string url, string user, string password, string database_name, int port, int max_conn, int close_log){
    this->url = url;
    this->user = user;
    this->password = password;
    this->database_name = database_name;
    this->close_log = close_log;
    this->port = port;

    for(int i=0;i<max_conn;i++){
        MYSQL* conn = NULL;
        conn = mysql_init(conn);
        if(conn == NULL){
            exit(1);
        }
        conn = mysql_real_connect(conn,url.c_str(),user.c_str(),password.c_str(), database_name.c_str(), port, NULL, 0);
        if(conn == NULL){
            exit(1);
        }
        connlist.push_back(conn);
        free_conn++;
    }
    reserve = sem(free_conn);
    this->max_conn = free_conn;
}

MYSQL* connection_pool::get_conn(){
    MYSQL* conn = NULL;
    if (connlist.size() == 0) return NULL;
    
    reserve.wait();
    lock.lock();

    conn = connlist.front();
    connlist.pop_front();
    free_conn--;
    cur_conn++;

    lock.unlock();
    return conn;
}

bool connection_pool::release_conn(MYSQL* conn){
    if(conn = NULL) return false;
    lock.lock();

    connlist.push_back(conn);
    free_conn++;
    cur_conn--;

    lock.unlock();
    reserve.post();
    return true;
}

void connection_pool::destroy_pool(){
    lock.lock();
    if(connlist.size() > 0){
        for(list<MYSQL*>::iterator it = connlist.begin();it!=connlist.end();it++){
            MYSQL* conn = *it;
            mysql_close(conn);
        }
        cur_conn = 0;
        free_conn = 0;
        connlist.clear();
    }
    lock.unlock();
}

int connection_pool::get_free_con(){
    return free_conn;
}

connection_pool::~connection_pool(){
    destroy_pool();
}

connectionRAII::connectionRAII(MYSQL** SQL, connection_pool *conn_pool){
    *SQL = conn_pool->get_conn();

    connRAII = *SQL;
    poolRAII = conn_pool;
}

connectionRAII::~connectionRAII(){
    poolRAII->release_conn(connRAII);
}