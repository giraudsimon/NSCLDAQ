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
#include "CBufferConverter.h"
#include "buffer.h"
#include "buftypes.h"

#include <DataFormat.h>
#include <string>
#include <iostream>
#include <map>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <utils.h>

static std::map<uint32_t, uint16_t> typeConversionMap;

/*-------------------------------------------------------------------------------------*/

// Construction, destruction and any canonicals implemented.

/**
 * Construction. We need to save the buffersize so we can make the right 
 * sized output artifacts:
 *
 * @param size - Number of bytes in the output buffer.
 */
CBufferConverter::CBufferConverter(uint32_t size) :
  m_nBufferSize(size),
  m_pPhysicsBuffer(0),
  m_pPut(0),
  m_nRunNumber(0),
  m_nTriggers(0),
  m_nTriggersProcessed(0),
  m_nLastSequence(0),
  m_fLastFloatingSequence(0.0)
  
{}

CBufferConverter::~CBufferConverter()
{
  delete m_pPhysicsBuffer;
}

/*----------------------------------------------------------------------------------*/

// Public methods

/**
 * Function call operator.  This is the function that actually does the real work.
 *
 */
int
CBufferConverter::operator()()
{

  RingItem*   pHeader;	// Really this will point to the entire item.

  try {

    while(pHeader = getItem()) {
      outputItem(pHeader);
      free(pHeader);		// It's a dynamically allocated thing.
    }
    
  }
  catch(int i) {              // exit code.
    
    return i;
  }
  catch(std::string msg) {
    std::cerr << msg << std::endl;
    return 1;
  }
  return 0;			// Success.
}
  
/*------------------------------------------------------------------------------*/

// Private utilities.

/**
 * Read an item from stdin and return a dynamically allcoated pointer to it.
 * @return RingItem*
 * @retval pointer to dynamic storage that contains the full item
 *         (not just the pointer).
 * @throw std::string - main thing that can happen to us is EOF or 
 *                      other read failures.
 */
RingItem* 
CBufferConverter::getItem() 
{

  // Read the header and figure out the size:

  RingItemHeader header;		
  readOrThrow(&header, sizeof(header));
  uint32_t size = computeSize(reinterpret_cast<RingItem&>(header));

  // Allocate the buffer for the entire item, move the header
  // in and figure out:
  // - Pointer to the item payload.
  // - remaining read size:

  uint8_t* buffer = reinterpret_cast<uint8_t*>(malloc(size));
  memcpy(buffer, &header, sizeof(header));
  uint8_t* pPayload = buffer + sizeof(RingItemHeader);
  uint32_t payloadSize = size - sizeof(RingItemHeader);

  // Read the payload and return the result.
  // Note that we're going to retain the original byte ordering.

  readOrThrow(pPayload, payloadSize);

  return reinterpret_cast<RingItem*>(buffer);


}
/**
 * Do what's needed to output an item to stdout.  In the case of PHYSICS_EVENT items,
 * We buffer a bunch of them until the data buffer would overflow (m_pPhysics buffer 
 * will be allocated 2x as big as needed).  For all other types, the
 * physics buffer will get flushed and a new buffer emitted.
 * Note that for Physics trigger types, we just keep statistics and use them
 * to figure out what our sequence numbers should look like so that we
 * get the right sampling efficiency if the data source is sampling events.
 * @param pItem - pointer to the entire ring item. Note that the type is given as
 *                RingItem* so that we're able to get the chunks we need.
 *
 */
void
CBufferConverter::outputItem(RingItem* pItem) 
{
  uint16_t type = mapType(getType(*pItem));
  switch(type) {
  case DATABF:
    bufferEvent(pItem);
    break;
  case SCALERBF:
    flush();
    outputScaler(pItem);
    break;
  case STATEVARBF:
    flush();
    outputStringArray(type, pItem);
    break;
  case RUNVARBF:
    flush();
    outputStringArray(type, pItem);
    break;
  case PKTDOCBF:
    flush();
    outputStringArray(type, pItem);
    break;
  case BEGRUNBF:
    flush();
    resetStatistics();
    outputStateChange(type, pItem);
    break;
  case ENDRUNBF:
    flush();
    outputStateChange(type, pItem);
    break;
  case PAUSEBF:
    flush();
    outputStateChange(type, pItem);
    break;
  case RESUMEBF:
    flush();
    outputStateChange(type, pItem);
    break;
  case TRIGGERSTATS:
    computeStatistics(pItem);
    break;
  default:
    std::cerr << "Unrecognized item type ignored : " << type << std::endl;
    break;
  }
}

 
 

