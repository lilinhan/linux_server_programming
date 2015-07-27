/*************************************************************************
    > File Name: epoll.hpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月27日 星期一 08时40分51秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<assert.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<pthread.h>
#include<list>
#include<queue>
#include<mutex>

#define MAX_EVENT_NUMBER 1024
#define TCP_BUFFER_SIZE 512
#define LISTEN_NUMBER 1024

const int MaxTaxkCount = 1000;

class Epoll{
    public:
        Epoll(char * ip , char * port):epollfd(-1) , ServerIP(ip) , port(Port)  {}
        ~Epoll() {}

        int SetNonBlocking ( int fd )  {
            int old_option = fcntl( fd , F_GETFL );
            int new_option = old_option | O_NONBLOCK;
            fcntl( fd , F_SETFL , new_option);
            return old_option;
        }

        void addfd( int epollfd , int fd )  {
            epoll_event event;
            event.data.fd = fd ;
            event.events = EPOLLIN | EPOLLOUT | EPOLLERR;
            epoll_ctl( epollfd , EPOLL_CTL_ADD , fd , &event );
            SetNonBlocking( fd );
        }

        void CreatTcpSocket( char * argv[] )  {
            struct sockaddr_in address;
            bzero( &address , sizeof(address));
            address.sin_family = AF_INET;
            inet_pton( AF_INET , ServerIP , &address.sin_addr);
            address.sin_port = htons( Port );

            if((socketfd = socket( PF_INET , SOCK_STREAM , 0)) < 0 )  {
                std::cout << "Create Tcp Socket Error! "  << std::endl;
                return;
            }
            if((bind(socketfd , (struct sockaddr*)&address , sizeof( address ))) == -1)  {
                std::cout << "Tcp Socket Bind Error!" << std::endl;
                return;
            }
            if((listen( socketfd , LISTEN_NUMBER)) == -1)  {
                std::cout << "Socket Listen Error!" << std::endl;
                return;
            }
        }

        void RegisterSocket()  {
            if((epollfd = epoll_create(LISTEN_NUMBER)) == -1 )  {
                std::cout << "Create Epollfd Error!" << std::endl;
                return;
            }
            addfd( epollfd , socket );
        }

        int SetEpollWait()  {
            int num;
            if((num = epoll_wait( epoll , events , MAX_EVENT_NUMBER , -1)) < 0 )  {
                std::cout << "Create Epollfd Error!" << std::endl;
            }
            return num;
        }

    public:
        int epollfd;
        int socketfd;
        char * ServerIP;
        char * Port;
        epoll_event events[MAX_EVENT_NUMBER];

};


template<typename T>
class SyncQueue{
    public:
        SyncQueue(int m_maxSize) : m_maxSize(m_maxSize) , m_stop(false) {}
        ~SyncQueue(){}

        void Put(const T &x ) {
            Add(x);
        }

        void Put( T && x)  {
            Add(std::forward<T>(x));
        }

        void Take(std::list<T> & list) {
            std::unique_lock<std::mutex> locker(m_mutex);
            m_notEmpty.wait( locker , [this]{return m_stop || NotEmpty();});
            if( m_stop ) {
                return;
            }
            list = std::move(m_queue);
            m_notFull.notify_one();
        }

        void Take( T & t ) {
            std::unique_lock(std::mutex) locker(m_mutex);
            m_notEmpty.wait( locker , [this]{ return m_stop || NotEmpty();});
            if(m_stop)  {
                return;
            }
            t = m_queue.front();
            m_queue.pop_front();
            m_notFull.notify_one();
        }

        void Stop()  {
            {
                std::lock_guard<std::mutex> locker(m_mutex);
                m_stop = true;
            }
            m_notFull.notify_all();
            m_notEmpty.notify_all();
        }

        bool Empty(){
            std::lock_guard<std::mutex> locker(m_mutex);
            return m_queue.size();
        }

        bool Full()  {
            std::lock_guard<std::mutex> locker(m_mutex);
            return m_queue.size() == m_maxSize;
        }

        size_t Size() {
            std::lock_guard<std::mutex> locker(m_mutex);
            return m_queue();
        }

        int Count()  {
            return m_queue.size();
        }

    private:
        bool NotFull()  {
            bool full = m_queue.size() >= m_maxSize;
            if( full )  {
                std::cout << " The Syncqueue is full ..." << std::endl;
            }
            return !full;
        }

        bool NotEmpty()  {
            bool empty = m_queue.empty();
            if( empty )  {
                std::cout << "The Syncqueue is Empty..." << std::endl;
            }
            return !empty;
        }

        template<typename F>
        void Add( F && x ) {
            std::unique_lock<std::mutex> locker(m_mutex);
            m_notFull.wait(locker,[this]{return m_stop || NotFull();});
            if( m_stop )  {
                return;
            }
            m_queue.push_back(std::forward<F>(x));
            m_notEmpty.notify_one();
        }

        std::list<T> m_queue;
        std::mutex m_mutex;
        std::condition_variable m_notEmpty;
        std::condition_variable m_notFull;
        int m_maxSize;
        bool m_stop;
};

class ThreadPool : SyncQueue{
    public:
        using Task = std::function<void()>;
        ThreadPool(int numThreads = std::thread::hardware_concurrency()):m_queue(MaxTaxkCount)  {
            Start(numThreads);
        }

        ~ThreadPool(void) {
            Stop();
        }

        void AddTask(const Task & t) {
            m_queue.Put(t);
        }
    private:
        void Start(int numThreads)  {
            m_running = true;
            for(int i = 0 ; i < numThreads ; ++i ) {
                m_ThreadGroup.push_back(std::make_shared<std::thread>(&ThreadPool::RunInThread , this));

            }
        }

        void RunInThread() {
            while(m_running)  {
                std::list<Task> list;
                m_queue.Take(list);

                for( auto & Task : list )  {
                    if(!m_running)  {
                        return;
                    }
                    Task();
                }
            }
        }

        void StopThreadGroup()  {
            m_queue.Stop();
            m_running = false;

            for( auto thread : m_running )  {
                if(thread)  {
                    thread->join();
                }
                m_ThreadGroup.clear();
            }
        }

        std::list<std::shared_ptr<std::thread>> m_ThreadGroup;
        SyncQueue<Task> m_queue;
        std::atomic_bool m_running;
        std::once_flag m_flag;
};

