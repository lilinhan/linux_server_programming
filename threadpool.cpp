/*************************************************************************
    > File Name: threadpool.cpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月20日 星期一 14时05分06秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include"threadpool.h"

void *run(void * arg)  {
        ThreadPool *pool = (ThreadPool *)arg;
        struct Job job;

        while(1) {    //死循环
                pthread_mutex_lock(&(pool->mutex));

                if(pool->isPoolClose)  {    //先判断线程池是否关闭，关闭线程就退出
                        pthread_mutex_unlock(&(pool->mutex));
                        pthread_exit(NULL);
                }

                while((pool->jobQueue.empty()))  {  //任务队列为空时，等待队列非空
                        pthread_cond_wait(&(pool->condQueueEmpty) , &(pool->mutex));
                        if(pool->isPoolClose)  {  //线程池关闭，线程就退出
                                pthread_mutex_unlock(&(pool->mutex));
                                pthread_exit(NULL);
                        }
                }

                job = pool->jobQueue.front();
                pool->jobQueue.pop_front();
                if(pool->jobQueue.size() == (pool->maxJobNum - 1))  {
                        pthread_cond_broadcast(&(pool->condQueueFull));
                }
                pthread_mutex_unlock(&(pool->mutex));

                (*(job.doJobFun))(job.arg);
        }
}

ThreadPool::ThreadPool(int threadNUm , int maxJobNum)  {
        this->threadNum = threadNum;
        this->maxJobNum = maxJobNum;

        if(pthread_mutex_init(&(this->mutex) , NULL))  {
                perror("pthread_mutex_init");
                exit(1);
        }

        if(pthread_cond_init(&(this->condQueueEmpty) , NULL))  {
                perror("pthread_cond_init");
                exit(1);
        }

        if(pthread_cond_init(&(this->condQueueFull) , NULL))  {
                perror("pthread_cond_init");
                exit(1);
        }

        this->pthreads = (pthread_t *)malloc(sizeof(pthread_t) * this->threadNUm);
        if( NULL == this->pthreads)  {
                perror("pthreads malloc");
                exit(1);
        }

        this->isPoolClose = false;
        unsigned int i;
        for (i = 0 ; i < this->threadNum ; ++i)  {
                pthread_create(&(this->pthreads[i]) , NULL , run , (void *)this);
        }
}

int ThreadPool::addJob(Job job) {
        assert( job.doJobFun != NULL );
        if(this->isPoolClose)  {
               return -1 ;
        }
        pthread_mutex_lock(&(this->mutex));
        while((this->jobQueue.size() == this->maxJobNum))  {
                pthread_cond_wait(&(this->condQueueFull) , &(this->mutex));
        }
        if(this->isPoolClose)  {
                pthread_mutex_unlock(&(this->mutex));
                return -1;
        }

        if(this->jobQueue.empty())  {
                this->jobQueue.push_back(job);
                pthread_cond_broadcast(&(this->condQueueEmpty));
        }else  {
                this->jobQueue.push_back(job);
        }

        pthread_mutex_unclock(&(this->mutex));

        return 0;
}

int ThreadPool::destory()  {
        pthread_mutex_lock(&(this->mutex));
        this->isPoolClose = true;
        this->jobQueue.clear();

        pthread_mutex_unclock(&(this->mutex));
        pthread_cond_broadcast(&(this->condQueueEmpty));
        pthread_cond_broadcast(&(this->condQueueFull));

        unsigned int i ;
        for( i = 0 ; i < this->threadNum ; ++i )  {
                pthread_join(this->pthreads[i] , NULL );
        }

        pthread_mutex_destory(&(this->mutex));
        pthread_cond_destory(&(this->condQueueEmpty));
        pthread_cond_destory(&(this->condQueueFull));
        free(this->pthreads);

        return 0;
}

ThreadPool::~ThreadPool()  {

}


int main( int argc , char * argv[] )  {

    return EXIT_SUCCESS;
}

