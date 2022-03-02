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

#include "glom.h"
#include "fragment.h"
#include "fragio.h"
#include <iostream>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <DataFormat.h>
#include <CRingItemFactory.h>
#include <CRingScalerItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <stdexcept>
#include <CAbnormalEndItem.h>
#include <CBufferedOutput.h>
#include <time.h>
#include "CBufferedFragmentReader.h"
#include "CEventAccumulatorSimple.h"

// File scoped  variables:

static uint64_t firstTimestamp;
static uint64_t lastTimestamp;
static uint64_t timestampSum;
static uint64_t fragmentCount;
static uint32_t sourceId;

static uint64_t outputEvents(0);


static bool     firstEvent(true);
static io::CBufferedOutput* outputter;

static CEventAccumulatorSimple* pAccumulator;

// We don't need threadsafe event fragment pools so:

namespace EVB {
    extern bool threadsafe;
}

static const unsigned BUFFER_SIZE=1024*1024;

/**
 * policyFromEnum
 *    @param timestamp policy enum value from gengetopt.
 *    @return CEventAccumulator::TimestampPolicy - corresponding policy value.
 */
CEventAccumulatorSimple::TimestampPolicy
policyFromEnum(enum_timestamp_policy policy)
{
    switch (policy) {
    case timestamp_policy_arg_earliest:
        return CEventAccumulatorSimple::first;
    case timestamp_policy_arg_latest:
        return CEventAccumulatorSimple::last;
    case timestamp_policy_arg_average:
        return CEventAccumulatorSimple::average;
    }
    throw std::invalid_argument("Invalid timestamp enumerator value.");
}

/**
 *  The next items are used to minimize  dynamic storage manipulations
 *  requred by Glom as profiling shows the realloc of pAccumulatedEvent
 *  to have been a significant part of the glom time:
 *
 *  pAccumulatedEvent - this is storage into which event fragments are buit into
 *  events.
 *
 *  pCurosr - this indicates where in pAccumulatedEvent the next chunk of  data
 *            should be put.
 *
 *  eventStorageSize - indicates the current size of pAccumulatedEvent.
 *
 *  totalEventSize - is the number of bytes that are actually present in
 *                   pAccumulatedEvent.
 */


static uint8_t*        pAccumulatedEvent(0);   
static uint8_t*        pCursor(0);
static size_t          eventStorageSize;
static size_t          totalEventSize(0);

static bool            nobuild(false);
static enum enum_timestamp_policy timestampPolicy;
static unsigned        stateChangeNesting(0);

static unsigned  maxFragments;  // See gitlab issue lucky 13.



static void
dump(std::ostream& o, void* pData, size_t nBytes)
{
    uint16_t* p(reinterpret_cast<uint16_t*>(pData));
    size_t  nwds(nBytes/sizeof(uint16_t));
    
    o << std::hex;
    for (int l = 0; l < nBytes; l += 8) {
        for (int i = 0; i < 8; i++) {
            if (l+i >= nBytes) break;
            o << *p << " ";
            p++;
        }
        o << std::endl;
    }
    
    o << std::dec << std::endl;
}
/**
 * expandAccumulatedEvent
 *    Increase the accumulated event storage by the indicated amount.
 *
 * @param nBytes - Amount of storage to add to pAccumulated event.
 * @note  pCursor is recomputed as is eventStorageSize
 */
static void expandAccumulatedEvent(size_t nBytes)
{
    eventStorageSize += nBytes;
    pAccumulatedEvent =
        static_cast<uint8_t*>(realloc(pAccumulatedEvent, eventStorageSize));
    pCursor = pAccumulatedEvent + totalEventSize;
}

/**
 * resetAccumulatedEvent
 *    Resets pCursor to the beginning of the accumulated event and
 *    sets totalEventSize -> 0
 *
 *    Doing this, rather than free, allows this storage to be re-used by the
 *    next event.
 */
