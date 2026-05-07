cmake_minimum_required(VERSION 3.16)
project(AsyncLogger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

# 源文件（目前还没有，先注释掉，等你创建了再取消注释）
# add_library(logger_lib STATIC
#     src/Sink.cpp
#     src/AsyncLogger.cpp
#     src/Logger.cpp
# )

add_executable(example examples/example.cpp)
target_compile_definitions(example PRIVATE DEBUG_ENV)

