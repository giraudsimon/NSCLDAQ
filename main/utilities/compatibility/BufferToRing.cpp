/**
// Purpose:
//   Convert NSCL buffered data to ring buffer data.  This is a filter specifically,
//   data buffera are processed on stdin and sent to stdout in a format that is 
//   usable by stdinto ring.
//    
*/
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/



#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <time.h>
#include <set>
#include <map>
#include <io.h>
#include <iostream>

#include <buffer.h>
#include <buftypes.h>
#include <DataFormat.h>
#include "btoroptions.h"
#include <dlfcn.h>
#include <fragment.h>

typedef uint64_t (*TimestampExtractor)(void*);

static size_t       BUFFERSIZE(8192); // (in bytes).
static uint64_t     eventsInRun(0);   // For PhysicsEventcountItems.

static std::set<int>                 okErrors;	// Acceptable errors in I/O operations.
static std::map<uint16_t, uint32_t>  textTypeMap; // map of text buffer types -> ring buffer item types.
static std::map<uint16_t, uint32_t>  stateTypeMap; // Same as above but for state change buffers.

// Counter used for run statistics:

static uint64_t triggers;

// Pulled out command line parameters.

static bool         createBodyHeaders(false);
static bool         incrementalScalers(false);
static unsigned     sourceId(0);
static std::string  timeStampLibrary;
static TimestampExtractor eventExtractor(0);   // getEventTimestamp
static TimestampExtractor scalerExtractor(0);  // getScalerTimestamp

bool missingEventExtractorWarned(false);
bool missingScalerExtractorWarned(false);

/**
 * findSymbol
 *    Finds a dll symbol for a timestamp extractor and saves it
 *    somewhere:
 *
 *  @param ppFunction - Pointer to where to store the final function
 *  @param dll        - DLL handle.
 *  @param name       - Name of the function.
 *
 *  @note This function does no error reportage.  That's up to the caller.
 *        ppFunction will have a 0 stored into it if the lookup failed.
 *        dlerror can then be used to fetch the error.
 */
static void
findSymbol(TimestampExtractor* ppFunction, void* dll, const char* name)
{
    void* pFunction = dlsym(dll, name);
    *ppFunction = reinterpret_cast<TimestampExtractor>(pFunction);
}

/**
 * loadTimestampLibrary
 *
 *   Load the timestamp library and locate extractor references.
 *   Note that since there are cases where data sources may be totally
 *   made up of scalers or totally events, we're only going to require
 *   that we have at least one of those.
 *
 * @note timeStampLibrary has the name of the shared object.
 * @note Errors are not recoverable and the programwill exit with an error.
 */
static void
loadTimestampLibrary()
{
    // Open the shared lib:
    
    void* pDllHandle = dlopen(timeStampLibrary.c_str(), RTLD_NOW | RTLD_NODELETE);
    if (!pDllHandle) {
        std::cerr << "Failed to load the timestamp extraction library: "
            << timeStampLibrary << dlerror() << std::endl;
        exit(EXIT_FAILURE);
    }
    findSymbol(&eventExtractor, pDllHandle, "getEventTimestamp");
    findSymbol(&scalerExtractor, pDllHandle, "getScalerTimestamp");

    if ((!eventExtractor) && (!scalerExtractor)) {
        std::cerr << "The library must have at least one of 'getEventTimestamp "
            << "or getScalerTimestamp defined but does not\n";
        exit(EXIT_FAILURE);
    }
}

/**
 * setOptions
 *    Process the command line options.
 *    If there are errors, the program fails.
 *  @param argc  - Number of command line parameters.
 *  @param argv  - The commandl ine parameters themselves.
 *
 * @note the file global variables above are pulled out.
 */
