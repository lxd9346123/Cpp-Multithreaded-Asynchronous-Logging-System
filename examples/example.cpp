#include "Logger.h"

int main(int argc, char const *argv[])
{
    mLog::GetInstance().print("haha%s","123");
    return 0;
}
