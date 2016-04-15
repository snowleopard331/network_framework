/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/07
*/

#ifndef _LOCKED_QUEUE_H_
#define _LOCKED_QUEUE_H_

#include <deque>
#include <boost/thread/lock_types.hpp>


namespace Boost_Based
{
    template< class T, class LockType, typename StorageType = std::deque<T> >
    class LockedQueue
    {
        LockType        _lock;          /**< Lock access to the queue. */
        StorageType     _queue;         /**< Storage backing the queue. */
        bool            _canceled;      /**< Cancellation flag. */

    public:        // Create a LockedQueue.
        LockedQueue()
            : _canceled(false) {}

        // Destroy a LockedQueue
        virtual ~LockedQueue() {}

        //Adds an item to the queue.
        void add(const T& item)
        {
            //boost::mutex::scoped_lock guard(this->_lock);
            //boost::recursive_mutex::scoped_lock guard(this->_lock);
            boost::unique_lock<LockType> guard(this->_lock);
            _queue.push_back(item);
        }

        // Gets the next result in the queue, if any.
        bool next(T& result)
        {
            boost::unique_lock<LockType> guard(this->_lock);
            if(!guard.owns_lock())
            {
                return false;
            }

            if(_queue.empty())
            {
                return false;
            }

            result = _queue.front();
            _queue.pop_front();

            return true;
        }

        template<class Checker>
        bool next(T& result, Checker& check)
        {
            boost::unique_lock<LockType> guard(this->_lock);
            if(!guard.owns_lock())
            {
                return false;
            }

            if(_queue.empty())
            {
                return false;
            }

            result = _queue.front();
            if(!check.Process(result))
            {
                return false;
            }

            _queue.pop_front();
            return true;
        }

        // Peeks at the top of the queue. Remember to unlock after use
        T& peek()
        {
            lock();

            T& result = _queue.front();

            return result;
        }

        // Cancels the queue
        void cancel()
        {
            boost::unique_lock<LockType> guard(this->_lock);
            _canceled = true;
        }

        // Checks if the queue is cancelled
        bool cancelled()
        {
            boost::unique_lock<LockType> guard(this->_lock);
            return _canceled;
        }
        // Locks the queue for access
        void lock()
        {
            //boost::mutex mutex;
            //boost::mutex::scoped_lock g(mutex);
            //g.lock();
            //g.unlock();

            this->_lock.lock();
        }

        // Unlocks the queue
        void unlock()
        {
            this->_lock.unlock();
        }

        // Checks if we're empty or not with locks held
        bool empty()
        {
            boost::unique_lock<LockType> guard(this->_lock);
            return _queue.empty();
        }
    };
}

#endif//_LOCKED_QUEUE_H_