/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/18
*/

#include "eventMgr.h"

EventMgr::EventMgr()
{

}

EventMgr::~EventMgr()
{
    m_eventList.clear();
}

void EventMgr::registerEvent(uint eventId, std::unique_ptr<Event>& pEvent)
{
    IF_NOT_RETURN(eventId < EVENT_ID_MAX);
    IF_NOT_RETURN(pEvent.get());

    if(!findSame(eventId, pEvent->getEventHandle()))
    {
        m_eventList.insert(std::pair<uint, std::unique_ptr<Event> >(eventId, std::move(pEvent)));
    }
}

void EventMgr::unregisterEvent(uint eventId, EventHandle* pEvnHandler)
{
    IF_NOT_RETURN(eventId < EVENT_ID_MAX);
    IF_NOT_RETURN(pEvnHandler);

    std::pair<EventList::iterator, EventList::iterator>  itPair;
    itPair = m_eventList.equal_range(eventId);

    for(EventList::iterator iter = itPair.first; iter != itPair.second;)
    {
        EventList::iterator itTemp = iter++;

        if(itTemp->second->getEventHandle() == pEvnHandler)
        {
            m_eventList.erase(itTemp);
            break;
        }
    }
}

bool EventMgr::findSame(uint eventId, EventHandle* pEvnHandler) const
{
    IF_NOT_RETURN_FALSE(pEvnHandler);

    std::pair<EventList::const_iterator, EventList::const_iterator>  itPair;
    itPair = m_eventList.equal_range(eventId);

    for(EventList::const_iterator iter = itPair.first; iter != itPair.second; ++iter)
    {
        if(iter->second->getEventHandle() == pEvnHandler)
        {
            return true;
        }
    }
    
    return false;
}

void EventMgr::fireEvent(uint eventId, void* pData)
{
    IF_NOT_RETURN(eventId < EVENT_ID_MAX);

    std::pair<EventList::iterator, EventList::iterator>  itPair;
    itPair = m_eventList.equal_range(eventId);

    for(EventList::iterator iter = itPair.first; iter != itPair.second;)
    {
        EventList::iterator itTemp = iter++;

        itTemp->second->getEventHandle()->OnEvent(eventId, pData);

        if(itTemp->second->getCount())
        {
            itTemp->second->reduceCount();
            if(0 == itTemp->second->getCount())
            {
                m_eventList.erase(itTemp);
                continue;
            }
        }
    }
}