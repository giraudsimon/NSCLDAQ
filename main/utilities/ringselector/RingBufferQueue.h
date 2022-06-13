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
 * @file RingBufferQueue.h
 * @brief Provide definitions of ring buffer queues.
 * @author Ron Fox <fox@nscl.msu.edu>
 */


#ifndef RINGBUFFERQUEUE_H
#define RINGBUFFERQUEUE_H

#include <CBufferQueue.h>
#include <CRingItem.h>

/*
 *  A RingBuffer queue is a CBufferQueue whose elements are pointers
 *  to CRingItems.
 */

typedef CBufferQueue<CRingItem*> RingBufferQueue, *pRingBufferQueue;

/**
 *  @class Queues
 *    This class containst the queues and a thin layer around them to
 *    facilitate inter process communication.  
 *    In reality with a queue that just contains a CRingItem* we don' need
 *    to have a free/in transit queue but we do this to limit the number of
 *    ring items that are allowed to be in-transit so that we know when
 *    to drop items on the floor.
 */
class Queues {
private:
  RingBufferQueue     m_Free;	/* Free queue elemnts */
  RingBufferQueue     m_InTransit;	/* queue element in transit to output thread. */

public:
  // Canonicals:

  Queues(int nFree);
  ~Queues();

  // Sender functions:

  CRingItem* getFree();
  CRingItem* getFreeW();
  void send(CRingItem* pItem);

  // Output thread functions:

  CRingItem* receive();
  void Free(CRingItem* pItem);
};

#endif
