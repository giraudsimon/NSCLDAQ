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
#ifndef __TESTSOURCE_H
#define __TESTSOURCE_H




/**
 * @file TestSource.h
 * @brief Definition for the TestSource class which produces data into the ringk.
 */
#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_UNISTD_H
#include <unistd.h>
#ifndef __CRT_UNISTD_H
#define __CRT_UNISTD_H
#endif
#endif

class CRingBuffer;

/**
 * @class TestSource
 *
 *  The TestSource class, when instantiated provides a wide variety of test data and
 *  edge conditions for the ring data source.
 *
 */
class TestSource 
{
  std::string m_ringName;
  int         m_size;
  int         m_elapsedTime;
  int         m_tsIncrement;
  uint64_t    m_timestamp;
  useconds_t  m_delay;
  bool        m_useBodyHeaders;

public:
  TestSource(std::string ringName, int eventSize, bool useBodyHeaders=false) : 
    m_ringName(ringName), m_size(eventSize), m_elapsedTime(0), 
    m_tsIncrement(2), m_timestamp(0), m_delay(0), 
    m_useBodyHeaders(useBodyHeaders)
  {}

public:
  void setTimestampIncrement(int newIncr) {m_tsIncrement = newIncr;}
  void setDelay(uint64_t newDelay) {m_delay = newDelay; }

  void setGenerateBodyHeaders (bool onoff);

public:
  void operator()();
    
  // private utilities:

  void dataFormat(CRingBuffer& ring);
  void beginRun(CRingBuffer& ring, int run, std::string title);
  void someEventData(CRingBuffer& rings, int events);
  void endRun(CRingBuffer& ring, int run, std::string title);
  void Scaler(CRingBuffer& ring, int nsclers, int nsec);
  
};

#endif

