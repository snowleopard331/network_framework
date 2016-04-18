/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/10/12
*/

#ifndef _THREADING_MODEL_H_
#define _THREADING_MODEL_H_

namespace Evil
{
    template<class T>
    class SingleThreaded
    {
    public:
        // empty object
        struct Lock
        {
            Lock()  
            {

            }

            Lock(const T&)  
            {

            }

            // for single threaded we ignore this
            Lock(const SingleThreaded<T>&)  
            {

            }
        };
    };

    
    template<class T, class MUTEX>
    class ClassLevelLockable
    {
    private:
        static MUTEX _mutex;

    public:
        ClassLevelLockable()
        {

        }

        friend class Lock;

        class Lock
        {
        public:
            Lock(const T&)
            {
                ClassLevelLockable<T, MUTEX>::_mutex.lock();
            }

            Lock(const ClassLevelLockable<T, MUTEX>&)
            {
                ClassLevelLockable<T, MUTEX>::_mutex.lock();
            }

            Lock()
            {
                ClassLevelLockable<T, MUTEX>::_mutex.lock();
            }

            ~Lock()
            {
                ClassLevelLockable<T, MUTEX>::_mutex.unlock();
            }
        };
    };
}

#endif//_THREADING_MODEL_H_