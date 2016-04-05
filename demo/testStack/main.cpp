#include <iostream>
#include "StackTrace.h"

int main()
{
    StackTrace stack;
    std::cout<<stack.c_str()<<std::endl;

    return 0;
}