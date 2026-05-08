#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>

#include "LogLevel.h"

#define mLOG_INFO(format, ...) mTool::mLog::GetInstance().print(mTool::LogLevel::INFO,format,##__VA_ARGS__);
#ifdef DEBUG_ENV
#define mLOG_DEBUG(format, ...) mTool::mLog::GetInstance().print(mTool::LogLevel::DEBUG,format,##__VA_ARGS__);
#else
#define mLOG_DEBUG(format, ...) ((void)0);
#endif
#define mLOG_WARN(format, ...) mTool::mLog::GetInstance().print(mTool::LogLevel::WARN,format,##__VA_ARGS__);
#define mLOG_ERROR(format, ...) mTool::mLog::GetInstance().print(mTool::LogLevel::ERROR,format,##__VA_ARGS__);

namespace mTool{

class mLog{
public:
    static mLog& GetInstance(){
        static mLog log;
        return log;
    }

    template<class... Args>
    void print(LogLevel level, const char* format,Args... args){
        std::string msg = formatString(format,args...);
        msg = getCurrentTimeString() + to_color(level) + to_string(level)
                + msg + "\033[0m";
        {
        std::lock_guard<std::mutex> lock(mtx_);
        msg_.push(msg);
        }
        cv_.notify_one();
    }
private:
    mLog(): running_(true), handle_msg_(&mLog::handle_msg,this){};                               // 构造函数
    mLog(const mLog&) = delete;             // 禁用拷贝
    mLog& operator=(const mLog&) = delete;  // 禁用传递
    ~mLog(){
        running_ = false;
        cv_.notify_one();
        if (handle_msg_.joinable())
            handle_msg_.join();
    }

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

    void handle_msg(){
        while (running_){
            std::unique_lock<std::mutex> lock(mtx_);
            cv_.wait(lock,[this](){return !msg_.empty() || !running_;});
            if (msg_.empty())
                break;
            std::cout << msg_.front() << std::endl;
            msg_.pop();
        }
        // 退出前清空剩余log
        while (!msg_.empty()){
            std::cout << msg_.front() << std::endl;
            msg_.pop();
        }
    }

    std::mutex mtx_;
    std::queue<std::string> msg_;
    std::condition_variable cv_;
    std::atomic<bool> running_;
    std::thread handle_msg_;
};

}
#endif