static void
setOptions(int argc, char** argv)
{
    // Parse the options:
    
    struct gengetopt_args_info args;
    if(cmdline_parser(argc, argv, &args)) {
        std::cerr << "Failed to process the command line\n";
        exit(EXIT_FAILURE);
    }
    
    /*
     * incremental scalers are independent of all the bod header crap:
     * Default is true if --incremental-scalers not given otherwise the
     * flag value rules:
     */
    
    incrementalScalers =
        args.incremental_scalers_given ?
            (args.incremental_scalers_arg == incremental_scalers_arg_yes) :
            true;
    /*
     *  If --create-body-header is given and is true we need to have all the
     *  related switches too.
     */
    if(args.create_body_header_given &&
       (args.create_body_header_arg == create_body_header_arg_yes))
    {
        if (!args.sourceid_given || !args.ts_extract_given) {
            std::cerr <<
                "If --create-body-header is true then " <<
                "--sourceid and --tsextract are required flags\n";
            exit(EXIT_FAILURE);
        }
        
        sourceId          = args.sourceid_arg;
        timeStampLibrary = args.ts_extract_arg;
        loadTimestampLibrary();
        createBodyHeaders = true;
    }
    
    
    BUFFERSIZE = args.buffersize_arg;
    if (BUFFERSIZE == 0) {
      std::cerr <<
        "FAILURE: The --buffersize must be greater than 0\n";
      exit(EXIT_FAILURE);
    }
}

/**
 * Get a buffer of data from stdin (STDIN_FILENO).
 * If necessary multiple read() operation are performed to deal
 * with potential buffering between the source an us (e.g. we are typically
 * on the ass end of a pipe where the pipe buffer may be smaller than an
 * event buffer.
 * @param pBuffer - Pointer to a buffer big enough to hold the event buffer.
 * @param size    - Number of bytes in the buffer.
 *
 * @return bool
 * @retval true - The buffer was read successfully.
 * @retval false - the buffer was not read successfully.
 */
bool getBuffer (uint16_t* pBuffer,  size_t nBytes)
{
  size_t nread;
  try {
    nread = io::readData(STDIN_FILENO, pBuffer, nBytes);
    if (nread < nBytes) return false;
  }
  catch(int e) {
    if (e) {
      fprintf(stderr, "%s : %s\n",
	      "Error on read: ", strerror(e));
    } else {
      fprintf(stderr, "%s\n", "End of file on stdin in mid read");
    }
    return false;
  }
  return true;
}

/**
 * Write a block of data to stdout.
 * As with getBuffer, multiple writes are done..until either the entire data
 * are written or
 * *  A write ends in an eof condition.
 * *  A write ends in an error condition that is not EAGAIN, EWOUDLDBLOCK or EINTR.
 *
 * @param pData - Pointer to the data to write.
 * @param size  - Number of words of data to write.
 * 
 * @return bool 
 * @retval - true on success, false otherwise.
 */

bool writeData (void* pData , size_t size)
{

  try {
    io::writeData(STDOUT_FILENO, pData, size);
  }
  catch(int e) {
    if (e) {
      fprintf(stderr, "%s : %s\n",
	      "Error on write", strerror(e));
    } else {
      fprintf(stderr, "%s\n", "STDOUT closed on us");
    }
    return false;
  }
  return true;
}

/**
 * Map a text buffer type to the closest corresponding
 * Ring item.
 * 
 * @param bufferType  type of the old style buffer.
 * 
 * @return uint32_t ring item type.
 *
 * @note uses the static map textTypeMap, setting it up if it's empty.
 *
 */

uint32_t mapTextBufferType (uint16_t bufferType)
{

  if (textTypeMap.empty())
  {
    textTypeMap[STATEVARBF] = MONITORED_VARIABLES;
    textTypeMap[RUNVARBF]   = MONITORED_VARIABLES;
    textTypeMap[PKTDOCBF]   = PACKET_TYPES;
    textTypeMap[PARAMDESCRIP] = MONITORED_VARIABLES;
  }


  return textTypeMap[bufferType];
}

/**
 * Does the same thing as mapTextBufferType but maps the state transition buffer types.
 * 
 * @param bufferTye - type to map.
 * 
 * @return uint32_t mapped type.
 *
 * @note this uses stateTypeMap to do the mapping, setting it up if needed.
 */