static void
resetAccumulatedEvent()
{
    pCursor = pAccumulatedEvent;
    totalEventSize = 0;
}
/**
 * addDataToAccumulatedEvent
 *    Adds more data to the accumulated event.
 *    - If the data won't fit, realloc is done and pCursor/eventStorageSize
 *      are adjusted,
 *    - Data are copied into the pAccumulatedEvent at pCursor
 *      and pCurosr/totalEventSize adjusted accordingly
 *
 * @param pData - pointer to the data to insert.
 * @param nBytes - Number of bytes of data to insert.
 *
 * @note the idea is that eventually the storage required by the
 * pAccumluated event will equilibrate to the largest event in the data stream.
 * Thus over time, there will be fewer and fewer realloc's.
 */
static void
addDataToAccumulatedEvent(void* pData, size_t nBytes)
{
    if ((nBytes + totalEventSize) > eventStorageSize) {
        expandAccumulatedEvent(nBytes);   // too big is just fine.
    }
    memcpy(pCursor, pData, nBytes);
    totalEventSize += nBytes;
    pCursor        += nBytes;
}

/**
 * outputGlomParameters
 *
 * Output a GlomParameters ring item that describes how we are operating.
 *
 * @param dt - Build time interval.
 * @param building - True if building.
 */
static void
outputGlomParameters(uint64_t dt, bool building)
{
    pGlomParameters p = formatGlomParameters(dt, building ? 1 : 0,
                                             timestampPolicy);
    outputter->put( p, p->s_header.s_size);
    outputter->flush();                       // We buffer via the event accumulator.
}

/**
 * flushEvent
 * 
 * Flush the physics event that has been accumulated
 * so far.
 *
 * If nothing has been accumulated, this is a noop.
 *
 */
static void
flushEvent()
{
  pAccumulator->finishEvent();    
  outputEvents++;                  // Count the event.
  
}

/**
 * outputEventCount
 *    Outputs an event count item with our counters and our output
 *    event source.  This is done to provide SpecTcl with event count items
 *    that reflect the built events.  The intent is to provide the tools to
 *    resolve https://git.nscl.msu.edu/daqdev/SpecTcl/issues/173.
 *
 * @param pItem - pointer to a periodic scaler raw ring item that will trigger the
 *                emission.  Note that the run offset and divisor will be
 *                taken from this item.
 */
static void
outputEventCount(pRingItemHeader pItem)
{
    pAccumulator->flushEvents();      // Write any complete events to output.
    
    // Forma the physics event count item.
    
    CRingScalerItem* pScaler = dynamic_cast<CRingScalerItem*>(
        CRingItemFactory::createRingItem(pItem)  
    );
    if (!pScaler) return;                // Failed convert.
    
    uint32_t tOffset = pScaler->getEndTime();
    uint32_t divisor = pScaler->getTimeDivisor();
    
    CRingPhysicsEventCountItem counters(
        NULL_TIMESTAMP, sourceId, 0, outputEvents, tOffset, 
        time(nullptr), divisor
    );
    // TODO
    outputter->put(
        counters.getItemPointer(), counters.getItemPointer()->s_header.s_size
    );
    outputter->flush();
}

/**
 * outputBarrier
 *
 *  Outputs a barrier event. The ring item type of a barrier
 *  depends:
 *  - If the payload can be determined to be a ring item,
 *    it is output as is.
 *  - If the payload can't be determined to be a ring item,
 *    the entire fragment, header and all is bundled
 *    into a ring item of type EVB_UNKNOWN_PAYLOAD
 *    this is an extension that hopefully helps us deal with
 *    non NSCL DAQ things.
 *
 * @param p - Pointer to the ring item.
 *
 */
static void
outputBarrier(EVB::pFlatFragment p)
{
    pRingItemHeader pH = 
      reinterpret_cast<pRingItemHeader>(p->s_body); 
  
    pAccumulator->addOOBFragment(p, sourceId);

    // This is correct if there is or isn't a body header in the payload
    // ring item.
    
    
    if (pH->s_type == BEGIN_RUN) {
      outputEvents = 0;
      stateChangeNesting++;
    }
    if (pH->s_type == END_RUN) {
      stateChangeNesting--;
    }
    if (pH->s_type == PERIODIC_SCALERS) {
        outputEventCount(pH);
    }
    if (pH->s_type == ABNORMAL_ENDRUN) stateChangeNesting = 0;
    

}
/**
 * emitAbnormalEnd
 *    Emits an abnormal end run item.
 */
