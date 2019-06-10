/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CBUFFERCONVERTER_H
#define __CBUFFERCONVERTER_H

#ifndef _CRT_STDINT_H
#include <stdint.h>
#ifndef _CRT_STDINT_H
#define _CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

// Forward declarations:

typedef struct _RingItem  RingItem;

/**
 * Application class for the ring buffer stream to buffer filter.
 * Ring items are taken from e.g. ringselector, or ringtostdout
 * on stdin and are buffered into old style nscl daq buffers which
 * are then shipped out stdout for someone else to deal with (e.g.
 * SpecTcl or an event logger.
 */
class CBufferConverter 
{
private:
  uint32_t   m_nBufferSize;
  uint16_t*  m_pPhysicsBuffer;
  uint16_t*  m_pPut;

  uint16_t   m_nRunNumber;

  uint64_t   m_nTriggers;
  uint64_t   m_nTriggersProcessed;
  uint32_t   m_nLastSequence; 
  double     m_fLastFloatingSequence;

public:
  // Canonicals we need.

  CBufferConverter(uint32_t size);
  ~CBufferConverter();

  // Function call operator is the entry point.

  int operator()();

  // Utility functions:
private:
  RingItem* getItem() throw(std::string, int);
  void        outputItem(RingItem* pItem) throw(std::string);

  // I/O stuff.

  void        readOrThrow(void* pData, size_t nBytes) throw(std::string, int);
  void        writeOrThrow(void* pData, size_t nBytes) throw(std::string);
  void        flush() throw(std::string);

  // Buffer oriented stuff.

  void        bufferEvent(RingItem* pItem) throw(std::string);
  void        outputScaler(RingItem* pItem) throw(std::string);
  void        outputStringArray(uint16_t itemType, RingItem* pItem) throw(std::string);
  void        outputStateChange(uint16_t itemType, RingItem* pItem) throw(std::string);
  void        outtputPayload(int16_t itemType, RingItem* pItem) throw(std::string);

  void        fillHeader(void* pDest, uint16_t type);
  void        createBuffer();

  // Byte order sensitive stuff.

  uint32_t    computeSize(RingItem& header);
  uint32_t    getType(RingItem& header);
  uint16_t    mapType(uint32_t ringType);

  bool        isSwapped(RingItem& header);
  uint64_t    swaq(uint64_t aquad);
  uint32_t    swal(uint32_t along);
  uint16_t    swaw(uint16_t aword);

  // Statistics stuff:

  void resetStatistics();
  void computeStatistics(RingItem* pHeader);
  uint32_t getSequence();	
  // Other stuff:
  
  void stockConversionMap();


  
};

#endif
