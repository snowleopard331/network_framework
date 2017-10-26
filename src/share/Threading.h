/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/08/24
*/

#ifndef _THREADING_H_
#define _THREADING_H_

#include <boost/atomic/atomic.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/tss.hpp>

#include "Common.h"

namespace Boost_Based
{
    class Runnable
    {
    private:
        boost::atomic_int   m_refs;

    public:
        Runnable()
            : m_refs(0)
        {

        }

        virtual ~Runnable() 
        {

        }

        virtual void run() = 0;

        void incReference()
        {
            ++m_refs;
        }

        void decReference()
        {
            if(!--m_refs)
            {
                delete this;
            }
        }
    };


    class Thread
    {
    private:

        Thread(const Thread&);

        Thread& operator=(const Thread&);

        boost::thread* m_pThread;

        Runnable* m_task;

        typedef boost::thread_specific_ptr<Thread> ThreadStorage;
        static ThreadStorage m_ThreadStorage;

    public:

        Thread();

        explicit Thread(Runnable* instance);

        ~Thread();

    public:
        bool start();

        bool wait();

        // unfinished
        void destroy();

    public:
        static void Sleep(unsigned long msecs);


    private:
        static void* ThreadTask(void* param);
    };
}

#endif//_THREADING_H_