/**
 * Read some data from stdin. 
 * @param pData  - Pointer to the data buffer into which the I/O is done.
 * @param nBytes - Number of bytes to read.
 * @throw std::string - EOF prior to completing the read.
 * @throw std::string - I/O error on read completion.
 * @throw int - 0 if EOF encountered at start of read.

 */
 
void
CBufferConverter::readOrThrow(void* pData, size_t nBytes) 
{
  if (!std::cin.eof()) {
    std::cin.read(reinterpret_cast<char*>(pData), nBytes);
    if (std::cin.fail()) {
      std::string("I/O failure on stdin");
    }
    size_t nRead = std::cin.gcount();
    if (std::cin.gcount() != nBytes) {
      std::string("EOF on stdin without completing entire read");
    }
    return;			// success
  }
  throw 0;
  
  

}
/**
 * Successfully write a block of data to stdout or throw an exception
 * Describing the failure (most likely failure is a broken pipe
 * 
 * @param p - Pointer to the data to write.
 * @param nBytes - Number of bytes to write.
 * @throw string - in the event bad() is true after the write.
 */
void
CBufferConverter::writeOrThrow(void* pData, size_t nBytes) 
{
  std::cout.write(reinterpret_cast<const char*>(pData), nBytes);
  if (std::cout.bad()) {
    throw std::string("Write to stdout failed");
  }
}
/**
 * Flush the current event buffer to the output file.
 * It's up to the caller to ensure that the event buffer is correctly
 * formatted at all times flush() might be called.
 */
void 
CBufferConverter::flush() 
{
  if(m_pPhysicsBuffer) {
    writeOrThrow(m_pPhysicsBuffer, m_nBufferSize);
    delete []m_pPhysicsBuffer;
    m_pPhysicsBuffer = 0;
  }
}

/**
 * Output a scaler buffer.
 * @param pItem - Pointer to the ring item.  This comes in as a RingItem* but
 *                is actually a pScalerItem.
 */
void
CBufferConverter::outputScaler(RingItem* pItem) 
{
  pScalerItemBody pScaler = reinterpret_cast<pScalerItemBody>(bodyPointer(pItem));
  uint16_t    buffer[m_nBufferSize/sizeof(uint16_t)];

  bheader*    pHeader = reinterpret_cast<bheader*>(buffer);
  sclbody*    pBody   = reinterpret_cast<sclbody*>(pHeader+1);

  // Fill in the chunks of the header that are invariant with buffer type:

  fillHeader(pHeader, SCALERBF);
  pBody->etime = pScaler->s_intervalEndOffset;
  pBody->btime = pScaler->s_intervalStartOffset;
  uint32_t nScalers = pHeader->nevt
                    = pScaler->s_scalerCount;
  for (int i = 0; i < nScalers; i++) {
    pBody->scalers[i] = pScaler->s_scalers[i];
  }

  // Figure out the bytes and word count:

  uint16_t nBytes = sizeof(bheader) + sizeof(sclbody) + (nScalers-1)*sizeof(uint32_t);
  pHeader->nwds   = nBytes/sizeof(uint16_t);

  // Output the buffer:

  writeOrThrow(buffer, m_nBufferSize);


}

/**
 * Output a string array buffer.
 * These are buffers that contain an array of strings.  Each array is terminated
 * by a sufficient number of nulls to ensure it starts on a word boundary (even byte boundary).
 *
 * @param itemType - NSCL type of the resultant buffer.
 * @param pItem    - Pointer to the ring item we are outputting.
 *                   This really a pTextItem.
 *
 */
