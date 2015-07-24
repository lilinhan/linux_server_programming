/*************************************************************************
    > File Name: test.cpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月21日 星期二 14时24分35秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include<thread>

void hello()
{
    std::cout << "Hello Concurrent World\n";
}

int main( int argc , char * argv[] )
{
    std::thread t(hello);
    t.join();
    return EXIT_SUCCESS;
}

