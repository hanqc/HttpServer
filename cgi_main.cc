#include <iostream>
#include<string>
#include "util.hpp"
#include<sstream>
void HttpResponse(const std::string& body)
{
    std::cout<<"Content-Length:"<<body.size()<<"\n";
    std::cout<<"\n";  //http协议中的空行
    std::cout<<body;
    return ;
}

int main()
{
    //1.先获取到method
    const char* method = getenv("REQUEST_METHOD");
    if(method == NULL)
    {
        HttpResponse("No env REQUEST_METHOD!");
        return 1;
    }
    //2.如果是get请求，从QUERY_STRING读取请求参数
    //4.解析query_string或者body中数据
    StringUtil::UrlParam params;
    if(std::string(method)=="GET")
    {
        const char* query_string = getenv("QUERY_STRING");
        StringUtil::ParseUrlParam(query_string,&params);
    }
    else if(std::string(method)=="POST")
    {
    //3.如果是post请求，从CONTENT_LENGTH读取body的长度
    //  根据body的长度，从标准输入中读取请求的body
    //4.解析query_string或者body中数据
        char buf[1024*10] = {0};
        read(0,buf,sizeof(buf)-1);
        StringUtil::ParseUrlParam(buf,&params);
    }
    //5.根据业务需要进行计算，此处的计算a+b的值
    int a = std::stoi(params["a"]);
    int b = std::stoi(params["b"]);
    int result = a+b;
    //6.根据计算结果，构造响应的数据，写回到标准输出中
    std::stringstream ss;
    ss<<"<h1> result = "<<result<<"</h1>";
    HttpResponse(ss.str());
    return 0;
}
