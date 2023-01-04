#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <map>

#include <lock.h>
#include <sql_connection_pool.h>
#include <timer.h>

class http_conn{
    public:
        static const int FILENAME_LEN = 200;
        static const int READ_BUFFER_SIZE = 2048;
        static const int WRITE_BUFFER_SIZE = 1024;

        enum METHOD{
            GET = 0,
            POST,
            HEAD,
            PUT,
            DELETE,
            TRACE,
            OPTIONS,
            CONNECT,
            PATH
        };

        enum CHECK_STATE{
            CHECK_STATE_REQUESTLINE = 0,
            CHECK_STATE_HEADER,
            CHECK_STATE_CONTENT
        };

        enum HTTP_CODE{
            NO_REQUEST,
            GET_REQUEST,
            BAD_REQUEST,
            NO_RESOURCE,
            FORBIDDEN_REQUEST,
            FILE_REQUEST,
            INTERNAL_ERROR,
            CLOSED_CONNECTION
        };

        enum LINE_STATUS{
            LINE_OK = 0,
            LINE_BAD,
            LINE_OPEN
        };

    public:
        http_conn(){};
        ~http_conn(){};

        void init(int sockfd, const sockaddr_in &addr, char*, int, int, string user, string passwd, string sqlname);
        void close_conn(bool real_close = true);
        void process();
        bool read_once();
        bool write();
        sockaddr_in* get_address(){ return &address; }
        void initmysql_result(connection_pool *connpool);
        int timer_flag;
        int improv;
    
    private:
        void init();
        HTTP_CODE process_read();
        bool process_write(HTTP_CODE ret);
        HTTP_CODE parse_request_line(char* text);
        HTTP_CODE parse_headers(char* text);
        HTTP_CODE parse_content(char* text);
        HTTP_CODE do_request();
        char* get_line(){ return read_buff + start_line; }
        LINE_STATUS parse_line();
        void unmap();
        bool add_reponse(const char* format, ...);
        bool add_content(const char* content);
        bool add_status_line(int status, const char* title);
        bool add_headers(int content_len);
        bool add_content_type();
        bool add_content_len(int content_len);
        bool add_linger();
        bool add_blank_line();
    
    public:
        static int epollfd;
        static int user_cnt;
        MYSQL* mysql;
        int state; // read=0, write=1

    private:
        int sockfd;
        sockaddr_in address;
        char read_buff[READ_BUFFER_SIZE];
        int read_idx;
        int checked_idx;
        int start_line;
        char write_buff[WRITE_BUFFER_SIZE];
        int write_idx;
        CHECK_STATE check_state;
        METHOD method;
        char real_file[FILENAME_LEN];
        char* url;
        char* version;
        char* host;
        int content_length;
        bool linger;
        char* file_addr;
        struct stat file_stat;
        struct iovec iv[2];
        int iv_cnt;
        int cgi;
        char* str;
        int bytes_to_send;
        int bytes_have_send;
        char* doc_root;

        map<string, string> users;
        int TRIMode;
        int close_log;

        char sql_user[100];
        char sql_passwd[100];
        char sql_name[100];
};
#endif
