
#include <CAbnormalEndRunFilterHandler.h>
#include <CRingItem.h>
#include <CDataSink.h>
#include <DataFormat.h>


CAbnormalEndRunFilterHandler::CAbnormalEndRunFilterHandler(const CAbnormalEndRunFilterHandler& rhs) 
  : m_sink(rhs.m_sink)
{}

// All of the functionality of the other are in the handleRingItem.
CRingItem* 
CAbnormalEndRunFilterHandler::handleRingItem(CRingItem* item) {

  if (item->type() == ABNORMAL_ENDRUN) {
    m_sink.put(item->getItemPointer(), item->size());
    throw CException("Found an abnormal end run item. Shutting down!");
  }

  return item;
}

CRingItem* 
CAbnormalEndRunFilterHandler::handleStateChangeItem(CRingStateChangeItem* item)
{
  return handleRingItem(item);
}

CRingItem* 
CAbnormalEndRunFilterHandler::handleScalerItem(CRingStateChangeItem* item)
{
  return handleRingItem(item);
}

CRingItem* 
CAbnormalEndRunFilterHandler::handleTextItem(CRingTextItem* item)
{
  return handleRingItem(item);
}

CRingItem* 
CAbnormalEndRunFilterHandler::handlePhysicsEventItem(CPhysicsEventItem* item)
{
  return handleRingItem(item);
}

CRingItem* 
CAbnormalEndRunFilterHandler::handlePhysicsEventCountItem(CRingPhysicsEventCountItem* item)
{
  return handleRingItem(item);
}

CRingItem* 
CAbnormalEndRunFilterHandler::handleFragmentItem(CRingFragmentItem* item)
{
  return handleRingItem(item);
}

