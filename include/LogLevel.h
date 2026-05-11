#ifndef LOGLEVEL_H
#define LOGLEVEL_H

#include <string>

namespace mTool{

enum class LogLevel{
    INFO = 0,
    DEBUG,
    WARN,
    ERROR
};

enum class TimeLogLevel{
    FILE_NAME,
    LOG_NAME
};

inline std::string to_string(const LogLevel& level){
    switch (level){
        case LogLevel::INFO : return " [INFO] ";
        case LogLevel::DEBUG : return " [DEBUG] ";
        case LogLevel::WARN : return " [WARN] ";
        case LogLevel::ERROR : return " [ERROR] ";
        default : return "[unkown]";
    }
}

inline std::string to_color(const LogLevel& level){
    switch (level){
        case LogLevel::INFO : return "\033[32m";
        case LogLevel::DEBUG : return "\033[36m";
        case LogLevel::WARN : return "\033[33m";
        case LogLevel::ERROR : return "\033[31m";
        default : return "[unknown]";
    }
}

}

#endif