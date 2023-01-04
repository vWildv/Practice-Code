#include <webserver.h>

webserver::webserver(){
    users = new http_conn[MAX_FD];

    char server_path[200];
    getcwd(server_path, 200);
    char troot[6] = "/root";
    root = (char*) malloc(strlen(server_path) + strlen(troot) + 1);
    strcpy(root, server_path);
    strcat(root, troot);

    users_timer = new client_data[MAX_FD];
}

webserver::~webserver(){
    close(epollfd);
    close(listenfd);
    close(pipefd[1]);
    close(pipefd[0]);
    delete[] users;
    delete[] users_timer;
    delete pool;    
}

void webserver::init(int port, string user, string passwd, string databasename, int log_write, int opt_linger, int trimode, int sql_num, int thread_num, int close_log, int actor_model){
    this->port = port;
    this->user = user;
    this->passwd = passwd;
    this->databasename = databasename;
    this->sql_num = sql_num;
    this->thread_num = thread_num;
    this->log_write = log_write;
    this->OPT_LINGER = opt_linger;
    this->TRIMode = trimode;
    this->close_log = close_log;
    this->actor_model = actor_model;
}

void webserver::tri_mode(){
    //LT LT
    if(TRIMode == 0){
        LISTENTRIMode = 0;
        CONNTRIMode = 0;
    }
    //LT ET
    else if(TRIMode == 1){
        LISTENTRIMode = 0;
        CONNTRIMode = 1;
    }
    //ET LT
    else if(TRIMode == 2){
        LISTENTRIMode = 1;
        CONNTRIMode = 0;
    }
    //ET ET
    else if(TRIMode == 3){
        LISTENTRIMode = 1;
        CONNTRIMode = 1;
    }
}

void webserver::log_writer(){
    return;
}

void webserver::sql_pool(){
    connpool = connection_pool::get_instance();
    connpool->init("localhost",user, passwd, databasename, 3306, sql_num, close_log);
    users->initmysql_result(connpool);
}

void webserver::thread_pool(){
    pool = new threadpool<http_conn>(actor_model, connpool, thread_num);
}

void webserver::event_listen(){
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    if(OPT_LINGER == 0){
        struct linger tmp = {0, 1};
        setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }
    else if(OPT_LINGER == 1){
        struct linger tmp = {1, 1};
        setsockopt(listenfd, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));
    }

    int ret = 0;
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int flag = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(listenfd, (struct sockaddr*) &addr, sizeof(addr));
    //printf("%d\n",ret);
    assert(ret >= 0);
    ret = listen(listenfd, 5);
    assert(ret >= 0);

    utls.init(TIMESLOT);

    epoll_event events[MAX_EVENT_NUMBER];
    epollfd = epoll_create(5);
    assert(epollfd != -1);

    utls.addfd(epollfd, listenfd, false, LISTENTRIMode);
    http_conn::epollfd = epollfd;

    ret = socketpair(PF_UNIX,SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    utls.setnonblocking(pipefd[1]);
    utls.addfd(epollfd, pipefd[0], false, 0);
    utls.add_sig(SIGPIPE, SIG_IGN);
    utls.add_sig(SIGALRM, utls.sig_handler, false);
    utls.add_sig(SIGTERM, utls.sig_handler, false);

    alarm(TIMESLOT);

    utils::pipefd = pipefd;
    utils::epollfd = epollfd;
}

void webserver::timer(int connfd, struct sockaddr_in client_addr){
    users[connfd].init(connfd, client_addr, root, CONNTRIMode, close_log, user,passwd, databasename);

    users_timer[connfd].addr = client_addr;
    users_timer[connfd].sockfd = connfd;
    util_timer* timer = new util_timer;
    timer->user_data = &users_timer[connfd];
    timer->cb_func = cb_func;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    users_timer[connfd].timer = timer;
    utls.timer_list.add_timer(timer);
}

