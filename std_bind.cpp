/*************************************************************************
    > File Name: std_bind.cpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月20日 星期一 12时09分52秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include<functional>
class A;

class A {
    public:
        virtual void f() {
            std::cout<<"A::f()"<<std::endl;
        }

        void init() {
            std::function<void ()> f = std::bind(&A::f,this);
            f();
        }
};

class B:public A{
    public:
        virtual void f() {
            std::cout<<"B::f()"<<std::endl;
        }
};

int main( int argc , char * argv[] )  {
    A* pa = new B();
    pa->init();
    return EXIT_SUCCESS;
}

