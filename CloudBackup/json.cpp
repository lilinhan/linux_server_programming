/*************************************************************************
    > File Name: json.cpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年08月02日 星期日 16时23分03秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include"jsoncpp-src-0.5.0/include/json/json.h"
int main( int argc , char * argv[] )  {
    //Json::Value root;

    //root["name"] = Json::Value("lewin");
    //root["age"] = Json::Value(23);
    //root["sex_is_male"] = Json::Value(true);
    //Json::StyledWriter sw;
    //std::cout << sw.write(root) << std::endl;

    const char * ptr = "{\"uploadid\": \"UP000000\",\"code\": 100,\"msg\": \"\",\"files\": \"\"}";
    Json::Reader reader;
    Json::Value root1;
    if(reader.parse(ptr,root1)) {
        std::string uploadid = root1["uploadid"].asString();
        int code = root1["code"].asInt();
        std::cout << uploadid << "\t" << code << "\t" << std::endl;
    }

    return EXIT_SUCCESS;
}