void webserver::adjust_timer(util_timer* timer){
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    utls.timer_list.adjust_timer(timer);
}

void webserver::deal_timer(util_timer* timer, int sockfd){
    timer->cb_func(&users_timer[sockfd]);
    if(timer) utls.timer_list.del_timer(timer);
}

bool webserver::deal_clientdata(){
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    if(LISTENTRIMode == 0){
        int connfd = accept(listenfd, (struct sockaddr*) &client_addr, &client_addr_len);
        //printf("conn: %d\n",connfd);
        if(connfd < 0){
            return false;
        }
        if(http_conn::user_cnt >= MAX_FD){
            utls.show_error(connfd, "Internal server busy");
            return false;
        }
        timer(connfd, client_addr);
    }
    else{
        while(1){
            int connfd = accept(listenfd, (struct sockaddr*) &client_addr, &client_addr_len);
            if(connfd < 0){
                break;
            }
            if(http_conn::user_cnt >= MAX_FD){
                utls.show_error(connfd, "Internal server busy");
                break;
            }
            timer(connfd, client_addr);
        }
        return false;
    }
    return true;
}

bool webserver::deal_sig(bool &timeout, bool &stop_server){
    int ret = 0;
    int sig;
    char sigs[1024];
    ret = recv(pipefd[0], sigs, sizeof(sigs), 0);
    if(ret == -1 || ret == 0) return false;
    else{
        for(int i = 0; i < ret; i++){
            switch(sigs[i]){
            case SIGALRM:
                timeout = true;
                break;
            
            case SIGTERM:
                stop_server = true;
                break;
            }
        }
    }
    return true;
}

void webserver::deal_read(int sockfd){
    util_timer* timer = users_timer[sockfd].timer;
    
    //reactor
    if(actor_model == 1){
        if(timer) adjust_timer(timer);
        pool->append(users + sockfd, 0);
        while(1){
            if(users[sockfd].improv){
                if(users[sockfd].timer_flag == 1){
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    //proactor
    else{
        bool f = users[sockfd].read_once();
        //printf("f: %d, sockfd: %d\n",f,sockfd);
        if(f){
            pool->append_p(users + sockfd);
            if(timer) adjust_timer(timer);
        }
        else{
            deal_timer(timer, sockfd);
        }
    }
}

void webserver::deal_write(int sockfd){
    util_timer* timer = users_timer[sockfd].timer;

    //reactor
    if(actor_model == 1){
        if(timer) adjust_timer(timer);
        pool->append(users + sockfd, 1);
        while(1){
            if(users[sockfd].improv){
                if(users[sockfd].timer_flag == 1){
                    deal_timer(timer, sockfd);
                    users[sockfd].timer_flag = 0;
                }
                users[sockfd].improv = 0;
                break;
            }
        }
    }
    //proactor
    else{
        if(users[sockfd].write()){
            if(timer) adjust_timer(timer);
        }
        else{
            deal_timer(timer, sockfd);
        }
    }
}

void webserver::event_loop(){
    bool timeout = false;
    bool stop_server = false;

    while(!stop_server){
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER,-1);
        if(number < 0 && errno != EINTR){
            break;
        }
        for(int i = 0; i < number; i++){
            int sockfd = events[i].data.fd;

            if(sockfd == listenfd){
                //printf("connect!!!\n");
                bool flag = deal_clientdata();
                if(flag == false) continue;
            }

            else if(events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)){
                util_timer* timer = users_timer[sockfd].timer;
                deal_timer(timer, sockfd);
            }

            else if((sockfd == pipefd[0]) && (events[i].events & EPOLLIN)){
                bool flag = deal_sig(timeout, stop_server);
            }

            else if(events[i].events & EPOLLIN){
                //printf("read!!\n");
                deal_read(sockfd);
            }

            else if(events->events & EPOLLOUT){
                deal_write(sockfd);
            }
        }
        if(timeout){
            utls.timer_handler();
            timeout = false;
        }
    }
}