void
CBufferConverter::outputStringArray(uint16_t itemType, RingItem* pItem) 
{
  pTextItemBody pStrings = reinterpret_cast<pTextItemBody>(bodyPointer(pItem));
  uint16_t    buffer[m_nBufferSize/sizeof(uint16_t)];
  uint32_t    nstrings = isSwapped(*pItem) ? swal(pStrings->s_stringCount) : pStrings->s_stringCount;

  bheader*    pHeader = reinterpret_cast<bheader*>(buffer);
  char*    pBody   = reinterpret_cast<char*>(pHeader+1);
  char*       pSrc    = reinterpret_cast<char*>(pStrings->s_strings);

  fillHeader(pHeader, itemType);
  pHeader->nwds       = sizeof(bheader)/sizeof(uint16_t);
  pHeader->nevt       = 0;
  memset(pBody, 0, m_nBufferSize - sizeof(bheader));

  int i;			// Need body scoping.
  for (i =0; i < nstrings; i++) {

    // Figure out the size of the string .. if it fits fine.
    // if not, output the buffer and reset pointers etc. to start a new buffer.
    
    size_t srclen = strlen(pSrc);
    size_t destLen = ((srclen + 2)/2 )*2; // +1 for null +1 to force it all to be an even # of bytes.
    if ((destLen + pHeader->nwds * sizeof(uint16_t)) < m_nBufferSize) {
      strcpy(pBody, pSrc);
      pBody += destLen;		// Buffer is pre-initialized to null.
      pSrc  += srclen + 1;	// +1 counts the null terminator.
      pHeader->nevt++;
      pHeader->nwds += destLen;

    }
    else {
      writeOrThrow(buffer, m_nBufferSize);
      char*    pBody   = reinterpret_cast<char*>(pHeader+1);
      char*    pSrc    = pStrings->s_strings;
      
      pHeader->nwds       = sizeof(bheader)/sizeof(uint16_t);
      pHeader->nevt       = 0;
      memset(pBody, 0, m_nBufferSize - sizeof(bheader));     
      i--;			// Gotta re do this string.
    }
    
  }
  // If there's a buffer with non-zero event count need to write it too:

  if (pHeader->nevt) {
    writeOrThrow(buffer, m_nBufferSize);
  }

}

/**
 * Handle a state change item.  This produces a buffer with a ctlbody.
 * 
 * @param itemType - Buffer type to use for this one.
 * @param pItem    - Pointer to ring item which is actually a 
 *                   pStateChangeItem
 *
 * @todo this code assumes time_t is a long.
 */
void
CBufferConverter::outputStateChange(uint16_t itemType, RingItem* pItem) 
{
  pStateChangeItemBody pSc = reinterpret_cast<pStateChangeItemBody>(bodyPointer(pItem));
  uint16_t         buffer[m_nBufferSize/sizeof(uint16_t)];

  bheader* pHeader = reinterpret_cast<bheader*>(buffer);
  ctlbody* pBody   = reinterpret_cast<ctlbody*>(pHeader+1);


  // We know the run number now so save it so this and all fillHeader calls can use it:
  // This has to correctly consider byte reordering.
  
  m_nRunNumber = isSwapped(*pItem) ? swal(pSc->s_runNumber) : pSc->s_runNumber;

  fillHeader(pHeader, itemType);


  // Need to create a bftime from the time_t timestamp in the buffer.

  // Here's the time_t === long === uint32_t assumption:

  time_t t = isSwapped(*pItem) ? swal(pSc->s_Timestamp) : pSc->s_Timestamp;
  struct tm timeStruct;
  localtime_r(&t, &timeStruct);

  pBody->tod.month = timeStruct.tm_mon +1;
  pBody->tod.day   = timeStruct.tm_mday;
  pBody->tod.year  = timeStruct.tm_year + 1900;
  pBody->tod.hours = timeStruct.tm_hour;
  pBody->tod.min   = timeStruct.tm_min;
  pBody->tod.sec   = timeStruct.tm_sec;
  pBody->tod.tenths = 0;

  // Now the remainder of the data:

  pBody->sortim   = pSc->s_timeOffset;
  memset(pBody->title, 0, 80);
  strncpy(pBody->title, pSc->s_title, 79); // ensure null termination.


  // And the nwds which is fixed for these buffers:

  pHeader->nwds = (sizeof(bheader) + sizeof(ctlbody))/sizeof(uint16_t);

  // Write the buffer out:


  writeOrThrow(buffer, m_nBufferSize);

}

/**
 * Build event buffers.  Event buffers are somewhat different than other buffers in that 
 * several events are put in a single buffer.
 * This is handled using the following member data:
 * - m_pPhysicsBuffer - Pointer to the buffer being constructed.  If this is null, 
 *                     no buffer is being constructed.
 * - m_pPut           - Location in the body of the physics buffer where the next event
 *                      is going.
 * - m_nBufferSize    - Size in bytes of buffers.
 * - the bheader of the m_pPhysicsBuffer which keeps running track of
 *   - nwds - Number of 16 bit words that have been inserted into the buffer to date.
 *   - nevt - Number of events that have been inserted into the bufer to date.
 */
