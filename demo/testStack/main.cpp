#include <iostream>
#include "StackTrace.h"

void fun1()
{
    StackTrace stack;
    std::cout<<stack.c_str()<<std::endl;
}

void fun2()
{
    fun1();
}

int main()
{
    fun2();

    return 0;
}