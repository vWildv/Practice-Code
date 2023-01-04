#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <stdio.h>
#include <exception>
#include <pthread.h>

#include <lock.h>
#include <sql_connection_pool.h>

template <typename T>
class threadpool{
    public:
        threadpool(int actor_model, connection_pool* connpool, int thread_number=8, int max_request=10000);
        ~threadpool();
        bool append(T* request, int state);
        bool append_p(T* request);

    private:
        static void* worker(void* arg);
        void run();

    private:
        int thread_number;
        int max_requests;
        pthread_t* threads;
        std::list <T*> workqueue;
        locker queuelocker;
        sem queuestat;
        connection_pool* connpool;
        int actor_model;
};

template <typename T>
threadpool<T>::threadpool(int actor_model, connection_pool* connpool, int thread_number, int max_requests){
    this->actor_model = actor_model;
    this->connpool = connpool;
    this->thread_number = thread_number;
    this->max_requests = max_requests;

    if(thread_number <= 0 || max_requests <= 0) throw std::exception();
    threads = new pthread_t[thread_number];
    if (!threads) throw std::exception();

    for(int i=0;i<thread_number;i++){
        if(pthread_create(threads + i, NULL, worker, this) != 0){
            delete[] threads;
            throw std::exception();
        }
        if(pthread_detach(threads[i]) != 0){
            delete[] threads;
            throw std::exception();
        }
    }
}

template<typename T>
threadpool<T>::~threadpool(){
    delete[] threads;
}

template<typename T>
bool threadpool<T>::append(T *request, int state){
    queuelocker.lock();
    if(workqueue.size() >= max_requests){
        queuelocker.unlock();
        return false;
    }

    request->state = state;
    workqueue.push_back(request);
    queuelocker.unlock();
    queuestat.post();
    return true;
}

template<typename T>
bool threadpool<T>::append_p(T *request){
    queuelocker.lock();
    if(workqueue.size() >= max_requests){
        queuelocker.unlock();
        return false;
    }

    workqueue.push_back(request);
    queuelocker.unlock();
    queuestat.post();
    return true;
}

template<typename T>
void*  threadpool<T>::worker(void* arg){
    threadpool* pool  = (threadpool*) arg;
    pool->run();
    return pool;
}

template <typename T>
void threadpool<T>::run(){
    while(1){
        queuestat.wait();
        queuelocker.lock();

        if(workqueue.empty()){
            queuelocker.unlock();
            continue;
        }

        T* request = workqueue.front();
        workqueue.pop_front();
        queuelocker.unlock();

        if(!request) continue;

        if(actor_model == 1){
            if(request->state == 0){
                if(request->read_once()){
                    request->improv = 1;
                    connectionRAII mysqlcon(&request->mysql, connpool);
                    request->process();
                }
                else{
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
            else{
                if(request->write()) request->improv = 1;
                else{
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        }
        else{
            connectionRAII mysqlconn(&request->mysql, connpool);
            request->process();
        }
    }
}
#endif