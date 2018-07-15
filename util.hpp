//////////////////////////////////////////////////////////////
//此文件放置一些工具类和函数
//为了让这些工具用的方便，直接把声明和实现都放在.hpp中
//《代码大全》(code complete)
//软件工程本质上是对复杂程度的管理
//////////////////////////////////////////////////////////////
#pragma once
#include <iostream>
#include <sys/socket.h>
#include <vector>
#include <sys/time.h>
#include <unordered_map>
#include <fstream>
//boost库
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

//时间戳的类
//把只要是关于时间戳的工作模块都使用这个
class TimeUtil{
public:
    //获取当前的秒级时间戳
    //不能使用无符号类型，因为时间戳是需要相减的，无符号就会出问题
    static int64_t TimeStamp(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return tv.tv_sec;//返回的当前时间戳
    }
    //获取当前的微秒级时间戳
    //不能使用无符号类型，因为时间戳是需要相减的，无符号就会出问题
    static int64_t TimeStampUS(){
        struct timeval tv;
        gettimeofday(&tv,NULL);
        return 1000*1000*tv.tv_sec + tv.tv_usec;//返回的当前时间戳,需要换算
    }
};


//枚举出错误标识
enum LogLevel{
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
};

//char* 当前日志的名字，当前日志的行号
//LOG(INFO) << "haha"  计时和打日志的要求
//加inline关键字可以换成static关键字，也可以实现相同的作用
inline std::ostream& Log(LogLevel level, const char* file, int line){
    //prefix 是日志级别
    std::string prefix = "I";
    if(level == WARNING){
        prefix = "W";
    }else if(level == ERROR){
        prefix = "E";
    }else if(level == CRITICAL){
        prefix = "C";
    }else if(level == DEBUG){
        prefix = "D";
    }
    std::cout<< "[" << prefix << TimeUtil::TimeStamp() << " " << file << ":" << line << "]";
    return std::cout; 
}

//文件名  __FILE__
//文件中的行  __LINE__
//不可以加分号，宏就是字符串替换
//不能使用函数，定义函数写死了文件和行号
//std::ostream& LOG(LogLevel,__FILE__,__LINE__){
//   return Log(level,__FILE__,__LINE__);
//}
#define LOG(level) Log(level, __FILE__, __LINE__)

class FileUtil
{
public:
    //从文件描述符中读取一行
    //一行的界定标识\n \r \r\n
    //返回的line中是不包含界定标识的
    //例如：
    //aaa\nbbb\nccc
    //调用Readline返回的line对象的内容为aaa，不包含\n
   static int ReadLine(int fd,std::string* line)
   {
       line->clear();
       while(true)
       {
           char c = '\0';
           ssize_t read_size = recv(fd,&c,1,0);
           if(read_size<=0)
           {
               return -1;
           }
           //如果当前字符是\r，就把这个情况处理成\n
           if(c=='\r')
           {
               //虽然从缓冲区读了一个字符，但是缓冲区并没有把他删掉
               recv(fd,&c,1,MSG_PEEK);
               if(c=='\n')
               {
                   //发现\r后面一个字符刚好就是\n，为了不影响下次循环，就需要把这样的字符从缓冲区干掉
                   recv(fd,&c,1,0);
               }
               else
               {
                    c = '\n';
               }
           }
           //这个条件涵盖了\r和\r\n的情况
           if(c=='\n')
           {
               break;
           }
           line->push_back(c);
       }
       return 0;
   }

   static int ReadN(int fd,size_t len,std::string* output)
   {
        output->clear();
        char c = '\0';
        for(size_t i=0;i<len;i++)
        {
            recv(fd,&c,1,0);
            output->push_back(c);
        }
        return 0;
   }

   static int ReadAll(int fd,std::string* output)
   {
        while(true)
        {
            char buf[1024] = {0};
            ssize_t read_size = read(fd,buf,sizeof(buf)-1);
            if(read_size<0)
            {
                perror("read");
                return -1;
            }
            if(read_size==0)
            {
                //读完了
                return 0;
            }
            buf[read_size] = '\0';
            (*output) += buf;
        }
        return 0;
   }

   static bool IsDir(const std::string& file_path)
   {
        return boost::filesystem::is_directory(file_path);
   }
   //从文件中读取全部内容到std::string中
   static int ReadAll(const std::string& file_path,std::string* output)
   {
        std::ifstream file(file_path.c_str());
        if(!file.is_open())
        {
            LOG(ERROR)<<"Open file error! file_path="<<file_path<<"\n";
            return -1;
        }
        //seekg调整文件指针的位置，此处是将文件指针调整到文件末尾
        file.seekg(0,file.end);
        //查询当前文件指针的位置，返回值就是文件指针位置相对于文件
        //起始位置的偏移量
        int length = file.tellg();
        //为了从头读取文件，需要把文件指针设置到开头位置
        file.seekg(0,file.beg);
        //读取完整的文件内容
        output->resize(length);
        file.read(const_cast<char*>(output->c_str()),length);
        //万一忘记写close，问题不大，
        //因为ifstream会在析构的时候自动关闭文件描述符
        file.close();
        return 0;
   }
};

class StringUtil
{
public:
    //把一个字符串，按照split_char进行切分，分成的n个子串，放到output数组中
    //token_compress_on的含义是：例如分隔符是空格，字符串是“a","b”
    //对于打开压缩情况，返回的子串就是有两个，“a”，“b”
    //token_compress_off 对于关闭压缩的情况，返回的子串就是有三个，“a”，“”，“b”
    static int Split(const std::string& input,const std::string& split_char,std::vector<std::string>* output)
    {
        boost::split(*output,input,boost::is_any_of(split_char),boost::token_compress_on);
        return 0;
    }
    
    typedef std::unordered_map<std::string,std::string> UrlParam;
    static int ParseUrlParam(const std::string& input,UrlParam* output)
    {
        //1.先按照取地址符号切分成若干个kv
        std::vector<std::string> params;
        Split(input,"&",&params);
        //2.在针对每一个kv，按照 = 切分，放到输出结果中
        for(auto item : params)
        {
            std::vector<std::string> kv;
            Split(item,"=",&kv);
            if(kv.size() != 2)
            {
                //说明参数非法
                LOG(WARNING)<<"kv format error! item="<<item<<"\n";
                continue;
            }
            (*output)[kv[0]] = kv[1];
        }
        return 0;
    }
};
