/*************************************************************************
    > File Name: std_function.cpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月20日 星期一 10时57分12秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include<functional>

typedef std::function<void ()> fp;

void g_fun()  {
    std::cout<<"g_fun()"<<std::endl;
}

class A {
    public:
        static void A_fun_static()  {
            std::cout<<"A_fun_static()"<<std::endl;
        }
        void A_fun()  {
            std::cout<<"A_fun()" <<std::endl;
        }
        void A_fun_int(int i)  {
            std::cout<<"A_fun_int()"<<i<<std::endl;
        }

        void init()  {
            fp fp1 = std::bind(&A::A_fun,this);
            fp1();
        }

        void init2()  {
            typedef std::function<void (int)> fpi;
            fpi f = std::bind(&A::A_fun_int,this,std::placeholders::_1);
            f(5);
        }
};

int main( int argc , char * argv[] )  {
    fp f2 = fp(&g_fun);
    f2();

    fp f1 = fp(&A::A_fun_static);
    f1();

    A().init();
    A().init2();

    return EXIT_SUCCESS;
}