uint32_t mapStateChangeType (uint16_t bufferType)
{

  if (stateTypeMap.empty())
  {
    stateTypeMap[BEGRUNBF] = BEGIN_RUN;
    stateTypeMap[ENDRUNBF] = END_RUN;
    stateTypeMap[PAUSEBF]  = PAUSE_RUN;
    stateTypeMap[RESUMEBF] = RESUME_RUN;
  }

  return stateTypeMap[bufferType];
}



/**
 * Generate phyics data ring items.  Each event results in a write of a correctly
 * formatted ring item to stdout... or an error.  This code assumes the byte ordering
 * is the same as the native system.
 *
 * @param pBuffer - Pointer to the raw data buffer.;
 * 
 * @return bool 
 * @return true - All events in the buffer were successfully written to stdout.
 * @return false - A write to stdout failed or some other error was detected.
 */

bool formatEvents (void* pBuffer)
{
  static    size_t buffers = 0;
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  uint16_t* pBody(reinterpret_cast<uint16_t*>(pHeader+1));
  
  int      wordsLeft(pHeader->nwds - sizeof(bheader)/sizeof(uint16_t));
  int      nEvents(pHeader->nevt);

  RingItemHeader eventHeader;


  // Note: Event size is 32 bits in the ring buffer and 16 bits in the old buffer style.
  // hence the extra sizeof(uint16_t) and the hole after putting in the word count
  // later on:

  for (int i = 0; i < nEvents; i++) {
    uint16_t eventSize = *pBody++;
    pPhysicsEventItem pItem;
    if (createBodyHeaders) {
        if (eventExtractor) {
            uint64_t timestamp = (*eventExtractor)(pBody-1);
            pItem = formatTimestampedEventItem(
                timestamp, sourceId, 0, eventSize - 1, pBody
            );
        } else {
            if (!missingEventExtractorWarned) {
                std::cerr << "Warning your timstamp extractor does not produce event "
                    << "timestamps.  Physics events will not have a body header\n";
                missingEventExtractorWarned = true;
            }
            pItem = formatEventItem(eventSize - 1, pBody);
        }
    } else {
        pItem = formatEventItem(eventSize - 1, pBody);    
    }
    
    bool status = writeData(pItem, itemSize(reinterpret_cast<pRingItem>(pItem)));
    free(pItem);

    if (!status) return false;

    pBody     += eventSize-1;	// Remember the pointer points to the event body.
    wordsLeft -= eventSize;

    if (wordsLeft < 0)
    {
      fprintf(stderr, "**** Bad buffer structure, wordsLeft < 0 (%d) at event %d out of %d\n",
	      wordsLeft, i, nEvents);

      return false;
    }
  }
  // If there are words left that's an error worth reporting too:


  if(wordsLeft) {
    fprintf(stderr, "*** Bad buffer structure, %d words left but no more events according to count (buffer %d)\n",
	    wordsLeft, buffers);
    return false;
  }

  triggers += nEvents;		// Update trigger count seen so far.
  buffers++;
  return true;
}

/**
 * Format a trigger count item and write it to stdout
 *
 * @param runTime - Offset into the run.
 * @param stamp   - Absolute timestamp.
 *
 * @return bool - false if failed to write it correctly.
 */
bool formatTriggerCount(uint32_t runTime, time_t stamp)
{
  pPhysicsEventCountItem pItem;
  if(createBodyHeaders) {
    pItem = formatTimestampedTriggerCountItem(
        NULL_TIMESTAMP, sourceId, 0, runTime, 1, stamp, triggers
    );
  } else {
    pItem = formatTriggerCountItem(runTime, stamp, triggers);
  }
  
  bool status = writeData(pItem, itemSize(reinterpret_cast<pRingItem>(pItem)));
  free(pItem);
  return status;
}

/**
 * scalerTimestamp 
 *
 *  Extract the 64 bit timestamp from the first 4 unused of the scaler buffer body
 *
 * @param pBody - Pointer to the scaler body.
 *
 * @return uint64_t timestamp value.
 */
