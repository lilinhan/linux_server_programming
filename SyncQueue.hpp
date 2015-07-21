/*************************************************************************
    > File Name: SyncQueue.cpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月20日 星期一 08时25分27秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include<list>
#include<mutex>
#include<thread>
#include<condition_variable>
#include<iostream>

template<typename T>

class SyncQueue  { //排队层，是一个同步队列，允许多个线程同时去添加或者取出任务
    public:
        SyncQueue(int maxSize) : m_maxSize(maxSize) , m_needStop(false) {}

        void Put(const T&x) {   //向任务队列添加任务
            Add(x);
        }

        void Put(T&&x)  {
            Add(std::forward<T>(x));    //将根据x的左右值来确定参数的左右值，forward完美转发问题
        }

        void Take(std::list<T>& list)  {  //取任务列表
            std::unique_lock<std::mutex> locker(m_mutex);  //先抢锁
            m_notEmpty.wait( locker , [this]{ return m_needStop || NotEmpty(); });  //使用条件变量，当共享资源达到某个条件时，继续执行

            if( m_needStop )  {   //任务队列停止，则退出
                return;
            }
            list = std::move(m_queue);  //将左值转化成右值
            m_notFull.notify_one();     //通知一个线程，任务队列没有满
        }

        void Take( T & t )  {   //取任务
            std::unique_lock<std::mutex> locker(m_mutex);  //先抢锁
            m_notEmpty.wait(locker,[this]{ return m_needStop || NotEmpty(); });   //等待满足条件的条件变量

            if(m_needStop)      //队列停止就结束
                return;

            t = m_queue.front();        //从任务队列中取出一个任务
            m_queue.pop_front();        //将任务从队列中删除
            m_notFull.notify_one();     //通知一个线程，任务队列不满
        }

        void Stop()  {      //停止任务队列
            {
                std::lock_guard<std::mutex> locker(m_mutex);  //加锁
                m_needStop = true;      //设置停止
            }
            m_notFull.notify_all();     //通知所有线程
            m_notEmpty.notify_all();    //通知所有线程
        }

        bool Empty()  {   //判断是否为空
            std::lock_guard<std::mutex> locker(m_mutex);   //先加锁
            return m_queue.empty();     //输出信息
        }

        bool Full()  {  //判断是否是满的
            std::lock_guard<std::mutex> locker(m_mutex);  //先加锁
            return m_queue.size() == m_maxSize;     //返回是否是满的
        }

        size_t Size()  {    //返回大小
            std::lock_guard<std::mutex> locker(m_mutex); //先加锁
            return m_queue.size();  //返回队列大小
        }

        int Count()  {
            return m_queue.size();   //返回队列大小
        }

    private:
        bool NotFull()  {  //判断队列是否为满
            bool full = m_queue.size() >= m_maxSize;
            if( full )  {
                std::cout<<"full ....."<<std::endl;
            }
            return !full;
        }

        bool NotEmpty() const  {  //判断队列是否为空
            bool empty = m_queue.empty();
            if( empty )  {
                std::cout<<"empty...."<<std::endl;
            }
            return !empty;
        }

        template<typename F>

        void Add( F && x )  {   //向队列中添加任务
            std::unique_lock< std::mutex >  locker(m_mutex);   //先抢锁
            m_notFull.wait(locker,[this]{ return m_needStop || NotFull(); });   //条件变量
            if(m_needStop)  {  //如果是停止状态，返回
                return;
            }
            m_queue.push_back(std::forward<F>(x));  //将任务添加到队列
            m_notEmpty.notify_one();        //通知已经有任务
        }

    private:
        std::list<T> m_queue;       //任务队列
        std::mutex m_mutex;     //锁
        std::condition_variable m_notEmpty;         //不为空的条件变量
        std::condition_variable m_notFull;          //不为满的条件变量
        int m_maxSize;      //任务队列的最大的大小
        bool m_needStop;        //停止的标志
};


