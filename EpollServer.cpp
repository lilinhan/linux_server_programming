/*************************************************************************
    > File Name: EpollServer.cpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月23日 星期四 10时24分01秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<cassert>
#include<unistd.h>
#include<errno.h>
#include<fcntl.h>
#include<sys/epoll.h>
#include<thread>
#include<strings.h>
#include<string.h>

#define MAX_EVENT_NUMBER 1024
#define LISTENNUM 5
#define TCP_BUFFER_SIZE 512
#define UDP_BUFFER_SIZE 1024

class Epoll {
    public:
        Epoll():epollfd(-1) {}
        ~Epoll(){}
        //给fd所指的文件设置为非阻塞
        int SetNonBlocking( int fd )  {
            int old_option = fcntl( fd , F_GETFL);  //获取原文件的属性
            int new_option = old_option | O_NONBLOCK;  //添加O_NONBLOCK(非阻塞)属性
            fcntl( fd , F_SETFL , new_option );  //设置原文件句柄设置新的属性
            return old_option;
        }
        //将fd注册监听
        void addfd( int epollfd , int fd )  {
            epoll_event event;   //创建监听事件
            event.data.fd = fd; //监听事件描述符
            event.events = EPOLLIN;  //设置监听可读
            epoll_ctl( epollfd , EPOLL_CTL_ADD , fd , &event ); //注册事件
            SetNonBlocking( fd ); //设置非阻塞
        }

        void CreateTcpSocket(char * argv[])  {  //创建和子服务器链接的TCP SOCKET
            const char * ip = argv[1];
            int port = atoi( argv[2] );
            struct sockaddr_in address; //创建地址协议信息
            bzero( &address , sizeof(address));  //将address中全部赋值为'\0'
            address.sin_family = AF_INET;    //设置TCP/IP协议族
            inet_pton(AF_INET , ip , &address.sin_addr);//将字符串转换为网络地址
            address.sin_port = htons( port );  //将端口转换为网络字节序


            if((TCPSocketFd = socket( PF_INET , SOCK_STREAM , 0 )) < 0)  { //创建tcp套接字，并判断是否合法
                std::cout << "Create TCP Socket Error!" <<std::endl;
                return;
            }
            if ( (bind( TCPSocketFd , (struct sockaddr*)&address , sizeof( address ))) == -1 )  {//将tcp套接字绑定，并判断是否合法
                std::cout << "Tcp Socket Bind Error" << std::endl;
                return;
            }
            if( (listen( TCPSocketFd , LISTENNUM )) == -1 )  { //监听该套接字，并判断是否成功
                std::cout << "Tcp Socket Listen Error!" << std::endl;
            }

        }

        void RegisteredSocket() {  //注册套接字
            if((epollfd = epoll_create(LISTENNUM) )== -1 ) {   //创建epollfd
                std::cout << "Create Epollfd Error!" << std::endl;
                return ;
            }
            addfd( epollfd , TCPSocketFd ); //注册
        }

        int SetUpEpollWait()  { //设置epoll等待
            int num;
            if((num = epoll_wait( epollfd , events , MAX_EVENT_NUMBER , -1)) < 0 )  {
                std::cout << "epoll failure\n" << std::endl;
            }
            return num;
        }

    public:
        int epollfd;        //epoll的文件句柄
        epoll_event events[MAX_EVENT_NUMBER];  //epoll事件列表
        int TCPSocketFd;  //TCP协议的套接字
};

int main( int argc , char * argv[] )  {
    Epoll e;
    int num;
    e.CreateTcpSocket(argv);
    e.RegisteredSocket();
    while(1) {
        num = e.SetUpEpollWait();
        for( int i =0 ; i < num ; ++i )  {
            int socketfd = e.events[i].data.fd;
            if(socketfd == e.TCPSocketFd)  {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( e.TCPSocketFd , ( struct sockaddr*)&client_address , &client_addrlength);
                e.addfd(e.epollfd , connfd);
            }
            else if( e.events[i].events & EPOLLIN )  {
                char buf[ TCP_BUFFER_SIZE ];
                while(1) {
                    memset( buf , '\0' , TCP_BUFFER_SIZE );
                    int ret = recv( socketfd , buf , TCP_BUFFER_SIZE-1 , 0 );
                    if( ret < 0 )  {
                        if((errno == EAGAIN ) || ( errno == EWOULDBLOCK )) {
                            break;
                        }
                        close(socketfd);
                        break;
                    }
                    else if( ret == 0 )  {
                        close(socketfd);
                    }
                    else {
                        send(socketfd , buf , ret , 0);
                    }
                }
            }
            else {
                std::cout << "something else happend \n" << std::endl;
            }
        }
    }
    close(e.TCPSocketFd );
    return EXIT_SUCCESS;
}

