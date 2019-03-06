/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
/**
 * @file TestSource.cpp
 * @brief Implement the TestSource class.
 */
#include "TestSource.h"
#include <CRingBuffer.h>
#include <CRingItem.h>
#include <CDataFormatItem.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <os.h>

/**
 * operator()
 *
 *  Entry point to the test.
 *  - Connect to the ring.
 *  - Insert a begin run, some data, some scalers and an end run.
 *  - Disconnect from the ring.
 */
void
TestSource::operator()() 
{
  CRingBuffer ring(m_ringName, CRingBuffer::producer); // Connect to the ring.
  dataFormat(ring);
  beginRun(ring, 1234, "This is the begin run");
  for (int i = 0; i < 1000; i++) {
    someEventData(ring, 500);
    Scaler(ring, 32, 5);
  }
  endRun(ring,  1234, "This is the end run");
}

/*------------------------------------------------------------------------------
** Private utilities.
*/

void
TestSource::dataFormat(CRingBuffer& ring)
{
  CDataFormatItem format;
  format.commitToRing(ring);
}

void
TestSource::beginRun(CRingBuffer& ring, int run, std::string title)
{
  CRingStateChangeItem begin(BEGIN_RUN);
  begin.setTitle(title);
  begin.setRunNumber(run);
  m_elapsedTime = 0;

  if ( m_useBodyHeaders ) {
    begin.setBodyHeader(m_timestamp,0,BEGIN_RUN);
  }

  begin.commitToRing(ring);
}

void 
TestSource::endRun(CRingBuffer& ring, int run, std::string title)
{
  CRingStateChangeItem end(END_RUN);
  end.setTitle(title);
  end.setRunNumber(run);
  end.setElapsedTime(m_elapsedTime);

  if ( m_useBodyHeaders ) {
    end.setBodyHeader(m_timestamp,0,END_RUN);
  }
  end.commitToRing(ring);
}

void 
TestSource::Scaler(CRingBuffer& ring, int nscalers, int nsec)
{
  CRingScalerItem item(nscalers);
  item.setStartTime(m_elapsedTime);
  m_elapsedTime += nsec;
  item.setEndTime(m_elapsedTime);

  for (int i = 0; i < nscalers; i++) {
    item.setScaler(i, i*10);
  }

  if ( m_useBodyHeaders ) {
    item.setBodyHeader(m_timestamp,0,0);
  }
  item.commitToRing(ring);

}

void
TestSource::someEventData(CRingBuffer& ring, int events)
{
  for (int i = 0; i < events; i++) {
    uint64_t timestamp = m_timestamp;
    m_timestamp += m_tsIncrement;

    CRingItem event(PHYSICS_EVENT);
    
    // Put the timestamp first:

    uint64_t* pTimestamp = reinterpret_cast<uint64_t*>(event.getBodyCursor());
    *pTimestamp++ = timestamp;
    uint16_t* p   = reinterpret_cast<uint16_t*>(pTimestamp);

    for (int i = 0; i < 30; i++) {
      *p++ = i;
    }
    event.setBodyCursor(p);

    if ( m_useBodyHeaders ) {
      event.setBodyHeader(m_timestamp,0,0);
    }

    event.commitToRing(ring);
    if (m_delay) {
      Os::usleep(m_delay);
    }

  }
}
