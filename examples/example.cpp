#include "Logger.h"

void log_thread1(){
    int i = 0;
    for (int j = 0; j < 10; j ++){
        i ++;
        mLOG_INFO("thread1 is logging : %d",i);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void log_thread2(){
    int i = 0;
    for (int j = 0; j < 10; j ++){
        i ++;
        mLOG_WARN("thread2 is logging : %d",i);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

int main(int argc, char const *argv[])
{
    std::thread t1(log_thread1);
    std::thread t2(log_thread2);
    if (t1.joinable())
        t1.join();
    if (t2.joinable())
        t2.join();
    return 0;
}