uint64_t
scalerTimestamp(sclbody* pBody)
{
  uint64_t timestamp;

  if (scalerExtractor) {
    return (*scalerExtractor)(pBody);
  } else {
    if (createBodyHeaders && !missingScalerExtractorWarned) {
        std::cerr << "The timestamp extractor does not have a scaler timestmap "
            << "extraction function.\n  The S800 scaler timestamp extraction"
            << " algorithm will be used\n";
        missingScalerExtractorWarned = true;
    }
    // Fall through to the s800 code.
  }

  timestamp = (uint16_t)pBody->unused2[0]; // Highest order.

  for (int i =0; i < 3; i++) {
    timestamp = timestamp << 16;
    timestamp |= (uint16_t)pBody->unused1[2-i];
  }
  // S800 kludge here:

  timestamp = timestamp << 3;

  // 

  return timestamp;
}
/**
 * scalerTimeDivisor
 *
 *  Returnthe time divisor for the scaler timebase.
 *
 * @param pBody - pointer to the scaler body.
 *
 * @return uint32_t divisor.
 */
uint32_t
scalerTimeDivisor(sclbody* pBody)
{
  uint32_t result;
  result = pBody->unused2[2];	// high order part.
  result = result << 16;	// shift into position.
  result |= pBody->unused2[1];	// or in low order part.

  return result;
}

/**
 * Function to write scaler buffers to file. Note that NSCLBuffers have start of run
 * offsets but not timestamps, rings have timestamps.  At this time we choose to
 * fill in the current time as a timestamp.  In the future we could calculate
 * the timestamp from the run-time offset and start of run timestamp.
 *
 * @param pBuffer - pointer to the buffer.
 *
 * @return bool
 * @retval true on success, false otherwise.
 */
bool formatScaler (void* pBuffer)
{
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  sclbody*  pBody(reinterpret_cast<sclbody*>(pHeader+1));
  uint32_t nScalers = pHeader->nevt;

  uint32_t    scalers[nScalers];
 
  time_t      timestamp;
  time(&timestamp);

  // format a trigger time buffer with our timestamp and the scaler end time:

  if (!formatTriggerCount(pBody->etime, timestamp)) {
    return false;
  }


  
  pScalerItem pTSItem;
  
  if (createBodyHeaders) {
    pTSItem = formatTimestampedScalerItem(
        scalerTimestamp(pBody), sourceId, 0, incrementalScalers,
        scalerTimeDivisor(pBody), timestamp, pBody->btime, pBody->etime,
        nScalers, pBody->scalers
    );
  } else  {
    pTSItem = formatScalerItem(
        nScalers, timestamp, pBody->btime, pBody->etime, pBody->scalers
    );    
  }
   
  int status = writeData(pTSItem, itemSize(reinterpret_cast<pRingItem>(pTSItem)));
  free(pTSItem);

  return status;

}

/**
 * Format a strings data buffer.  The actual type of the buffer depends on the 
 * type of the input buffer:
 * \verbatim 
 *   STATEVARBF -> MONITORED_VARIABLES
 *    RUNVARBF  -> MONITORED_VARIABLES
 *    PKTDOCBF  -> PACKET_TYPE
 *    PARAMDESCRIP -> MONITORED_VARIABLES
 *
 * \endverbatim
 *
 *  Note since no timing information is given in the source data, a current timestamp will be
 *  generated bu the run offset time will be set to 0.
 *
 * @param pBuffer - Pointer to the buffer.
 *
 * @return bool
 * @return true on success, false otherwise.
 */

bool formatStrings (void* pBuffer)
{
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  char*     pBody(reinterpret_cast<char*>(pHeader + 1));
  int       nStrings = pHeader->nevt;

  TextItem header;
  size_t   nStringSize(0);

  time_t stamp;
  time(&stamp);

  const char** pStrings = new const char *[nStrings]; // Will have ptrs to the strings.
  for (int i =0; i < nStrings; i++) {
    pStrings[i]  = pBody;
    
    // Length + null must always be even:

    int length = strlen(pBody) + 1; // length + null.
    if (length %2) length++;	    // Make it even if it's odd.
    pBody += length;		    // next string.
    
  }
  pTextItem pItem;
  if (createBodyHeaders) {
    pItem = formatTimestampedTextItem(
        NULL_TIMESTAMP, sourceId, 0,
        nStrings, stamp, 0, pStrings, mapTextBufferType(pHeader->type), 1
    );
  } else {
    pItem = formatTextItem(
        nStrings, stamp, 0, pStrings, mapTextBufferType(pHeader->type)
    );  
  }
  
  bool status = writeData(pItem, itemSize(reinterpret_cast<pRingItem>(pItem)));
  free(pItem);
  delete [] pStrings;
  return status;

}
/**
 * barrierType
 *
 * @param bufferType - the buffer type of a state transition buffr.
 * @return uint32-t  - Type of barrier implied by this buffer type.
 */