void 
CBufferConverter::bufferEvent(RingItem* pItem) 
{
  // 

  pPhysicsEventItem pEvent = reinterpret_cast<pPhysicsEventItem>(pItem);
  uint16_t* pEventBody     = reinterpret_cast<uint16_t*>(bodyPointer(pItem));


  // If there is no buffer we need to create one:

  if (!m_pPhysicsBuffer) createBuffer();
  bheader*          pHeader= reinterpret_cast<bheader*>(m_pPhysicsBuffer);

  // If the event won't fit we need to flush the current buffer and create a new one:
  // Note the number of words in the event is in the first word of the item payload
  // by NSCL convention:
  // 
  uint16_t nEventSize = isSwapped(*pItem) ? swaw(pEventBody[0]) : pEventBody[0];
  if( (nEventSize + pHeader->nwds)*sizeof(uint16_t) > m_nBufferSize) {
    flush();
    createBuffer();
    pHeader= reinterpret_cast<bheader*>(m_pPhysicsBuffer);
 
  }

  // If the event still won't fit we're in deep as that means the event is bigger than our
  // buffer size:

  assert((nEventSize + pHeader->nwds)*sizeof(uint16_t) <= m_nBufferSize);

  // Insert the event, update the pointers counters etc:
  // note that the event size is a 16 bit word in the compibility mode
  // but 32 bits here:

  m_pPut[0] = nEventSize - 1;
  memcpy(&m_pPut[1], &pEventBody[2], (nEventSize-2)*sizeof(uint16_t));

  // Keep the books on where the next event goes, how many words and events are in the
  // buffer and how many events we've seen so far in our life.

  m_pPut += nEventSize -1;
  pHeader->nwds += nEventSize-1;
  pHeader->nevt++;
  m_nTriggersProcessed++;
  
}
/**
 * Fill in the 'standard' elements of the buffer header. 
 * For now we're going to use native byte ordered signatures.
 * Really we should do something pretty nasty like write localhost'
 * formatted headers and then, if the ring is a different byte ordering
 * we'd re-order the entier buffer.
 *
 * @param pDest - The header to fill in.
 * @param type  - The buffer type to use.
 *
 * @note fields that are not filled in (nevt, nwds, unused) are not gauranteed to have 
 *       specific value.
 * @note cks is set to zero.
 * @note Seq comes from m_nlastSequence, and run comes from m_nRunNumber.
 */
void
CBufferConverter::fillHeader(void* pDest, uint16_t type)
{
  bheader* pHeader = reinterpret_cast<bheader*>(pDest);

  pHeader->type = type;
  pHeader-> cks = 0;
  pHeader->run = m_nRunNumber;
  pHeader->seq = m_nLastSequence;
  pHeader->nlam = 0;
  pHeader->cpu = 0;
  pHeader->nbit = 0;
  pHeader->buffmt = BUFFER_REVISION;
  pHeader->ssignature = 0x0102;
  pHeader->lsignature = 0x01020304;


}
/**
 * Create a new event buffer.  All of this includes
 * setting the put cursor etc.
 */
void
CBufferConverter::createBuffer()
{
  // It's an error to do this if there's an unflushed buffer:

  assert(!m_pPhysicsBuffer);
  getSequence();		// update the sequence data.
  m_pPhysicsBuffer = new uint16_t[m_nBufferSize/sizeof(uint16_t)];
  bheader* pHeader = reinterpret_cast<bheader*>(m_pPhysicsBuffer);
  fillHeader(pHeader, DATABF);

  pHeader->nwds = sizeof(bheader)/sizeof(uint16_t);
  pHeader->nevt = 0;
  m_pPut = reinterpret_cast<uint16_t*>(pHeader+1);
  
}

/**
 * Compute the size of a ring item given is header:
 * @param header - reference to a ring header.
 * @return uint32_t 
 * @retval Item size (includes header size) in bytes in the
 *         host's system representation.
 */
uint32_t
CBufferConverter::computeSize(RingItem& header)
{
  if(isSwapped(header)) {
    return swal(header.s_header.s_size);
  }
  else {
    return header.s_header.s_size;
  }
}

/**
 * Determines if the header passed in was geneated on a system with different
 * byte ordering than the host.  The analysis is done by looking at the
 * Type longword which is supposed to have the most significant 16 bits zero.
 * @param header - reference to the rnig header to be analyzed.
 * @return bool
 * @retval true -if the header was generated on a system with differnt byte ordering.
 * @retval false - if the header was not swapped.
 */
