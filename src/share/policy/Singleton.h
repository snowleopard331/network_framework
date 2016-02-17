/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/10/12
*/

#ifndef _SINGLETON_H_
#define _SINGLETON_H_

#include "policy/ThreadingModel.h"
#include "policy/CreationPolicy.h"
#include "policy/ObjectLifeTime.h"

namespace Jovi
{
    template
    <
        typename T,
        class ThreadingModel = Jovi::SingleThreaded<T>,
        class CreatePolicy = Jovi::OperatorNew<T>,
        class LifeTimePolicy = Jovi::ObjectLifeTime<T>
    >

    class Singleton
    {
    public:
        static T& Instance();

    protected:
        Singleton() {}

    private:
        Singleton(const Singleton&);

        Singleton& operator=(const Singleton&);

        static void DestroySingleton();

        typedef typename ThreadingModel::Lock Guard;
        static T* _instance;
        static bool _destroyed;
    };

    template<typename T, class ThreadingModel, class CreatePolicy, class LifeTimePolicy>
    T* Singleton<T, ThreadingModel, CreatePolicy, LifeTimePolicy>::_instance = NULL;

    template<typename T, class ThreadingModel, class CreatePolicy, class LifeTimePolicy>
    bool Singleton<T, ThreadingModel, CreatePolicy, LifeTimePolicy>::_destroyed = false;

    template<typename T, class ThreadingModel, class CreatePolicy, class LifeTimePolicy>
    T& Jovi::Singleton<T, ThreadingModel, CreatePolicy, LifeTimePolicy>::Instance()
    {
        if(!_instance)
        {
            // double-checked Locking pattern, thread safe
            Guard();

            if(!_instance)
            {
                if(_destroyed)
                {
                    _destroyed = false;
                    LifeTimePolicy::OnDeadReference();
                }

                _instance = CreatePolicy::Create();
                LifeTimePolicy::SchedeleCall(&DestroySingleton);
            }
        }

        return *_instance;
    }

    template<typename T, class ThreadingModel, class CreatePolicy, class LifeTimePolicy>
    void Jovi::Singleton<T, ThreadingModel, CreatePolicy, LifeTimePolicy>::DestroySingleton()
    {
        CreatePolicy::Destroy(_instance);
        _instance = NULL;
        _destroyed = true;
    }
}

#define  INSTANTIATE_SINGLETON_1(TYPE) \
    template class Jovi::Singleton<TYPE, Jovi::SingleThreaded<TYPE>, Jovi::OperatorNew<TYPE>, Jovi::ObjectLifeTime<TYPE> >

#endif//_SINGLETON_H_