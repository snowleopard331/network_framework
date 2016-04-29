/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/08/24
*/

#include "Threading.h"

using namespace Boost_Based;

Thread::Thread()
    : m_pThread(NULL)
    , m_task(NULL)
{

}

Thread::Thread(Runnable* instance)
    : m_pThread(NULL)
    , m_task(instance)
{
    if(m_task)
    {
        m_task->incReference();
    }

    bool isThreadStart = start();
    Evil_ASSERT(isThreadStart);
}

Thread::~Thread()
{
    // deleted runnable object (if no other references)
    if(m_task)
    {
        m_task->decReference();
    }
}

bool Thread::start()
{
    if(m_task == NULL || m_pThread)
    {
        return false;
    }

    if(!m_pThread)
    {
        m_pThread = new boost::thread(boost::bind(Thread::ThreadTask, m_task)); 
    }

    if(m_pThread)
    {
        m_task->incReference();
    }

    return (m_pThread != NULL);
}

void* Thread::ThreadTask(void* param)
{
    Runnable* _task = static_cast<Runnable*>(param);

    _task->run();

    // task execution complete, free referecne added at
    _task->decReference();

    return NULL;
}

bool Thread::wait()
{
    if(!m_task || !m_pThread)
    {
        return false;
    }

    m_pThread->join();

    return true;
}

// unfinished
void Thread::destroy()
{
    if(!m_task || !m_pThread)
    {
        return;
    }

    // kill thread

    m_task->decReference();
}

void Thread::Sleep(unsigned long secs)
{
    boost::this_thread::sleep(boost::posix_time::seconds(secs));
}

//Thread* Thread::current()
//{
//    Thread* _thread = m_ThreadStorage.get();
//    if(!_thread)
//    {
//        _thread = new Thread();
//        _thread->m_pThread = Thread::currentThread();
//
//        // aotu cleanup oldValue if oldValue exist
//        m_ThreadStorage.reset(_thread);
//    }
//
//    return _thread;
//}
//
//boost::thread* Thread::currentThread()
//{
//    // TODO
//}