#pragma once
#include <string>
#include <unordered_map>

namespace http_server{

typedef std::unordered_map<std::string,std::string> Header;

struct Request{
    std::string method;
    std::string url;
    //例如url为一个形如http://www.baidu.com/index.html?kwd="cpp"
    std::string url_path;//index.html
    std::string query_string;//kwd="cpp"
    //std::string version://暂时不考虑版本号
    Header header;//一组字符串键值对
    std::string body;//http的请求body
};

struct Response{
    int code;//状态码
    std::string desc;//状态码描述

    //下面这两个变量专门给处理静态页面时使用的
    //当前请求如果是请求静态页面，这两个字段会被填充
    //并且cgi_resp字段为空
    Header header;//响应报文中的header数据
    std::string body;//响应报文中的body数据

    //下面这个变量专门给CGI来使用，并且如果当前请求时CGI的话
    //cgi_resp就会被CGI程序进行填充
    //并且，header和body这两个字段为空
    std::string cgi_resp;//CGI程序返回给父进程的内容，包含了部分header和body，引入这个变量，是为了避免
                         //解析CGI程序返回的内容，因为这部分内容可以直接写到socket中
};
//当前请求的上下文，包含了这次请求的所有需要的中间数据
//方便进行扩展，整个处理请求的过程中，每个环节都能够拿到
//所有和这次请求相关的数据
class HttpServer;
struct Context{
    Request req;
    Response resp;
    int new_sock;
    HttpServer* server;
};

//实现核心流程的类
class HttpServer{
public:
    //以下的几个函数，返回0表示成功
    //返回小于0的值表示执行失败
    int Start(const std::string& ip,short port);
private:
    //根据http请求字符串，进行反序列化
    //从socket中读取一个字符串，输出Request对象
    int ReadOneRequest(Context* context);
    //根据Response对象，拼接成一个字符串，写回到客户端
    int WriteOneResponse(Context* context);
    //根据Request对象，构造Response对象
    int HandlerRequest(Context* context);
    int Process404(Context* context);
    int ProcessStaticFile(Context* context);
    int ProcessCGI(Context* context);
private:
    static void* ThreadEntry(void* arg);
    int ParseFirstLine(const std::string& first_line,std::string* method,std::string* url);
    int ParseUrl(const std::string& url,std::string* url_path,std::string* query_string);
    int ParseHeader(const std::string& header_line,Header* header);
    void GetFilePath(const std::string& url_path,std::string* file_path);
    //以下为测试函数
    void PrintRequest(const Request& req);
};
}//end http_server 