static uint32_t
barrierType(uint16_t bufferType)
{
    switch (bufferType) {
        case BEGRUNBF:
            return 1;
        case ENDRUNBF:
            return 2;
        case PAUSEBF:
            return 3;
        case RESUMEBF:
            return 4;
    }
    return 1;
}
/**
 * Write a state change data buffer. Buffer Type mappings:
 *
 * \verbatim
 *     BEGRUNBF  -> BEGIN_RUN
 *     ENDRNBF   -> END_RUN
 *     PAUSEBF   -> PAUSE_RUN
 *     RESUMEBF  -> RESUME_RUN
 *
 * \endverbatim
 * 
 * @return bool
 * @retval true on success, false on error.
 */

bool formatStateChange (void* pBuffer)
{
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  ctlbody* pBody(reinterpret_cast<ctlbody*>(pHeader+1));

  StateChangeItem item;

  struct tm bTime;
  char      textualTime[1000];
  sprintf(textualTime, "%d-%d-%d %d:%d:%d",
	  bTime.tm_year = pBody->tod.year,
	  bTime.tm_mon  = pBody->tod.month,
	  bTime.tm_mday = pBody->tod.day,
	  bTime.tm_hour = pBody->tod.hours,
	  bTime.tm_min  = pBody->tod.min,
	  bTime.tm_sec  = pBody->tod.sec);

  strptime(textualTime, 
	   "%Y-%m-%d %T", &bTime);
  time_t stamp = mktime(&bTime);
  pStateChangeItem pItem;
  if (createBodyHeaders) {
    pItem = formatTimestampedStateChange(
        NULL_TIMESTAMP, sourceId, 1,
        stamp, pBody->sortim, pHeader->run, barrierType(pHeader->type), pBody->title,
        mapStateChangeType(pHeader->type)
    );
  } else {
    pItem = formatStateChange(
        stamp, pBody->sortim, pHeader->run, pBody->title,
        mapStateChangeType(pHeader->type)
    );    
}
  
  bool status = writeData(pItem, itemSize(reinterpret_cast<pRingItem>(pItem)));
  free(pItem);
  return status;

}


/**
 * Based on the buffer type dispatch to the appropriate ring item
 * generating function.
 *
 * @param pBuffer -  Pointer to the raw data buffer.
 *
 * @return bool
 */
bool bufferToRing (void* pBuffer)
{
  bheader* pHeader(reinterpret_cast<bheader*>(pBuffer));
  int16_t    type = pHeader->type;

  switch (type) {
    case DATABF:
      return formatEvents(pBuffer);
      break;
    case SCALERBF:
    case SNAPSCBF:
      return formatScaler(pBuffer);
      break;
    case STATEVARBF:
    case RUNVARBF:
    case PKTDOCBF:
    case PARAMDESCRIP:
      return formatStrings(pBuffer);
      break;
    case BEGRUNBF:
      triggers = 0;		// Begin run zeroes the trigger count.
    case ENDRUNBF:
    case PAUSEBF:
    case RESUMEBF:
      return formatStateChange(pBuffer);
      break;
    default:
      fprintf(stderr, "Got a buffer whose type we did not expect: %d\n", type);
      fprintf(stderr, "Ingoring the data in this buffer!!\n");
      return false;
      break;
  }
}


int main (int argc, char *argv[])
{

  setOptions(argc, argv);
  uint16_t    dataBuffer[BUFFERSIZE/sizeof(uint16_t)];


  // Process the data:

  while (getBuffer(dataBuffer, BUFFERSIZE)) {
      if(!bufferToRing(dataBuffer)) {
	break;
      }

  }
  return 0;
} 

