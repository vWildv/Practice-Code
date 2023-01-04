#include <webserver.h>
using namespace std;

class config{
    public:
        int PORT;
        int LOGWrite;
        int TRIGMode;
        int LISTENTrigmode;
        int CONNTrigmode;
        int OPT_LINGER;
        int sql_num;
        int thread_num;
        int close_log;
        int actor_model;

        config(){
            PORT = 1213;
            LOGWrite = 0;
            //listenfd LT + connfd LT 0
            TRIGMode = 0;
            //listenfd LT 0, ET 1
            LISTENTrigmode = 0;
            //connfd LT 0, ET 1
            CONNTrigmode = 0;
            //优雅关闭链接，默认不使用
            OPT_LINGER = 0;
            sql_num = 8;
            thread_num = 8;
            close_log = 0;
            //proactor 0, reactor 1
            actor_model = 0;
        };
        ~config(){};
        void parse_arg(int argc, char* argv[]){
            int opt;
            const char *str = "p:l:m:o:s:t:c:a:";
            while ((opt = getopt(argc, argv, str)) != -1){
                switch (opt){
                case 'p':
                    PORT = atoi(optarg);
                    break;
                case 'l':
                    LOGWrite = atoi(optarg);
                    break;
                case 'm':
                    TRIGMode = atoi(optarg);
                    break;
                case 'o':
                    OPT_LINGER = atoi(optarg);
                    break;
                case 's':
                    sql_num = atoi(optarg);
                    break;
                case 't':
                    thread_num = atoi(optarg);
                    break;
                case 'c':
                    close_log = atoi(optarg);
                    break;
                case 'a':
                    actor_model = atoi(optarg);
                    break;
                default:
                    break;
                }
            }
        }
};

int main(int argc, char* argv[]){
    string user = "root";
    string passwd = "root";
    string databasename = "webdb";

    config cfig;
    cfig.parse_arg(argc, argv);

    webserver server;
    server.init(cfig.PORT, user, passwd, databasename, cfig.LOGWrite, 
                cfig.OPT_LINGER, cfig.TRIGMode, cfig.sql_num, cfig.thread_num, 
                cfig.close_log, cfig.actor_model);
    server.log_writer();
    server.sql_pool();
    server.thread_pool();
    server.tri_mode();
    server.event_listen();
    server.event_loop();
    return 0;
}