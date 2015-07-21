/*************************************************************************
    > File Name: TestThreadPool.cpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月21日 星期二 11时00分48秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include"ThreadPool.cpp"

void TestThreadPool()  {
    ThreadPool pool(2);

    std::thread thd1([&pool] {
                for(int i = 0 ; i < 10 ; i++ )  {
                    auto thdId = std::this_thread::get_id();
                    pool.AddTask([thdId]{std::cout<<"同步层线程1的线程ID:"<<thdId<<std::endl; });
                }
    });

    std::thread thd2([&pool] {
                for(int i = 0 ; i < 10 ; i++ )  {
                    auto thdId = std::this_thread::get_id();
                    pool.AddTask([thdId] { std::cout<<"同步层线程2的线程ID:"<<thdId<<std::endl; });
                }
    });

    std::this_thread::sleep_for(std::chrono::seconds(2));
    getchar();
    pool.Stop();
    thd1.join();
    thd2.join();
};
int main( int argc , char * argv[] )  {
    TestThreadPool();
    return EXIT_SUCCESS;
}

