#include <http_conn.h>

#include <mysql/mysql.h>
#include <fstream>

const char* OK_200_title = "OK";
const char* ERROR_400_title = "Bad Request";
const char* ERROR_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char* ERROR_403_title = "Forbidden";
const char* ERROR_403_form = "You do not have permission to get file form this server.\n";
const char* ERROR_404_title = "Not Found";
const char* ERROR_404_form = "The requested file was not found on this server.\n";
const char* ERROR_500_title = "Internal Error";
const char* ERROR_500_form = "There was an unusual problem serving the request file.\n";

locker lock;
map<string, string> users;

void http_conn::initmysql_result(connection_pool* connpool){
    MYSQL *mysql = NULL;
    connectionRAII mysqlcon(&mysql, connpool);

    if(mysql_query(mysql, "SELECT username,passwd FROM user")){
        printf("SELECT error:%s\n", mysql_error(mysql));
    }

    MYSQL_RES* result = mysql_store_result(mysql);
    int num_fields = mysql_num_fields(result);
    MYSQL_FIELD* feilds  = mysql_fetch_field(result);

    while(MYSQL_ROW row = mysql_fetch_row(result)){
        string tmp1(row[0]);
        string tmp2(row[1]);
        users[tmp1] = tmp2;
    }
}

int setnonblocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addfd(int epollfd, int fd, bool one_shot, int TRIGMode){
    epoll_event event;
    event.data.fd = fd;
    if(TRIGMode == 1) event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
    else event.events = EPOLLIN | EPOLLRDHUP;

    if(one_shot) event.events |= EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void removefd(int epollfd, int fd){
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void modfd(int epollfd, int fd, int ev, int TRIGMode){
    epoll_event event;
    event.data.fd = fd;

    if (1 == TRIGMode)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int http_conn::user_cnt = 0;
int http_conn::epollfd = -1;

void http_conn::close_conn(bool real_close){
    if(real_close && (sockfd!=-1)){
        //printf("close %d\n", sockfd);
        removefd(epollfd, sockfd);
        sockfd = -1;
        user_cnt--;
    }
}

void http_conn::init(int sockfd, const sockaddr_in &addr, char* root, int TRIMode, int cose_log, string user, string passwd, string sqlname){
    this->sockfd = sockfd;
    this->address = addr;

    addfd(epollfd, sockfd, true, TRIMode);
    user_cnt++;

    doc_root = root;
    this->TRIMode = TRIMode;
    this->close_log = close_log;

    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name, sqlname.c_str());

    init();
}

void http_conn::init(){
    mysql = NULL;
    bytes_to_send = 0;
    bytes_have_send = 0;
    check_state = CHECK_STATE_REQUESTLINE;
    linger = false;
    method = GET;
    url = 0;
    version = 0;
    content_length = 0;
    host = 0;
    start_line = 0;
    checked_idx = 0;
    read_idx = 0;
    write_idx = 0;
    cgi = 0;
    state = 0;
    timer_flag = 0;
    improv = 0;

    memset(read_buff, 0, READ_BUFFER_SIZE);
    memset(write_buff, 0, WRITE_BUFFER_SIZE);
    memset(real_file, 0, FILENAME_LEN);
}

http_conn::LINE_STATUS http_conn::parse_line(){
    for(char tmp; checked_idx < read_idx; checked_idx++){
        tmp = read_buff[checked_idx];
        if(tmp == '\r'){
            if((checked_idx + 1) == read_idx) return LINE_OPEN;
            else if(read_buff[checked_idx + 1] == '\n'){
                read_buff[checked_idx++] = 0;
                read_buff[checked_idx++] = 0;
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if(tmp == '\n'){
            if(checked_idx > 1 && read_buff[checked_idx - 1] == '\r'){
                read_buff[checked_idx-1] = 0;
                read_buff[checked_idx++] = 0;
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_OPEN;
}

bool http_conn::read_once(){
    if(read_idx >= READ_BUFFER_SIZE) return false;
    int bytes_read = 0;
    
    //LT
    if(TRIMode == 0){
        //printf("sockfd: %d, ridx: %d\n",sockfd, read_idx);
        bytes_read = recv(sockfd, read_buff + read_idx, READ_BUFFER_SIZE - read_idx, 0);
        //printf("br: %d\n",bytes_read);
        read_idx += bytes_read;

        return bytes_read > 0;
    }
    //ET
    else{
        while(true){
            bytes_read = recv(sockfd, read_buff + read_idx, READ_BUFFER_SIZE - read_idx, 0);
            if(bytes_read == -1){
                if(errno == EAGAIN || errno == EWOULDBLOCK) break;
                return false;
            }
            else if(bytes_read == 0) return false;
            read_idx += bytes_read;
        }
        return true;
    }
}

http_conn::HTTP_CODE http_conn::parse_request_line(char* text){
    url = strpbrk(text, " \t");
    if(!url) return BAD_REQUEST;
    *url++ = 0;

    char* meth = text;
    if(strcasecmp(meth, "GET") == 0) method = GET;
    else if(strcasecmp(meth, "POST") == 0) method = POST, cgi = 1;
    else return BAD_REQUEST;

    url += strspn(url, " \t");
    version = strpbrk(url, " \t");
    if(!version) return BAD_REQUEST;
    *version++ = 0;
    version += strspn(version, " \t");
    if (strcasecmp(version, "HTTP/1.1") != 0) return BAD_REQUEST;

    if(strncasecmp(url, "http://", 7) == 0){
        url += 7;
        url = strchr(url, '/');
    }
    if(strncasecmp(url, "https://", 7) == 0){
        url += 8;
        url = strchr(url, '/');
    }

    if(!url || url[0] != '/') return BAD_REQUEST;
    if(strlen(url) == 1) strcat(url, "judge.html");
    check_state = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::parse_headers(char* text){
    if(text[0] == 0){
        if(content_length != 0){
            check_state = CHECK_STATE_CONTENT;
            return NO_REQUEST;
        }
        return GET_REQUEST;
    }
    else if(strncasecmp(text, "Connection:", 11) == 0){
        text += 11;
        text += strspn(text, " \t");
        if(strcasecmp(text, "keep-alive") == 0) linger = true;
    }
    else if (strncasecmp(text, "Content-length:", 15) == 0){
        text += 15;
        text += strspn(text, " \t");
        content_length = atol(text);
    }
    else if (strncasecmp(text, "Host:", 5) == 0){
        text += 5;
        text += strspn(text, " \t");
        host = text;
    }
    else{
        printf("unkonwn header: %s\n", text);
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::parse_content(char* text){
    if(read_idx >= (content_length + checked_idx)){
        text[content_length] = '\0';
        str = text;
        return GET_REQUEST;
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::process_read(){
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret  = NO_REQUEST;
    char* text = 0;

    while((check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) || ((line_status = parse_line()) == LINE_OK)){
        text = get_line();
        start_line = checked_idx;
        switch(check_state){
        case CHECK_STATE_REQUESTLINE:
            ret = parse_request_line(text);
            if(ret == BAD_REQUEST) return BAD_REQUEST;
            break;
        case CHECK_STATE_HEADER:
            ret = parse_headers(text);
            if(ret == BAD_REQUEST) return BAD_REQUEST;
            else if(ret == GET_REQUEST) return do_request();
            break;
        case CHECK_STATE_CONTENT:
            ret = parse_content(text);
            if(ret == GET_REQUEST) return do_request();
            line_status = LINE_OPEN;
            break;
        default:
            return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::do_request(){
    strcpy(real_file, doc_root);
    int len =  strlen(doc_root);
    const char* p = strrchr(url, '/');

    if(cgi && (p[1] == '0' || p[1] == '1')){
        char flag = url[1];
        
        char *url_real = (char*) malloc(sizeof(char) * 200);
        strcpy(url_real, "/");
        strcat(url_real, url+2);
        strncpy(real_file + len, url_real, FILENAME_LEN - len - 1);
        free(url_real);

        char name[200], passwd[100];
        int i;
        for(i = 5; str[i] != '&'; i++) name[i - 5] = str[i];
        name[i - 5] = '\0';

        int j = 0;
        for(i = i + 10; str[i] != '\0'; i++,j++) passwd[j] = str[i];
        passwd[j] = '\0';

        if(p[1] == '1'){
            char* sql_insert = (char*) malloc(sizeof(char) * 200);
            strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
            strcat(sql_insert, "'");
            strcat(sql_insert, name);
            strcat(sql_insert, "', '");
            strcat(sql_insert, passwd);
            strcat(sql_insert, "')");

            if (users.find(name) == users.end()){
                lock.lock();
                int res = mysql_query(mysql, sql_insert);
                users.insert(make_pair(name, passwd));
                lock.unlock();

                if(!res) strcpy(url, "/log.html");
                else strcpy(url, "/registerError.html");
            }
            else
                strcpy(url, "/registerError.html");
        }
        else if(p[1] == '0'){
            if (users.find(name) != users.end() && users[name] == passwd) strcpy(url, "/welcome.html");
            else strcpy(url, "/logError.html");
            printf("%s\n",url);
        }
    }

    if(p[1] == 'r'){
        char *url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(url_real, "/register.html");
        strncpy(real_file + len, url_real, strlen(url_real));

        free(url_real);
    }
    else if(p[1] == 'l'){
        char *url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(url_real, "/log.html");
        strncpy(real_file + len, url_real, strlen(url_real));

        free(url_real);
    }
    else if(p[1] == 'p'){
        char *url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(url_real, "/picture.html");
        strncpy(real_file + len, url_real, strlen(url_real));

        free(url_real);
    }
    else if(p[1] == 'v'){
        char *url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(url_real, "/video.html");
        strncpy(real_file + len, url_real, strlen(url_real));

        free(url_real);
    }
    else strncpy(real_file + len, url, FILENAME_LEN - len - 1);

    if(stat(real_file, &file_stat) < 0) return NO_RESOURCE;
    if (!(file_stat.st_mode & S_IROTH)) return FORBIDDEN_REQUEST;
    if (S_ISDIR(file_stat.st_mode)) return BAD_REQUEST;

    int fd = open(real_file, O_RDONLY);
    file_addr = (char*) mmap(0, file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    return FILE_REQUEST;
}

void http_conn::unmap(){
    if(file_addr){
        munmap(file_addr, file_stat.st_size);
        file_addr = 0;
    }
}

bool http_conn::write(){
    int tmp = 0;
    if(bytes_to_send == 0){
        modfd(epollfd, sockfd, EPOLLIN, TRIMode);
        init();
        return true;
    }

    while(1){
        tmp = writev(sockfd, iv, iv_cnt);

        if(tmp < 0){
            if(errno == EAGAIN){
                modfd(epollfd, sockfd, EPOLLOUT, TRIMode);
                return true;
            }
            unmap();
            return false;
        }

        bytes_have_send += tmp;
        bytes_to_send -= tmp;
        if(bytes_have_send >= iv[0].iov_len){
            iv[0].iov_len = 0;
            iv[1].iov_base = file_addr + (bytes_have_send - write_idx);
            iv[1].iov_len = bytes_to_send;
        }
        else{
            iv[0].iov_base = write_buff + bytes_have_send;
            iv[0].iov_len = iv[0].iov_len - bytes_have_send;
        }

        if(bytes_to_send <= 0){
            unmap();
            modfd(epollfd, sockfd, EPOLLIN, TRIMode);

            if(linger){
                init();
                return true;
            }
            else{
                return false;
            }
        }
    }
}

bool http_conn::add_reponse(const char* format, ...){
    if(write_idx >= WRITE_BUFFER_SIZE) return false;
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(write_buff + write_idx, WRITE_BUFFER_SIZE - 1 - write_idx, format, arg_list);
    if(len >= (WRITE_BUFFER_SIZE - 1 - write_idx)){
        va_end(arg_list);
        return false;
    }
    write_idx += len;
    va_end(arg_list);

    return true;
}

bool http_conn::add_status_line(int status, const char* title){
    return add_reponse("%s %d %s\r\n", "HTTP/1.1", status, title);
}

bool http_conn::add_content_len(int content_len){
    return add_reponse("Content-Length:%d\r\n", content_len);
}

bool http_conn::add_linger(){
    return add_reponse("Connection:%s\r\n", (linger == 1)?"keep-alive":"close");
}

bool http_conn::add_blank_line(){
    return add_reponse("%s", "\r\n");
}

bool http_conn::add_headers(int content_len){
    return add_content_len(content_len) && add_linger() && add_blank_line();
}

bool http_conn::add_content_type(){
    return add_reponse("Content-Type:%d\r\n", "text/html");
}

bool http_conn::add_content(const char* content){
    return add_reponse("%s", content);
}

bool http_conn::process_write(HTTP_CODE ret){
    switch(ret){
    case INTERNAL_ERROR:
        add_status_line(500, ERROR_500_title);
        add_headers(strlen(ERROR_500_form));
        if(!add_content(ERROR_500_form)) return false;
        break;
    case BAD_REQUEST:
        add_status_line(404, ERROR_404_title);
        add_headers(strlen(ERROR_404_form));
        if(!add_content(ERROR_404_form)) return false;
        break;
    case FORBIDDEN_REQUEST:
        add_status_line(403, ERROR_403_title);
        add_headers(strlen(ERROR_403_form));
        if(!add_content(ERROR_403_form)) return false;
        break;
    case FILE_REQUEST:
        add_status_line(200, OK_200_title);
        if(file_stat.st_size != 0){
            add_headers(file_stat.st_size);
            iv[0].iov_base = write_buff;
            iv[0].iov_len = write_idx;
            iv[1].iov_base = file_addr;
            iv[1].iov_len = file_stat.st_size;
            iv_cnt = 2;
            bytes_to_send = write_idx + file_stat.st_size;
            return true;
        }
        else{
            const char* ok_string = "<html><body></body></html>";
            add_headers(strlen(ok_string));
            if(!add_content(ok_string)) return false;
        }
    default:
        return false;
    }

    iv[0].iov_base = write_buff;
    iv[0].iov_len = write_idx;
    iv_cnt = 1;
    bytes_to_send = write_idx;
    return true;
}

void http_conn::process(){
    HTTP_CODE read_ret = process_read();
    if(read_ret == NO_REQUEST){
        modfd(epollfd, sockfd, EPOLLIN, TRIMode);
        return;
    }
    bool write_ret = process_write(read_ret);
    if(!write_ret) close_conn();
    modfd(epollfd, sockfd, EPOLLOUT, TRIMode);
}