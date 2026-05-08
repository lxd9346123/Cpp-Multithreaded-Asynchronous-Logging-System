cmake_minimum_required(VERSION 3.16)
project(AsyncLogger)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(example examples/example.cpp)

# 如果需要使用DEBUG级别输出
target_compile_definitions(example PRIVATE DEBUG_ENV)

