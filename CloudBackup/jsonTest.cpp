/*************************************************************************
    > File Name: jsonTest.cpp
    > Author: lewin
    > Mail: lilinhan1303@gmail.com
    > Company:  Xiyou Linux Group
    > Created Time: 2015年07月31日 星期五 10时21分58秒
 ************************************************************************/

#include<iostream>
#include<string>
#include<cstdlib>
#include<cerrno>
#include</usr/include/jsoncpp/json/json.h>

int ParseJsonFromString()  {
    const char * str = "{\"uploadid\":\"UP000000\",\"code\":100,\"msg\":\"\",\"files\":\"\"}";

    Json::Reader reader;
    Json::Value root;
    if(reader.parse(str , root)) {
        std::string upload_id = root["uploadid"].asString();
        int code = root["code"].asInt();
    }
    std::string out = root.toStyledString();
    Json::FastWriter wtriter;
    std::string out2 = wtriter.write(root);

    return 0;
}

int main( int argc , char * argv[] )  {
    int status = ParseJsonFromString();
    return EXIT_SUCCESS;
}

