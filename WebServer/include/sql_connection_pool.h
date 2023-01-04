#ifndef CONNECTION_POOL_H
#define CONNECTION_POOL_H

#include <stdio.h>
#include <list>
#include <mysql/mysql.h>
#include <error.h>
#include <string.h>
#include <iostream>
#include <string>

#include <lock.h>
using namespace std;

class connection_pool{
    public:
        MYSQL* get_conn();
        bool release_conn(MYSQL* conn);
        int get_free_con();
        void destroy_pool();

        static connection_pool* get_instance();

        void init(string url, string user, string password, string database_name, int port, int max_conn, int close_log);

        string url;
        int port;
        string user;
        string password;
        string database_name;
        int close_log;

    private:
        connection_pool();
        ~connection_pool();

        int max_conn;
        int cur_conn;
        int free_conn;
        locker lock;
        list<MYSQL* > connlist;
        sem reserve;
};

class connectionRAII{
    public:
        connectionRAII(MYSQL** con, connection_pool* conn_pool);
        ~connectionRAII();
    
    private:
        MYSQL* connRAII;
        connection_pool* poolRAII;
};

#endif
