/*************************************************************************
    > File Name: ThreadPool.cpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月21日 星期二 09时52分54秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include<list>
#include<thread>
#include<functional>
#include<memory>
#include<atomic>
#include"SyncQueue.hpp"

const int MaxTaskCount = 100;
class ThreadPool  {
    public:
        using Task = std::function<void()>;   //using取代了typedef 类似定义函数指针
        ThreadPool(int numThreads = std::thread::hardware_concurrency()):m_queue(MaxTaskCount)  {  //初始化线程池
            Start(numThreads);   //Start()函数 创建n个线程
        }

        ~ThreadPool(void)  {   //析构函数，没停止时则主动停止线程池
            Stop();
        }

        void Stop()  {      //停止函数
            std::call_once(m_flag,[this]{StopThreadGroup(); });
        }

        void AddTask(const Task & task)  {   //添加任务
            m_queue.Put(task);
        }


    private:
            void Start(int numThreads)  {       //创建线程
                m_running = true;  //开始线程池
                for( int i = 0 ; i < numThreads ; ++i )  {
                    m_threadgroup.push_back(std::make_shared<std::thread>(&ThreadPool::RunInThread , this));  //将线程放入线程池
                }
            }

            void RunInThread()  {    //线程的活动
                while(m_running)  {
                    std::list<Task> list;
                    m_queue.Take(list);  //取一个任务

                    for ( auto & task : list )  {
                        if(!m_running)  {
                            return;
                        }

                        task();
                    }
                }
            }

            void StopThreadGroup()  {   //停止线程队列
                m_queue.Stop();
                m_running = false;

                for(auto thread : m_threadgroup )  {
                    if(thread)  {
                        thread->join();
                    }
                    m_threadgroup.clear();
                }
            }

            std::list<std::shared_ptr<std::thread>> m_threadgroup;
            SyncQueue<Task> m_queue;
            std::atomic_bool m_running;
            std::once_flag m_flag;
};

