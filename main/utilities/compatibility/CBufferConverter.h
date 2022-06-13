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

#ifndef CBUFFERCONVERTER_H
#define CBUFFERCONVERTER_H

#include <stdint.h>
#include <string>

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
  RingItem* getItem() ;
  void        outputItem(RingItem* pItem) ;

  // I/O stuff.

  void        readOrThrow(void* pData, size_t nBytes) ;
  void        writeOrThrow(void* pData, size_t nBytes) ;
  void        flush() ;

  // Buffer oriented stuff.

  void        bufferEvent(RingItem* pItem) ;
  void        outputScaler(RingItem* pItem) ;
  void        outputStringArray(uint16_t itemType, RingItem* pItem) ;
  void        outputStateChange(uint16_t itemType, RingItem* pItem) ;
  void        outtputPayload(int16_t itemType, RingItem* pItem) ;

  void        fillHeader(void* pDest, uint16_t type);
  void        createBuffer();

  // Byte order sensitive stuff.

  uint32_t    computeSize(RingItem& header);
  uint32_t    getType(RingItem& header);
  uint16_t    mapType(uint32_t ringType);

  bool        isSwapped(RingItem& header);
 

  // Statistics stuff:

  void resetStatistics();
  void computeStatistics(RingItem* pHeader);
  uint32_t getSequence();	
  // Other stuff:
  
  void stockConversionMap();


  
};

#endif
