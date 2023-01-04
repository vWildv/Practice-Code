#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>

#include <threadpool.h>
#include <http_conn.h>

const int MAX_FD = 65536;
const int MAX_EVENT_NUMBER = 10000;
const int TIMESLOT = 5;

class webserver{
    public:
        webserver();
        ~webserver();

        void init(int port, string user, string passwd, string databasename, int log_write, int opt_linger, int trimode, int sql_num, int thread_num, int close_log, int actor_model);
        void thread_pool();
        void sql_pool();
        void log_writer();
        void tri_mode();
        void event_listen();
        void event_loop();
        void timer(int connfd, struct sockaddr_in client_addr);
        void adjust_timer(util_timer* timer);
        void deal_timer(util_timer* timer, int sockfd);
        bool deal_clientdata();
        bool deal_sig(bool &timeout, bool &stop_server);
        void deal_read(int socfd);
        void deal_write(int sockfd);

        int port;
        char* root;
        int log_write;
        int close_log;
        int actor_model;
        int pipefd[2];
        int epollfd;
        http_conn* users;

        connection_pool* connpool;
        string user;
        string passwd;
        string databasename;
        int sql_num;

        threadpool<http_conn>* pool;
        int thread_num;

        epoll_event events[MAX_EVENT_NUMBER];
        int listenfd;
        int OPT_LINGER;
        int TRIMode;
        int LISTENTRIMode;
        int CONNTRIMode;

        client_data* users_timer;
        utils utls;
};

#endif