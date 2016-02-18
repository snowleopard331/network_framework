/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/10/12
*/

#include "ObjectLifeTime.h"
#include <cstdlib>

namespace Jovi
{
    extern "C" void external_wrapper(void* p)
    {
        std::atexit((void (*)())p);
    }

    void at_exit(void (*func)())
    {
        external_wrapper((void*)func);
    }
}