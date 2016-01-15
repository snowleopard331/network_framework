/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/10/12
*/

#ifndef _OBJECT_LIFETIME_H_
#define _OBJECT_LIFETIME_H_

typedef void (* Destroy)(void);

namespace Jovi
{
    void at_exit(void (*func)());

    template<class T>
    class ObjectLifeTime
    {
        static void SchedeleCall(void (*destroyer)())
        {
            at_exit(destroyer);
        }

        static void OnDeadReference();
    };

    template<class T>
    void ObjectLifeTime<T>::OnDeadReference()   // We don't handle Dead Reference for now
    {

    }
}

#endif//_OBJECT_LIFETIME_H_