void emitAbnormalEnd()
{
    CAbnormalEndItem end;
    pRingItem pItem= end.getItemPointer();
    uint8_t fragmentStorage[pItem->s_header.s_size + sizeof(EVB::FragmentHeader)];
    EVB::pFlatFragment pF = reinterpret_cast<EVB::pFlatFragment>(fragmentStorage);
    pF->s_header.s_timestamp = NULL_TIMESTAMP;
    pF->s_header.s_sourceId = 0xffffffff;
    pF->s_header.s_size     = pItem->s_header.s_size;
    pF->s_header.s_barrier  = 0;
    memcpy(pF->s_body, pItem, pItem->s_header.s_size);
    outputBarrier(pF);
}

/**
 * acumulateEvent
 * 
 *  This function is the meat of the program.  It
 *  glues fragments together (header and payload)
 *  into a dynamically resized chunk of memory pointed
 *  to by pAccumulatedEvent where  totalEventSize 
 *  is the number of bytes that have been accumulated 
 *  so far.
 *
 *  firstTimestamp is the timestamp of the first fragment
 *  in the acccumulated data.though it is only valid if 
 *  firstEvent is false.
 *
 *  Once the event timestamp exceeds the coincidence
 *  interval from firstTimestamp, the event is flushed
 *  and the process starts all over again.
 *
 * @param dt - Coincidence interval in timestamp ticks.
 * @param pFrag - Pointer to the next event fragment.
 */
void
accumulateEvent(uint64_t dt, EVB::pFlatFragment pFrag)
{
  // See if we need to flush:

  uint64_t timestamp = pFrag->s_header.s_timestamp;
 // std::cerr << "fragment: " << timestamp << std::endl;

  // This bit of kludgery is because we've observed that the
  // s800 emits data 'out of order' from its filter. Specifically,
  // it emits scaler data with larger timestamps than the next
  // physics event.  This causes second level event builders to
  // declare an out of order/data late condition which can result in
  // slightly out of order fragments.
  // We're going to compute timestamp differences in 'both directions'
  // and consider ourselves inside dt if either of them makes the
  // window.
  //
  
  uint64_t tsdiff1    = timestamp-firstTimestamp;
  uint64_t tsdiff2    = firstTimestamp - timestamp;
  uint64_t tsdiff     = (tsdiff1 < tsdiff2) ? tsdiff1 : tsdiff2;
  
  /**
   * Flush the currently accumulated event if any of the following
   * hold:
   *    *   We're not in --build mode so fragemts are events.
   *    *   This fragment's timestamp is outside the coincidence interval
   *    *   There are just too many fragments (timestamp stuck).
   */
  
  if (nobuild || (!firstEvent && ((tsdiff) > dt)) || (fragmentCount > maxFragments)) {
    pAccumulator->finishEvent();
    firstEvent = true;                // next event is first.
  }
  // If firstEvent...our timestamp starts the interval:

  if (firstEvent) {
    firstTimestamp = timestamp;
    firstEvent     = false;
    fragmentCount  = 0;
    timestampSum   = 0;
  }
  lastTimestamp    = timestamp;
  fragmentCount++;
  
  timestampSum    += timestamp;

  pAccumulator->addFragment(pFrag, sourceId);
  
    // Add the data to the accumulated event:
  


}

static void outputEventFormat()
{
    // std::cerr << "Outputting event format element\n";
    DataFormat format;
    format.s_header.s_size = sizeof(DataFormat);
    format.s_header.s_type = RING_FORMAT;
    format.s_mbz         = 0;
    format.s_majorVersion = FORMAT_MAJOR;
    format.s_minorVersion = FORMAT_MINOR;
    
    outputter->put( & format, sizeof(format));
    outputter->flush();
}