bool
CBufferConverter::isSwapped(RingItem& header)
{
  return ((header.s_header.s_type & 0xffff0000) != 0);
}


/**
 ** Return the type of a ring item... if necessary, they type is byteswapped
 ** @param header - Reference to a ring item header.
 ** @return uint32_t
 ** @return Ring item type.
 */
uint32_t 
CBufferConverter::getType(RingItem& header)
{
  return isSwapped(header) ? swal(header.s_header.s_type) :  header.s_header.s_type;
}
/**
 * Given a ring item type, return the corresponding NSCL DAQ buffer type.
 *
 * @param ringType - Ring item data type.
 * @return uint16_t
 * @retval corresponding item type.
 * @retval 0xffff if the typed does not map.
 *
 */
uint16_t
CBufferConverter::mapType(uint32_t ringType)
{
  stockConversionMap();
  std::map<uint32_t, uint16_t>::iterator p = typeConversionMap.find(ringType);
  if (p != typeConversionMap.end()) {
    return p->second;
  }
  return 0xffff;
}
/**
 * Stock the type conversion map.  This map converts ring buffer types to 
 * NSCL Old style buffer types. The map is sparse in ring buffer types and therefore
 * it is reaalloy and std::map rather than a simple lookup table.
 * 
 * @note If the conversion map is not empty, this is a no-op.
 */
void
CBufferConverter::stockConversionMap()
{
  if(typeConversionMap.empty()) {
    typeConversionMap[BEGIN_RUN]           = BEGRUNBF;
    typeConversionMap[END_RUN]             = ENDRUNBF;
    typeConversionMap[PAUSE_RUN]           = PAUSEBF;
    typeConversionMap[RESUME_RUN]          = RESUMEBF;
    typeConversionMap[PACKET_TYPES]        = PKTDOCBF;
    typeConversionMap[MONITORED_VARIABLES] = RUNVARBF;
    typeConversionMap[PERIODIC_SCALERS] = SCALERBF;
    typeConversionMap[PHYSICS_EVENT]       = DATABF;
    typeConversionMap[PHYSICS_EVENT_COUNT] = TRIGGERSTATS;

  }
}


/**
 * Reset the counters that are involved in managing the sampling
 * fraction statistics.
 */
void
CBufferConverter::resetStatistics()
{
  m_nLastSequence      = 0;
  m_nTriggers          = 0;
  m_nTriggersProcessed = 0;
  m_fLastFloatingSequence = 0.0;
}
/**
 * Process an trigger statistics item (PhysicsEventCountItem).
 * This produces no output, it just updates m_nTriggersProcessed.
 * it's important to do the byte ordering correctly with this one.
 * as we are going to do some computing with it to produce
 * event sequence numbers.
 *
 *  @paeram pItem - pointer to a ring item.
 */
void
CBufferConverter::computeStatistics(RingItem* pItem)
{
  pPhysicsEventCountItemBody pStats = reinterpret_cast<pPhysicsEventCountItemBody>(bodyPointer(pItem));

  m_nTriggers =  isSwapped(*pItem) ? swaq(pStats->s_eventCount) : pStats->s_eventCount;
}
/**
 * Returns the next sequence number of an event buffer consistent with the 
 * sampling fraction computed described by m_nTriggers an d
 * m_nTriggersProcessed.  This is done by adding the 1/sampling fraction to 
 * m_fLastFloatingSequence and truncating that to an integer.
 * Note that by definition 1/sampling fraction > 1.0
 * If the sampling fractino is not 1.0 then eventually this causes a skip in the integer
 * sequence.
 * 
 * @return uint32_t
 * @retval the next event sequence number to use.
 * @note - this is also stored in m_nLastSequence so that it can be retrieved quickly by non
 *         event buffer makers.
 */
uint32_t
CBufferConverter::getSequence()
{
  // Note that if the triggers processed is 0.0 we haven't seen one of those
  // yet and we must assume 1.0 sampling until we do:

  double increment = 1.0;
  if (m_nTriggers != 0) {
    increment = static_cast<double>(m_nTriggers)/static_cast<double>(m_nTriggersProcessed);
  }
  m_fLastFloatingSequence += increment;
  m_nLastSequence = m_fLastFloatingSequence;
  return m_nLastSequence;

}
