/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/18
*/

#ifndef _EVENT_MGR_H_
#define _EVENT_MGR_H_

#include "Common.h"
#include "policy/Singleton.h"

///---------------------------- define event type -----------------------------------

enum EVENT_ID
{
    EVENT_ID_TEST       = 1,

    EVENT_ID_MAX,
};


///----------------------------- event data define ----------------------------------

// EVENT_ID_TEST
struct EventDataTest
{
    int test1;
    std::string test2;

    EventDataTest() : test1(0) {}
};


///----------------------------- event ----------------------------------

class EventHandle;

class Event
{
public:
    explicit Event(EventHandle* pEvnHandler, uint count)
        : m_eventId(0)
        , m_count(count)
        , m_pEventHandler(pEvnHandler)
    {
        // ?? pEvnHandler == 0, throw exception
    }

    virtual ~Event()
    {

    }

public:
    EventHandle* getEventHandle() const
    {
        return m_pEventHandler;
    }

    uint getCount() const
    {
        return m_count;
    }

    void reduceCount()
    {
        --m_count;
    }

private:
    uint            m_eventId;
    uint            m_count;            /**< 0 - no limit times, no zero is the number was triggered*/
    EventHandle*    m_pEventHandler;
};


// base class for the event definition of object
class EventHandle
{
public:
    EventHandle()
    {

    }

    virtual ~EventHandle()
    {

    }

public:
    virtual void OnEvent(uint eventId, void* pData) = 0;
};


// all global event, to be continue...
class EventMgr
{
public:
    friend class Evil::OperatorNew<EventMgr>;

private:
    EventMgr();
    ~EventMgr();

public:
    // the same eventId can be registed only once
    void registerEvent(uint eventId, std::unique_ptr<Event>& pEvent);

    // before the object was destroyed, if any event has registed, this func must be called
    void unregisterEvent(uint eventId, EventHandle* pEvnHandler);

    void fireEvent(uint eventId, void* pData);

private:
    bool findSame(uint eventId, EventHandle* pEvnHandler) const;

private:
    typedef std::multimap< uint, std::unique_ptr<Event> >     EventList;

    EventList   m_eventList;
};

#define sEventMgr Evil::Singleton<EventMgr>::Instance()

#endif//_EVENT_MGR_H_