/**
 * Main for the glommer
 * - Parse the arguments and extract the dt.
 * - Until EOF on input, or error, get fragments from stdin.
 * - If fragments are not barriers, accumulate events
 * - If fragments are barriers, flush any accumulated 
 *   events and output the barrier body as a ring item.
 *
 * @param argc - Number of command line parameters.
 * @param argv - array of pointers to the parameters.
 */
int
main(int argc, char**  argv)
{
  // Parse the parameters;

  gengetopt_args_info args;
  cmdline_parser(argc, argv, &args);
  int dtInt = static_cast<uint64_t>(args.dt_arg);
  nobuild      = args.nobuild_given;
  timestampPolicy = args.timestamp_policy_arg;
  sourceId       = args.sourceid_arg;
  maxFragments   = args.maxfragments_arg;

  EVB::threadsafe = false;     // Don't need threadsafe fragment pools.

  outputter = new io::CBufferedOutput(STDOUT_FILENO, BUFFER_SIZE);
  outputter->setTimeout(2);    // Flush every two sec if data rate is slow.
  
    pAccumulator = new CEventAccumulatorSimple(
        STDOUT_FILENO, 2, BUFFER_SIZE, maxFragments,
        policyFromEnum(timestampPolicy)
    );
  
  outputEventFormat();
  

  std::cerr << (nobuild ? " glom: not building " : "glom: building events: ");
  if (!nobuild) {
    std::cerr << dtInt << std::endl;
  } else {
    std::cerr << std::endl;
  }

  if (!nobuild && (dtInt < 0)) {
    std::cerr << "Coincidence window must be >= 0 was "
	      << dtInt << std::endl;
    exit(-1);
  }
  uint64_t dt = static_cast<uint64_t>(dtInt);
  nobuild      = args.nobuild_flag;

  /*
     main loop.. .get fragments and handle them.
     two targets for a fragment:
     accumulateEvent - for non-barriers.
     outputBarrier   - for barriers.
  */

  CBufferedFragmentReader reader(STDIN_FILENO);
  bool firstBarrier(true);
  bool consecutiveBarrier(false);
  try {
        while (1) {
          const EVB::pFlatFragment p = reader.getFragment();
          
          // If error or EOF flush the event and break from
          // the loop:
          
          if (!p) {
            flushEvent();
            pAccumulator->flushEvents();
            std::cerr << "glom: EOF on input\n";
                if(stateChangeNesting) {
                    emitAbnormalEnd();
                }
            break;
          }
          // We have a fragment:
          
          if (p->s_header.s_barrier) {
            flushEvent();
            pAccumulator->flushEvents();
            outputBarrier(p);
            
            
            // Barrier type of 1 is a begin run.
            // First begin run barrier will result in
            // emitting a glom parameter record.
    
            
            if(firstBarrier && (p->s_header.s_barrier == 1)) {
                outputGlomParameters(dtInt, !nobuild);
                firstBarrier = false;
            }
          } else {
    
            // Once we have a non barrier, reset firstBarrier so that we'll
            // emit a glom parameters next time we have a barrier.
            // This is needed if the event builder is run in persistent mode.
            // see gitlab issue #11 for nscldaq.
            
            firstBarrier = true;
            
            // If we can determine this is a valid ring item other than
            // an event fragment it goes out out of band but without flushing
            // the event.
        
            pRingItemHeader pH = reinterpret_cast<pRingItemHeader>(p->s_body);
        
            
            if (pH->s_type == PHYSICS_EVENT) {
              accumulateEvent(dt, p); // Ring item physics event.
            } else {
              outputBarrier(p);	// Ring item non-physics event.
            }
            
        }
          
    }
  }
  catch (std::string msg) {
    std::cerr << "glom: " << msg << std::endl;
  }
  catch (const char* msg) {
    std::cerr << "glom: " << msg << std::endl;
  }
  catch (int e) {
    std::string msg = "glom: Integer error: ";
    msg += strerror(e);
    std::cerr << msg << std::endl;
  }
  catch (std::exception& except) {
    std::string msg = "glom: ";
    msg += except.what();
    std::cerr << msg << std::endl;
  }
  catch(...) {
    std::cerr << "Unanticipated exception caught\n";

  }
    // Out of main loop because we need to exit.

  return 0;
}
