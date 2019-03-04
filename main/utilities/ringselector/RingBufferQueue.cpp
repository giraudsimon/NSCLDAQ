/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


/**
 * @file RingBufferQueues.cpp
 * @brief Handle interthread communications between the main and output
 *        thread.
 * @author Ron Fox <fox@nscl.msu.edu>
 */
#include "RingBufferQueue.h"

static  CRingItem* EMPTY(reinterpret_cast<CRingItem*>(0xffffffff));



/**
 * Constructor
 *  @param nFree - number of queue elements to initially put in the free queue.
 */
Queues::Queues(int nFree) 
{
  CRingItem* anItem(EMPTY);
  for (int i =0; i < nFree; i++) {
    m_Free.queue(anItem);	// Can't put in null pointers because of how getFree works.
  }
}
/**
 * Destructor
 *   All items in the transit queue are removed and deleted.  Other than that
 *   the queues should be able to take care of themselves.
 */
Queues::~Queues()
{
  CRingItem* pItem;
  while(m_InTransit.getnow(pItem)) {
    delete pItem;
  }
}


/**
 * getFree
 *   Get an element from the free queue.
 *  @return CRingItem*  
 *  @retval Null if there are no free items remaining.
 *  @retval not null - the free item.
 *
 * @note this never blocks.
 */
CRingItem*
Queues::getFree() 
{
  CRingItem* pItem;

  if (m_Free.getnow(pItem)) {
    return pItem;
  } else {
    return 0;
  }
}
/**
 * getFreeW
 *   Get an element from the free queue - blocking until there's one.
 *
 * @return CRingItem* pointer to the item.
 */
CRingItem*
Queues::getFreeW()
{
  return m_Free.get();
}
/**
 * send
 *    Send a ring item to the output thread by putting it in the
 *    m_InTransit queue.
 *
 * @param pItem - pointer to the item to transfer.
 */
void
Queues::send(CRingItem* pItem)
{
  m_InTransit.queue(pItem);
}
/**
 * receive
 *   Receives an item from the in transit queue....blocks until
 *   one is available?
 *
 * @return CRingItem* - pointer to the item received.
 */
CRingItem*
Queues::receive()
{
  return m_InTransit.get();
}

/**
 * Free
 *   Frees a ring item.  The assumption is that since the ring item
 *   came from a ring it is dynamic and must be deleted.  An entry is
 *   also added to the free queue.
 *
 * @param pItem - Pointer to the ring item to free.
 */
void
Queues::Free(CRingItem* pItem)
{
  delete pItem;
  m_Free.queue(EMPTY);	// must be non-null see getFree.

}

void* gpTCLApplication(0);
