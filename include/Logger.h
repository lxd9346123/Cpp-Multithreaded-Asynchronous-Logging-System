#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>

class mLog{
public:
    static mLog& GetInstance(){
        static mLog log;
        return log;
    }

    template<class... Args>
    void print(const char* format,Args... args){
        std::string msg = formatString(format,args...);
        std::cout << getCurrentTimeString()
                  << " "
                  << msg
                  << std::endl;
    }
private:
    mLog(){};                               // 构造函数
    mLog(const mLog&) = delete;             // 禁用拷贝
    mLog& operator=(const mLog&) = delete;  // 禁用传递

    template<class... Args>
    std::string formatString(const char* format,Args... args){
        int size = std::snprintf(nullptr,0,format,args...);
        if (size < 0)
            return "format error";
        std::vector<char> buffer(size + 1);
        std::snprintf(buffer.data(),buffer.size(),format,args...);
        return std::string(buffer.data());
    }

    std::string getCurrentTimeString(){
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);

        std::tm tm_time;
    #ifdef _WIN32
        localtime_s(&tm_time, &time);
    #else
        localtime_r(&time, &tm_time);
    #endif
        std::ostringstream oss;
        oss << std::put_time(&tm_time, "[%Y-%m-%d %H:%M:%S]");
        return oss.str();
    }
};

#endif