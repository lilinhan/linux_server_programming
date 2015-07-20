/*************************************************************************
    > File Name: threadpool.h
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月20日 星期一 13时48分51秒
 ************************************************************************/
#ifndef THREADPOOL_H
#define THREADPOOL_H

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include<pthread>
#include<cassert>
#include<cstdio>
#include<deque>

class ThreadPool  {
        unsigned int threadNum;         //线程池中开启线程的个数
        unsigned int maxJobNum;         //队列中Job的最大值
        deque<Job> jobQueue；           //任务队列
        pthread_t *pthreads;            //线程池中所有线程的pthread_t
        pthread_mutex_t mutex;          //互斥信号量
        pthread_cond_t condQueueEmpty;  //队列为空的条件
        pthread_cond_t condQueueFull;   //队列为满的条件
        bool isPoolClose;               //线程池是否关闭

        public:
                ThreadPool(int threadNum = 10 , int maxJobNUm=100);
                int addJob(Job job);
                int destory();
                ~ThreadPool();
                friend void * run (void *arg);
};

#endif
