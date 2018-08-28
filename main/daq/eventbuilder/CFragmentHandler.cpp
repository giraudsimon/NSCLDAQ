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

#include "CFragmentHandler.h"
#include "fragment.h"

#include <string.h>
#include <assert.h>

#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cstdint>
#include <time.h>
#include "COutputThread.h"

#include "CopyPopUntil.h"

using std::uint32_t;
using std::uint64_t;



/*---------------------------------------------------------------------
** Static  data:
*/

CFragmentHandler* CFragmentHandler::m_pInstance(0);

static const size_t Mega(1024*1024);

static const time_t DefaultBuildWindow(20); // default seconds to accumulate data before ordering.
static const uint32_t IdlePollInterval(1);  // Seconds between idle polls.
static const time_t DefaultStartupTimeout(4); // default seconds to accumulate data before ordering.
static time_t timeOfFirstSubmission(UINT64_MAX); //
static const  size_t defaultXonLimit(9*Mega);     // Default total fragment storage at which we can xon
static const  size_t defaultXoffLimit(10*Mega);    // Default total fragment storage at which we xoff.

/*---------------------------------------------------------------------
 * Debugging
 */

static void hexDump(uint8_t* p, uint32_t bytes) {
  for (int i = 0; i < bytes; i++) {
    unsigned value = *p++;
    std::cerr << value << " ";
    if (((i+1) & 0xf) == 0) {
      std::cerr << std::endl;
    }
  }
}

static void dumpFragment(EVB::pFragment p) {
  std::cerr << "------------------ Fragment -------------------\n";
  std::cerr << "Timestamp:    " << std::hex << p->s_header.s_timestamp << std::dec << std:: endl;
  std::cerr << "Source:       " << p->s_header.s_sourceId << std::endl;
  std::cerr << "PayloadSize:  " << std::hex << p->s_header.s_size << std::dec << std::endl;
  std::cerr << "Barrier type: " << p->s_header.s_barrier << std::endl;
  std::cerr << "Payload: \n";
  std::cerr << std::hex;
  hexDump(reinterpret_cast<uint8_t*>(p->s_pBody), p->s_header.s_size);
  std::cerr << std::dec << std::endl;

}

// Timestamp comparison for sorted merge:

static bool
TsCompare(std::pair<time_t, EVB::pFragment>& q1, std::pair<time_t, EVB::pFragment>& q2)
{
  return q1.second->s_header.s_timestamp < q2.second->s_header.s_timestamp ;
}

/*--------------------------------------------------------------------------
 ** Creationals: Note this is a singleton, constructors are private.
 */

/**
 * CFragmentHandler
 *
 * constructor:
 *   - m_nNewest -> 0, m_nOldest -> maxint64.
 *   - the build window and coincidence windows are set to some
 *     default values -- they should not be left that way.
 *   - m_pInstance -> this.
 */
CFragmentHandler::CFragmentHandler() :
  m_outputThread(*(new COutputThread()))

{
    m_outputThread.start();
    m_nBuildWindow = DefaultBuildWindow;
    m_nStartupTimeout = DefaultStartupTimeout;
    m_pInstance = this;
    resetTimestamps();

    m_nNow = time(NULL);	// Initialize the time.
    m_nOldestReceived = INT32_MAX; // Hopefully that makes it infinitely future.

    // Start the idle poll off:

    m_timer = Tcl_CreateTimerHandler(1000*IdlePollInterval, &CFragmentHandler::IdlePoll, this);
    
    // Set the Xoff values:
    
    m_nXonLimit  = defaultXonLimit;
    m_nXoffLimit = defaultXoffLimit;
    m_fXoffed    = false;
    m_nTotalFragmentSize = 0;
}
/**
 * Destructor - for now just kill off the timer -- don't worry about
 *              memory leaks as the destructor can only be used in tests.
 */
CFragmentHandler::~CFragmentHandler()
{
    Tcl_DeleteTimerHandler(m_timer);
}

/**
 * getInstance
 *
 * This is the public creational.  If m_pInstance doesn't exist,
 * a new CFragmentHandler is created.
 *
 * @return CFragmentHandler*
 * @retval - A pointer to the unique instance of the CFragmentHandler.
 *           note this may have been created by this call.
 */
CFragmentHandler*
CFragmentHandler::getInstance()
{
    // If needed, construct the object:
    
    if (!m_pInstance) {
        new CFragmentHandler();    // Constructor sets m_pInstance.
    }
    return m_pInstance;
}

/*--------------------------------------------------------------
 ** Object operations.
 */

/**
 * addFragments
 *   Adds a set of fragments to their queues.  The fragments
 *   are passed in as an array of flattened fragments with
 *   a total byte count for the fragments.
 *   - Fragments are put in the right queues.
 *   - Where appropriate, the newest/oldest events are updated.
 *   - If appropriate, events are built.
 *
 *   @param nSize      - Number of bytes worth of event fragment data.
 *   @param pFragments - Pointer to the first fragment.
 *
 *   @note This method is a no-op if nSize is 0.
 *   @throw std::string exception if there's an inconsistency between
 *          the fragment sizes and nSize so that fragments
 *          don't end when nSize is exactly zero.
 */
static  EVB::FragmentHeader lastHeader;
static bool first(true);


void
CFragmentHandler::addFragments(size_t nSize, EVB::pFlatFragment pFragments)
{
    m_nNow = time(NULL);
    if (m_nNow < m_nOldestReceived) {
      m_nOldestReceived = m_nNow; // Really done first time.
      m_nMostRecentlyEmptied = m_nNow;
    }

    if (first) {
      timeOfFirstSubmission = m_nNow;
    }
   int frags = 0;
   int srcid = -1;
    while (nSize) {
      EVB::pFragmentHeader pHeader = &(pFragments->s_header);
      frags++;
      srcid = pHeader->s_sourceId;
      size_t fragmentSize = totalFragmentSize(pHeader);
      if((pHeader->s_size + sizeof(EVB::FragmentHeader)) > nSize) {
	std::stringstream s;
	s << "Last fragment has too many bytes: " << nSize
	  << " bytes in fragment group but " << fragmentSize
	  << " bytes are in the last fragment!";
	throw s.str();
      }



      addFragment(pFragments);
      
      // Point to the next fragment.
      
      char* pNext = reinterpret_cast<char*>(pFragments);
      lastHeader = *pHeader;
      first      = false;

      pNext      += fragmentSize;
      pFragments  = reinterpret_cast<EVB::pFlatFragment>(pNext);
      nSize -= fragmentSize;
    }
    
    findOldest();		// Probably not needed but pretty quick.
    if (m_nNow-timeOfFirstSubmission > m_nStartupTimeout) {
      // Don't flush until we have allowed time for all data sources to 
      // establish themselves
      flushQueues();		// flush events with received time stamps older than m_nNow - m_nBuildWindow
    }
    checkXoff();
}
/**
 * setBuildWindow
 * 
 * Set the build window.  The build window determines how far apart the
 * oldest and newest event can get in time before events are built.
 * 
 * @param windowWidth - seconds in the build window.
 */
void
CFragmentHandler::setBuildWindow(time_t windowWidth)
{
    m_nBuildWindow = windowWidth;
}
/**
 * getBuildWindow
 *
 * Return the value of the current build window.
 *
 * @return time_t - build window in seconds.
 */
time_t
CFragmentHandler::getBuildWindow() const
{
  return m_nBuildWindow;
}
/**
 * setStartupTimeout
 * 
 * Set the startup timeout duration.  The startup timeout duration determines the amount
 * of time after the first fragment is received when calls to flushQueues is not allowed. This is
 * to ensure that all queues have been created before flushing.
 * 
 * @param duration - the number of seconds to use as the startup timeout
 */
void
CFragmentHandler::setStartupTimeout(time_t duration)
{
    m_nStartupTimeout = duration;
}
/**
 * getStartupTimeout
 *
 * Return the value of the current startup timeout duration
 *
 * @return time_t - duration of the startup timeout 
 */
time_t
CFragmentHandler::getStartupTimeout() const
{
  return m_nStartupTimeout;
}
/**
 * setXonThreshold
 *   Sets the low watermark for flow control in bytes of fragment body.
 *   If the number of queued bytes drops below this while the system
 *   is in the XOffed state, the Xon observers will be fired.
 *
 * @param nBytes - new watermark.
 */
void
CFragmentHandler::setXonThreshold(size_t nBytes) {
    m_nXonLimit = nBytes;
}
/**
 * setXoffThreshold
 *    Sets the high watermark for flow control in bytes of fragment body.
 *    If the number of queued bytes rises above this value while the system
 *    is not Xoffed, the Xoff observers will be fired.
 * @param size_t nBytes new level.
 */
void
CFragmentHandler::setXoffThreshold(size_t nBytes) {
    m_nXoffLimit = nBytes;
}

/**
 * addObserver
 *
 * The fragment handler does not really  know how to deal with built
 * events.  Chunks of code (like an output stage) that want to know
 * about events that have been built register 'observer' objects
 * when an event has been built all registered observers are
 * called with a gather vector of pointer to the fragments that make
 * up the event passed as a parameter.
 *
 * This method registers an event built observer.
 *
 * @param pObserver - Pointer to the observer object to register.
 */
void
CFragmentHandler::addObserver(::CFragmentHandler::Observer* pObserver)
{
    m_outputThread.addObserver(pObserver);
}
/**
 * removeObserver:
 *
 * Removes an observer that was registerd by addObserver
 * This method is a no-op if there are no such observers as
 *
 * @param pObserver - Pointer to the observer to remove.
 */
void
CFragmentHandler::removeObserver(::CFragmentHandler::Observer* pObserver)
{
  try {
    m_outputThread.removeObserver(pObserver);
  }
  catch (...) {
    // Its a no-op to remove a nonexistent observer at this level.
  }
}
/**
 * addDataLateObserver
 *
 *  Add an observer to the list of objects that are invoked when 
 *  a data late event occurs See dataLate below for information about what
 *  that means.
 * 
 * @param pObserver - pointer to the  new observer.
 */
void
CFragmentHandler::addDataLateObserver(CFragmentHandler::DataLateObserver* pObserver)
{
  m_DataLateObservers.push_back(pObserver);
}
/**
 * removeDataLateObserver
 *   
 * Removes an existing data late observer from the observer list.
 * note that it is not an error to attempt to remove an observer that does
 * not actually exist.
 *
 * @param pObserver - Pointer to the observer to remove.
 */
void
CFragmentHandler::removeDataLateObserver(CFragmentHandler::DataLateObserver* pObserver)
{
  std::list<DataLateObserver*>::iterator p = find(
					  m_DataLateObservers.begin(), m_DataLateObservers.end(),
					  pObserver);
  if (p != m_DataLateObservers.end()) {
    m_DataLateObservers.erase(p);
  }
}
/**
 * addBarrierObserver
 *
 *  Barrier observers are called when a successful barrier has been 
 *  dispatched.  A successful barrier is one where all input queues
 *  have a barrier by the time timestamps for events following the
 *  barrier have gone past the build interval.
 *
 *  Observers are called passing a vector of pairs where each pair is
 *  a source Id and the type of barrier contributed by that source.
 *  It is really applciation dependent to know if mixed barrier types
 *  are errors, or normal.  No judgement is passed here.
 *
 * @param pObserver - Pointer to a BarrierObserver class which implements
 *                    operator()(std::vector<std::pair<uint32_t, uint32_t> >);
 */
void
CFragmentHandler::addBarrierObserver(CFragmentHandler::BarrierObserver* pObserver)
{
  m_goodBarrierObservers.push_back(pObserver);
}
/**
 * removeBarrierObserver
 *
 *  Remove a barrier observer from the set of observers that get called on a
 *  completed barrier.  It is a no-op to try to remove an observer that is not
 *  in the list.
 *
 * @param pObserver - pointer to the observer to remove.
 */
void
CFragmentHandler::removeBarrierObserver(CFragmentHandler::BarrierObserver* pObserver)
{
  std::list<BarrierObserver*>::iterator p = std::find(m_goodBarrierObservers.begin(),
						      m_goodBarrierObservers.end(), pObserver);
  if (p != m_goodBarrierObservers.end()) {
    m_goodBarrierObservers.erase(p);
  }
}
/**
 * addPartialBarrierObserver
 *
 * In the event a barrier is only partially made (some input queues are missing
 * their barrier fragments, a list of partial barrier observers is invoked.
 * This method adds a new partial barrier observer to the end of the list
 * of observers for this event.
 *
 * @param pObserver - Pointer to the observer to add.
 */
void
CFragmentHandler::addPartialBarrierObserver(CFragmentHandler::PartialBarrierObserver* pObserver)
{
  m_partialBarrierObservers.push_back(pObserver);
}
/**
 * removePartialBarrierObserver
 *
 *  Removes a partial barrier observer.
 *
 * @param pObserver - Partial barrier
 */
void
CFragmentHandler::removePartialBarrierObserver(CFragmentHandler::PartialBarrierObserver* pObserver)
{
  std::list<PartialBarrierObserver*>::iterator p = std::find(m_partialBarrierObservers.begin(),
							     m_partialBarrierObservers.end(), 
							     pObserver);
  if (p != m_partialBarrierObservers.end()) {
    m_partialBarrierObservers.erase(p);
  }
}
/**
 **   Methods for the duplicate timestamp observer.
 */

/**
 * addDuplicateTimestampObserver
 *    Add an observer for duplicate timestamps.
 *
 *  @param pObserver - the observer to add.
 */
void
CFragmentHandler::addDuplicateTimestampObserver(DuplicateTimestampObserver* pObserver)
{
    m_duplicateTimestampObservers.push_back(pObserver);
}
/**
 * removeDuplicateTimestampObserver
 *
 *    Removes an existing duplicate timestamp observer.
 *
 *  @param pObserver - The observer to remove.
 */
void
CFragmentHandler::removeDuplicateTimestampObserver(DuplicateTimestampObserver* pObserver)
{
    std::list<DuplicateTimestampObserver*>::iterator p = std::find(
        m_duplicateTimestampObservers.begin(), m_duplicateTimestampObservers.end(),
        pObserver
    );
    if (p != m_duplicateTimestampObservers.end()) {
        m_duplicateTimestampObservers.erase(p);
    }
}


/**
 * observeDuplicateTimestamp
 *    Called when a queue is outputing a non barrier fragment with the same timestamp
 *    as the last one.  All observers in the m_duplicateTimestampObservers list are invoked.
 *
 *  @param sourceId - Id of the source that is flagging this event.
 *  @param timestamp - The value of the duplicate timestamp.
 */
void
CFragmentHandler::observeDuplicateTimestamp(uint32_t sourceId, uint64_t timestamp)
{
    std::list<DuplicateTimestampObserver*>::iterator p =
        m_duplicateTimestampObservers.begin();
    while(p != m_duplicateTimestampObservers.end()) {
        DuplicateTimestampObserver* pObserver = *p;
        (*pObserver)(sourceId, timestamp);
        
        p++;
    }
}


/**
 * addFlowControlObserver
 *   Adds an observer of flow control.  These observers have Xon and Xoff methods
 *   that get called on flow control transitions.
 *
 *  @param pObserver - pointer to the observer to add.
 */
void
CFragmentHandler::addFlowControlObserver(FlowControlObserver* pObserver)
{
    m_flowControlObservers.push_back(pObserver);
}
/**
 * removeFlowControlObserver
 *   Remove an existing flow control observer from the current list of observers.
 *
 *   @param pObserver - The observer to remove.
 */
void
CFragmentHandler::removeFlowControlObserver(FlowControlObserver* pObserver)
{
    std::list<FlowControlObserver*>::iterator p =
        std::find(m_flowControlObservers.begin(), m_flowControlObservers.end(),
                  pObserver);
        
    if (p != m_flowControlObservers.end()) {
        m_flowControlObservers.erase(p);
    }
}

/**
 * addNonMonotonicTimestampObserver
 *   Add an observer that is called if an input queue has a timestamp that is
 *   non-monotonic with respect to the previous timestamp.
 *
 *  @param pObserver - pointer to the observer to add.
 */
void
CFragmentHandler::addNonMonotonicTimestampObserver(
    CFragmentHandler::NonMonotonicTimestampObserver* pObserver
)
{
    m_nonMonotonicTsObservers.push_back(pObserver);
}
/**
 * removeNonMonotonicTimestampobserver
 *   Remove an observer observer established with
 *   addNonMonotonicTimestampObserver above
 *
 *   @param pObserver - pointer to the observer to remove.
 *
 *   @note we do not own the storage for the observer, if the observer
 *   is dynamically allocated it is not our responsibility to delete it.
 */
void
CFragmentHandler::removeNonMonotonicTimestampobserver(
    CFragmentHandler::NonMonotonicTimestampObserver* pObserver
)
{
    std::list<NonMonotonicTimestampObserver*>::iterator p =
        std::find(m_nonMonotonicTsObservers.begin(),
                  m_nonMonotonicTsObservers.end(), pObserver);
    if (p != m_nonMonotonicTsObservers.end()) {
        m_nonMonotonicTsObservers.erase(p);
    }
}

/**
 * observeOutOfOrderInput
 *    Fire the observers associated with an out of order timestamp on an
 *    input queue.
 *
 *   @param sourceId - id of the source that has out of order input.
 *   @param prior    - Timestamp prior to the out of order one.
 *   @param bad      - Out of order timestamp.
 */
void
CFragmentHandler::observeOutOfOrderInput(
    unsigned sourceId, uint64_t prior, uint64_t bad
)
{
    std::list<NonMonotonicTimestampObserver*>::iterator p =
        m_nonMonotonicTsObservers.begin();
    while (p != m_nonMonotonicTsObservers.end()) {
        NonMonotonicTimestampObserver* pObs = *p;
        (*pObs)(sourceId, prior, bad);
        p++;
    }
}
/**
 * Xon
 *   Initiate a flow on event.  This means invoking the Xon method
 *   of each flow control observer registered with us.
 */
void
CFragmentHandler::Xon()
{
    std::list<FlowControlObserver*>::iterator p =
        m_flowControlObservers.begin();
    while (p != m_flowControlObservers.end()) {
        FlowControlObserver* pObserver = *p;
        pObserver->Xon();
        p++;
    }
    m_fXoffed = false;
}

/**
 * Xoff
 *   Initiates a flow off event.  This means invoking the Xoff method
 *   on each flow control observer registered with us:
 */
void
CFragmentHandler::Xoff()
{
    std::list<FlowControlObserver*>::iterator p =
        m_flowControlObservers.begin();;
        while (p != m_flowControlObservers.end()) {
            FlowControlObserver* pObserver = *p;
            pObserver->Xoff();           
            p++;
        }
        m_fXoffed = true;
}


/**
 * flush
 * 
 * There come times when it is necessary to just build event fragments
 * until there are none left. This method does that.
 *
 */
void
CFragmentHandler::flush()
{

  flushQueues(true);
  m_nNewest = 0;
  m_nOldest = UINT64_MAX;
}

/**
 * getStatistics
 *
 *   Return information about the fragment statistics.
 *   This can be used to drive a GUI that monitors the status
 *   of the software.
 *
 *   @return CFragmentHandler::InputStatistics
 *   @retval struct that has the following key fields:
 *   - s_oldestFragment - The timestamp of the oldest queued fragment.
 *   - s_newestFragment - The timestamp of the newest queued fragment.
 *   - s_totalQueuedFragments - Total number of queued fragments.
 *   - s_queueStats     - vector of individual queue statistics.
 *                        each element is a struct of type
 *                        CFragmentHandler::QueueStatistics which has the fields:
 *                        # s_queueId - Source id of the queue.
 *                        # s_queueDepth - number of fragments in the queue.
 *                        # s_oldestElement - Timestamp at head of queue.
 */
CFragmentHandler::InputStatistics
CFragmentHandler::getStatistics()
{
    InputStatistics result;
    
    // get the individual chunks:
    
    result.s_oldestFragment = m_nOldest;
    result.s_newestFragment = m_nNewest;
    
    QueueStatGetter statGetter;
    
    statGetter = for_each(m_FragmentQueues.begin(), m_FragmentQueues.end(), statGetter);
    
    result.s_totalQueuedFragments = statGetter.totalFragments();
    result.s_queueStats           = statGetter.queueStats();
    
    
    return result;
}
/**
 * createSourceQueue
 *
 *  Creates a fragment queue for a source id.  This is called to set up 
 *  an  initial set of a-priori queues in order to make initial barrier
 *  handling simpler.  If the queue already exists, this is a no-op.
 *
 * @param[in] sockName - name of socket that has this id.
 * @param[in] id  - source id of the queue.
 */
void
CFragmentHandler::createSourceQueue(std::string sockName, uint32_t id)
{

  getSourceQueue(id);		// Creates too.
  m_liveSources.insert(id);		          // Sources start live.
  m_socketSources[sockName].push_back(id);
}

/**
 * markSourceFailed
 *
 * Marks a source as failed.  This just removes it from the live sources
 * set but maintains its input queue.  If there are dead sources,
 * - Barrier synchronization can proceed without them.
 * - All barriers synchronizations are considered incomplete.
 * 
 * @note receipt of a fragment from a source automatically makes it live again. 
 *
 * @param[in] id - Id of the source to mark, as dead.
 */
void
CFragmentHandler::markSourceFailed(uint32_t id)
{
  m_liveSources.erase(id);
  
  // If there's a pending barrier synchronization and all of the missing
  // sources are dead do an incomplete barrier.
  //
  if(m_fBarrierPending) {
    if (countPresentBarriers() == m_liveSources.size()) {	
      std::cerr << "markSourceFailed -- generating barrier on source dead\n";
      std::list<std::pair<time_t, EVB::pFragment> >& sortedFragments(
          *(new std::list<std::pair<time_t, EVB::pFragment> >)
      );
      generateMalformedBarrier(sortedFragments);
      observe(sortedFragments);
    }
  }
}
/** 
 * markSocketFailed
 *
 *  Mark a socket failed.:
 * - Locate the socket in the m_socketSources map
 * - declare each source in that sockets list failed via markSourceFailed
 * - enter the source list in the deadSockets map and 
 * - Kill it from the socketSources map.
 *
 * @param sockName - socket name.
 *
 * @note See reviveSocket below.
 */
void
CFragmentHandler::markSocketFailed(std::string sockName)
{
  std::map<std::string, std::list<uint32_t> >::iterator p = m_socketSources.find(sockName);
  if (p != m_socketSources.end()) {
    std::list<uint32_t>::iterator pSource = p->second.begin();
    while (pSource != p->second.end()) {
      markSourceFailed(*pSource);
      ++pSource;
    }

    m_deadSockets[sockName] = p->second;
    m_socketSources.erase(p);
  } else {
    std::string msg = sockName;
    msg += " is not a known socket to the fragment handler";
    throw msg;
  }
}
/**
 * 
 * reviveSocket
 *
 * Called to revive the sources associated with a socket.
 * - The socket is looked up in the m_deadSockets map.
 * - The sources for it are created.
 * - The source list is moved to the socketSources map.
 *
 * @param sockName - Name of the socket being revived.
 */
void
CFragmentHandler::reviveSocket(std::string sockname)
{

  std::map<std::string, std::list<uint32_t> >::iterator p = m_deadSockets.find(sockname);
  if (p != m_deadSockets.end()) {
    std::list<uint32_t>::iterator pSource = p->second.begin();
    while (pSource != p->second.end()) {
      SourceQueue& queue = getSourceQueue(*pSource);
      m_liveSources.insert(*pSource);		     // Sources start live.
      pSource++;
    }
    m_socketSources[sockname] = p->second;
  }
  else {
    std::string msg = sockname;
    msg += " not in the dead source list";
    throw msg;
  }
}

/**
 * resetTimestamps
 *   
 * Restes the timestamps to their contruction values.
 */
void
CFragmentHandler::resetTimestamps()
{
  m_nOldest = (UINT64_MAX);
  m_nNewest = (0);
  m_nMostRecentlyPopped = (0);
  m_nFragmentsLastPeriod = (0);
  m_fBarrierPending = (false);
  m_nMostRecentlyPopped = 0;

  for (Sources::iterator p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
    p->second.reset();
  }
}
      
/**
 * clearQueues
 *
 *  Clears all knowledge of data sources. Specifically:
 *  - m_deadSockets is cleared.
 *  - m_socketSources is cleared.
 *  - m_liveSources is cleared.
 *  - m_FragmentQueues all fragments are deleted, the queues and the
 *    map itsel are cleared.
 *  - The m_fBarrierPending flag is cleared.
 *
 */
void
CFragmentHandler::clearQueues()
{
  m_deadSockets.clear();
  m_socketSources.clear();
  m_liveSources.clear();
  m_fBarrierPending = false;

  // Get rid of all queued fragments:

  for (Sources::iterator s = m_FragmentQueues.begin(); s != m_FragmentQueues.end(); s++) {
    SourceQueue& q(s->second);
    while (!q.s_queue.empty()) {
      delete q.s_queue.front().second;
      q.s_queue.pop_front();
      
    }
  }
  m_FragmentQueues.clear();
}

/*---------------------------------------------------------------------
 ** Utility methods (private).
 */

/**
 * flushQueues
 *
 *  Flush the output queues to the observers.  By default, this flushes
 *  queues until the oldest queue element is 'too new' to flush.
 *  If, however completly is true, queues are fluhsed until empty.
 *
 * @param completely - If true, queues are flushed until empty.
 *                     otherwise, the build window and m_nNow determine
 *                     when the flush stops.
 *                     Once the events are ordered into a vector, the
 *                     observers are called to deal with them
 *                     and storage associated with them deleted.
 */
void
CFragmentHandler::flushQueues(bool completely)
{
 


  // Ensure there's at least one fragment available:


  //std::vector<EVB::pFragment>& sortedFragments(*(new std::vector<EVB::pFragment>));
  
  std::list<std::pair<time_t, EVB::pFragment> >&
    sortedFragments(*(new std::list<std::pair<time_t, EVB::pFragment> >));
  
  // If there are fragments in all queues, we can dequeue up until the
  // stamp mark
  
  if (noEmptyQueue()) {
    uint64_t mark  = findStampMark();
    for (auto p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
      std::list<std::pair<time_t, EVB::pFragment> > partialSort;
      DequeueUntilStamp(partialSort, p->second.s_queue, mark);
      sortedFragments.merge(partialSort, TsCompare);
    }
  }
#if 0  
  while (noEmptyQueue() // || (m_nNow - m_nOldestReceived > m_nBuildWindow)
	  || completely ) {
    if (queuesEmpty()) {
      break;	// Done if there are no more frags.
    }
    std::pair<time_t, ::EVB::pFragment>* p = popOldest();
    if (p) {
      if (p->second->s_header.s_timestamp < m_nMostRecentlyPopped) {
        dataLate(*(p->second));        
      } else {
        m_nMostRecentlyPopped = p->second->s_header.s_timestamp;
      }
      m_nTotalFragmentSize--;
      sortedFragments.push_back(p->second);
      delete p;
    } else {
      break;		// If there are more fragments they are barriers.
    }
  }
#endif

  // Now we need to flush all fragments that are older than the build window.
  // This deals with the case of a source that just doesn't emit fragments
  // very often (e.g. scaler only case).
  // This should only be done if the most recently emptied queue was emptied outside
  // the build interval time.

  // We can only flush partial queues for enqueue times that are older
  // than the time window:
  
  
  m_nNow = time(NULL);
  time_t windowEnd = m_nNow - m_nBuildWindow;
  for (auto p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
    std::list<std::pair<time_t, EVB::pFragment> > partialSort;
    DequeueUntilAbsTime(partialSort, p->second.s_queue, windowEnd);
    sortedFragments.merge(partialSort, TsCompare);
  }   
#if 0
  findOldest();     // Previous code could have made oldest uh.. newer.
  time_t firstOldest = m_nOldestReceived;
  m_nNow = time(NULL);
  if ((m_nNow - m_nOldestReceived) >= m_nBuildWindow) {
    while (!queuesEmpty() && ((m_nNow - m_nOldestReceived) >= m_nBuildWindow) ) {
      std::pair<time_t, ::EVB::pFragment>* p = popOldest();
      findOldest();
      if (p) {
        if (p->second->s_header.s_timestamp < m_nMostRecentlyPopped) {
          dataLate(*(p->second));        
        } else {
          m_nMostRecentlyPopped = p->second->s_header.s_timestamp;
        }
        m_nTotalFragmentSize--;
        sortedFragments.push_back(p->second);
        delete p;
      } else {
        break;
      }
    }
#ifdef DEBUG
    if (queuesEmpty()) {
      std::cerr << "Empty queues\n";
    } else {
      std::cerr << "Not empty\n";
    }
#endif
  }
#endif  

  // If completely, flush all remaining fragments - this is for a planned
  // exit.
  
  if (completely) {
    for(auto q = m_FragmentQueues.begin(); q != m_FragmentQueues.end(); q++) {
      std::list<std::pair<time_t, EVB::pFragment> > partialSort(
        q->second.s_queue.begin(), q->second.s_queue.end()
      );
      q->second.s_queue.clear();              // Clear the queue.
      sortedFragments.merge(partialSort, TsCompare);
    }
  }

  // Observe the fragments we have now:
  

  observe(sortedFragments);
  
  
  // If XOFed and below the low water mark, XON:
  
  checkXon();

  
  // If a barrier is pending check it and, if the flush was complete,
  // tail call to continue building:
  
  if(m_fBarrierPending) {
    checkBarrier(completely);	// Complete forces barriers out.
  }
}
    

/**
 * popOldest
 *
 *   Remove an oldest fragment from the sources queue and update m_nOldest
 *
 *   @return ::EVB::pFragment - pointer to a fragment whose timestamp
 *           matches m_nOldest
 *   @retval - Null is returned if there are no non-barrier events in the
 *             queues.
 *
 *   @note this is all very brute force.  A much quicker algorithm to find
 *         the oldest fragment would be to retain in addtion to m_nOldest
 *         the queue that it was put in...however we still need to iterate
 *         over the queues to update m_nOldest.  This is a bit
 *         short circuited by:
 *         - Keeping track of it as we search for the first match to m_nOldest
 *         - short circuiting the loop if we find another queue with
 *           an m_nOldest match as there can't be anything older than that
 *           by definition.
 */
std::pair<time_t, ::EVB::pFragment>*
CFragmentHandler::popOldest()
{
    uint64_t nextOldest = UINT64_MAX;   // Must be older than that.
    Sources::iterator   pOldestQ;       // Iterator indicating the oldest fragment.
 
    /*
      Find the oldest fragment:
      *  Barrier fragments are immune from return.
      *  If indeed we find one, it's removed from it's queue.
      *  If indeed we find one, the oldest info is recomputed 
      */


    int i = 1;
    for (Sources::iterator p = m_FragmentQueues.begin(); 
        p != m_FragmentQueues.end(); p++) {
      if (!p->second.s_queue.empty()) {
        std::pair<time_t, ::EVB::pFragment> frag = p->second.s_queue.front();   // head of queue.
        if (frag.second->s_header.s_barrier == 0) {                             // only non-barriers.
          if (frag.second->s_header.s_timestamp < nextOldest) {                 // Oldest one yet.
            pOldestQ = p;
            nextOldest = frag.second->s_header.s_timestamp;
          } 
        } else {
          m_fBarrierPending = true; // Mark a pending barrier.
        } 
      }

      i++;
    }



    if (nextOldest != UINT64_MAX) { // Found one in pOldestQ.
      std::pair<time_t, ::EVB::pFragment> oldestFrag = pOldestQ->second.s_queue.front();
      uint64_t fragmentTimestamp  = oldestFrag.second->s_header.s_timestamp;
      
      pOldestQ->second.s_lastPoppedTimestamp = fragmentTimestamp;
      pOldestQ->second.s_bytesDeQd          += oldestFrag.second->s_header.s_size;
      pOldestQ->second.s_bytesInQ           -= oldestFrag.second->s_header.s_size;
      pOldestQ->second.s_queue.pop_front();

      // If this queue has been emptied mark that time:

      if (pOldestQ->second.s_queue.empty()) {
	m_nMostRecentlyEmptied = time(NULL);
      }
      findOldest();


      return new std::pair<time_t, ::EVB::pFragment>(oldestFrag);
    } else {			    // Either all barriers or all empty.
      return 0;
    }

    // Should not get here but...
    return 0;

}

/**
 * observe
 *
 * Invoke each observer for the event  we've been passed.
 *
 * @param event - Gather vector for the event.
 *
 * @note - the storage associated with the fragments in the vector
 *         is released after all observers have been called.
 *         If observers want to maintain fragments they will therefore need
 *         to copy them.
 */
void
CFragmentHandler::observe(std::list<std::pair<time_t, EVB::pFragment> >& event)
{
    m_outputThread.queueFragments(&event);
}
/**
 * dataLate
 * 
 *  This method is called by addFragment in the event a fragment timestamp
 *  is older than m_nNewest by more than m_nBuildWindow, indicating that
 *  the data should have been output earlier. At this time,
 *  the data has not yet been put in the data source queue.
 *  We will pass the fragment along with the value of the largest timestamp
 *  to each element of the m_DataLateObservers list.
 *
 * @param fragment - Reference to the fragment that is late.
 */
void 
CFragmentHandler::dataLate(const ::EVB::Fragment& fragment)
{
  std::list<DataLateObserver*>::iterator p = m_DataLateObservers.begin();
  while (p != m_DataLateObservers.end()) {
    DataLateObserver* pObserver = *p;
    (*pObserver)(fragment, m_nNewest);
    p++;
  }

}
/**
 * addFragment
 *
 * Add a single event fragment to the appropriate event queue.
 * The fragment is copied so that ownership of the parameter remains
 * with the caller.  allocateFragment is used to create the fragment
 * storage adnfreeFragment should be used to release that storage
 * (and that is done by buildEvent() typically).
 *
 * @param pFragment - Pointer to the flattened fragment.
 * 
 * @note This method can also alter the value of m_nNewest if its
 *       timestamp says it is the newest fragment.
 */
void
CFragmentHandler::addFragment(EVB::pFlatFragment pFragment)
{
    bool     assigned            = false;
    uint64_t priorOldest         = m_nOldest;

    m_nFragmentsLastPeriod++;	//  We were not idle.

    // Allocate the fragmentand copy it:
    
    EVB::pFragmentHeader pHeader = &pFragment->s_header;
    EVB::pFragment pFrag         = allocateFragment(pHeader); // Copies the header.
    uint64_t timestamp           = pHeader->s_timestamp;
    bool     isBarrier           = pHeader->s_barrier != 0;

    memcpy(pFrag->s_pBody, pFragment->s_body, pFrag->s_header.s_size);

    // Get a reference to the fragment queue, creating it if needed:
    
    SourceQueue& destQueue(getSourceQueue(pHeader->s_sourceId));


    // If the timestamp is null, assign the newest timestamp from that source to it:

    if ((timestamp == NULL_TIMESTAMP)) {
      assigned = true;
      timestamp = destQueue.s_newestTimestamp;
      pFrag->s_header.s_timestamp = timestamp;             // Avoid duplicate.
      assigned = true;
#ifdef DEBUG
      std::cerr << "Assigned timestamp " << std::hex << timestamp << std::dec << std::endl;
#endif
    } else {
      if ((pFrag->s_header.s_timestamp - timestamp) > uint64_t(0x100000000ll)) {
        std::cerr << "orderer: Big timestamp jump last: 0x" << std::hex
          << timestamp << " Next: 0x" << pFrag->s_header.s_timestamp << std::endl;
      }
    }
    /*
     Bug #4516 - avoid counting duplicate timestamps if the timestamp was
                 assigned because those are duplicate by design.
    
       This code makes the tacit assumption that timestamps of 0 are not likely to be
       assigned timestamps... because queue s_newestTimestamp is initialized to 0.
    */
    if ((timestamp == destQueue.s_newestTimestamp) && (!assigned)) {
      observeDuplicateTimestamp(pHeader->s_sourceId, timestamp);  
    }
    
    // Update stastistics:
    
    destQueue.s_newestTimestamp = timestamp;                  // for sure the newest 
    destQueue.s_bytesInQ       += pFrag->s_header.s_size;     // Only count payloads.
    destQueue.s_totalBytesQd   += pFrag->s_header.s_size;     // Total bytes into the queue.
    
    
    // Before we push the queue element, see if we need to observe a bad
    // timestamop:
    
    
    
    
    
    uint64_t newTimestamp   = pFrag->s_header.s_timestamp;
    uint64_t priorTimestamp =
        destQueue.s_lastTimestamp;
    
    if (newTimestamp < priorTimestamp) {
        observeOutOfOrderInput(pHeader->s_sourceId, priorTimestamp, newTimestamp);
    }

    destQueue.s_queue.push_back(std::pair<time_t, EVB::pFragment>(m_nNow, pFrag));
    destQueue.s_lastTimestamp = newTimestamp;
    
    
    m_liveSources.insert(pHeader->s_sourceId); // having a fragment makes a source live.
    
    // update newest/oldest if needed -- and not a barrier:
    // 
    // Since data can come out of order across sources it's possible
    // to update oldest as well as newest.

    // If the timing of receiving this fragment would result in an
    // out of order fragment, it's late:
    //

    
    if ((timestamp < m_nOldest)) {
	if ((m_nOldest - timestamp) > 0x100000000ll && (m_nOldest != 0xffffffffffffffff)) {
	  std::cerr << "addFragment... timestamp taking a big step back from : " << std::hex
		    << m_nOldest << " to " << timestamp << std::dec << std::endl;
	}
#ifdef DEBUG
	std::cerr << "add fragment decreasing oldest timestamp from: "
		  << std::hex << m_nOldest << " to: " 
		  << timestamp << std::dec << std::endl;
#endif
	m_nOldest = timestamp;
    }
    // Queue out of order fragment?
    if (!destQueue.s_queue.empty()) {
      if (timestamp < destQueue.s_newestTimestamp) {
	std::cerr << "Queue timestamp took a jump backwards from : "
		  << std::hex << destQueue.s_newestTimestamp
		  << " to " << timestamp << std::dec << std::endl;
      }
    }
    
    if (timestamp > m_nNewest) m_nNewest = timestamp;
#ifdef DEBUG

    if ((priorOldest != m_nOldest) && (m_nOldest == 0)) {
      std::cerr << "addFragment assigned m_nOldest -> 0 was: " 
		<< std::hex << priorOldest << std::dec << std::endl;
      dumpFragment(pFrag);
      
    }
#endif
    // Tally the fragment size and Xoff if the high water mark was hit:
    
    m_nTotalFragmentSize++;


}
/**
 * totalFragmentSize
 *
 * Computes the total size of a fragment.  This is just the size
 * of the payload + size of the header.
 *
 * @param pHeader - Pointer to an event builder fragment header.
 *
 * @return size_t
 * @retval total size of fragment descsribed by the header.
 */
size_t
CFragmentHandler::totalFragmentSize(EVB::pFragmentHeader pHeader)
{
    return pHeader->s_size + sizeof(EVB::FragmentHeader);
}

/**
 * queuesEmpty
 *   Returns true if all data source queues are empty.
 *
 *  @return bool
 *
 *  @note we don't use for_each as short of tossing an exception
 *        there's no way to short circuit the loop.
 */
bool
CFragmentHandler::queuesEmpty()
{
    for(Sources::iterator p = m_FragmentQueues.begin();
        p != m_FragmentQueues.end(); p++) {
        if (!p->second.s_queue.empty()) return false;
    }
    return true;
}
/**
 * noEmptyQueue
 *
 *  @return bool 
 *  @retval true if all queues have some data.
 *  @retval false if at least one queue is empty.
 */

bool
CFragmentHandler::noEmptyQueue()
{
    for(Sources::iterator p = m_FragmentQueues.begin();
        p != m_FragmentQueues.end(); p++) {
        if (p->second.s_queue.empty()) return false;
    }
    return true;
}
/*-----------------------------------------------------------
 ** Locally defined classers
 */
/**
 * @class QueueStateGetter
 *
 * Event source queue visitor that gathers input statistics.
 * This class is a functional and is intended to be called from
 * a for_each loop over the set of input queues.
 * It gathers the total number of fragments as well
 * as the number of fragments in the queue and the oldest fragment in the queue.
 */

/**
 * Construction sets the total fragment count to zero.
 */
CFragmentHandler::QueueStatGetter::QueueStatGetter() :
  m_nTotalFragments(0)
{}


/**
 * operator()
 *   Called for each queue to accumulate stats for that queue.
 *
 *  @param source - reference to that data source queue.
 */
void
CFragmentHandler::QueueStatGetter::operator()(SourceElementV& source)
{
    SourceQueue&  sourceQ(source.second);    
    QueueStatistics stats;
    stats.s_queueId       = source.first;
    stats.s_queueDepth    = sourceQ.s_queue.size();
    stats.s_oldestElement =
        stats.s_queueDepth ? sourceQ.s_queue.front().second->s_header.s_timestamp 
                           : 0;
    stats.s_queuedBytes       = sourceQ.s_bytesInQ;
    stats.s_dequeuedBytes     = sourceQ.s_bytesDeQd;
    stats.s_totalQueuedBytes  = sourceQ.s_totalBytesQd;
    
    m_nTotalFragments += stats.s_queueDepth;
    m_Stats.push_back(stats);
}
/**
 * totalFragments()
 * 
 * Return the total number of queued elements.
 *
 * @return uint32_t
 */
uint32_t
CFragmentHandler::QueueStatGetter::totalFragments()
{
    return m_nTotalFragments;
}
/**
 * queueStats
 *
 * Return the queue statistics vector:
 *
 * @return std::vector<QueueStatistics>
 *
 */
std::vector<CFragmentHandler::QueueStatistics>
CFragmentHandler::QueueStatGetter::queueStats()
{
    return m_Stats;
}
/**
 * generateBarrier
 *   
 *  Called to remove all barriers from the fronts of source
 *  queues and add them to the output event list.
 *  
 * @param outputList - the output list.  This is an output parameter and
 *                     we do it this way to avoid copy construction of the whole
 *                     vector.
 *
 * @return BarrierSummary
 * @retval A summary of the barrier fragment types for the sources that
 *         contributed and the sources that did not have a barrier rarin' to go.
 *
 */
CFragmentHandler::BarrierSummary
CFragmentHandler::generateBarrier(std::list<std::pair<time_t, EVB::pFragment> >& outputList)
{
  // Iterate through the output queues and add any
  // barrier events to the outputList.


  BarrierSummary result;


  
  for (Sources::iterator p = m_FragmentQueues.begin(); p!= m_FragmentQueues.end(); p++) {
    if (!p->second.s_queue.empty()) {
      std::pair<time_t, ::EVB::pFragment> front = p->second.s_queue.front();
      ::EVB::pFragment pFront = front.second;
      p->second.s_lastPoppedTimestamp = pFront->s_header.s_timestamp;
      p->second.s_bytesDeQd           += pFront->s_header.s_size;
      p->second.s_bytesInQ            -= pFront->s_header.s_size;
      if (pFront->s_header.s_barrier) {
        outputList.push_back(front);
        p->second.s_queue.pop_front();
        result.s_typesPresent.push_back(
            std::pair<uint32_t, uint32_t>(p->first, uint32_t(pFront->s_header.s_barrier))
        );
        m_nTotalFragmentSize--;
        checkXon();

      } else {

        result.s_missingSources.push_back(p->first);
      }
    } else {

      result.s_missingSources.push_back(p->first); // just as missing if the queue is empty.
    }
  }

  m_fBarrierPending = false;
  findOldest();

  return result;
  
}
/**
 * generateMalformedBarrier
 *
 *  Called when we have finished processing output events but there is an 
 *  incomplete barrier. *  this is an error...but we need to flush those
 *  fragments.
 *
 * @param outputList - Output fragment list (see above).
 */
void
CFragmentHandler::generateMalformedBarrier(std::list<std::pair<time_t, EVB::pFragment> >& outputList)
{

  BarrierSummary bs = generateBarrier(outputList);
 
  partialBarrier(bs.s_typesPresent, bs.s_missingSources);

}
/**
 * goodBarrier
 *
 *   Generate a complete barrier and fire the observers associated with them.

 * @param outputList - Output fragment list (see above).
 */
void
CFragmentHandler::goodBarrier(std::list<std::pair<time_t, EVB::pFragment> >& outputList)
{
  BarrierSummary bs = generateBarrier(outputList);

  // If there's a non empty missing sources this is a partial barrier actually.

  if (bs.s_missingSources.empty()) {
    observeGoodBarrier(bs.s_typesPresent);
  } else {

    partialBarrier(bs.s_typesPresent, bs.s_missingSources);
  }
}
/**
 * findOldest
 *
 * When a barrier (even a partial one) we may not have a correct value for
 * m_nOldest.  This method re-determines the oldest fragment by examing all
 * nonempty fragment queue's front element.  The m_nOldest is set to the timesstamp
 * in the oldest fragment or to m_nNewest if all queues are emtpy.
 */
void
CFragmentHandler::findOldest()
{

  uint64_t oldest = UINT64_MAX;		// Automatically right if all queues are empty.
  time_t   oldestReceived = (static_cast<uint64_t>(~0)) >> 1;  // All ones except sign bit in case time_t is signed
  time_t   firstOldest    = oldestReceived;


  for (Sources::iterator p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
    if (!p->second.s_queue.empty()) {
      std::pair<time_t, ::EVB::pFragment> front =  p->second.s_queue.front();
      ::EVB::pFragment pf = front.second;

      if ((pf->s_header.s_timestamp < oldest) && (pf->s_header.s_barrier ==0)) { // nonbarriers only counted
#ifdef DEBUG

	if (oldest == 0) {
	  std:: cerr << "That's zero!! \n";
	  dumpFragment(pf);
	}
#endif
	oldest = pf->s_header.s_timestamp;
      }
      if (front.first < oldestReceived) {
	oldestReceived = front.first;
      }
    }
  }
  if (oldest != UINT64_MAX) {
    m_nOldest = oldest;
  }
  if (oldestReceived != firstOldest) {
    m_nOldestReceived = oldestReceived;
  }



}
/**
 * goodBarrier
 *  
 * Fire off all of the good barrier observers.
 *
 * @param types - Vector of pairs. Each pair contains, in order, the data source ID
 *                and the barrier event type committed to output for that data source.
 */
void
CFragmentHandler::observeGoodBarrier(std::vector<std::pair<uint32_t, uint32_t> >& types)
{

  for (std::list<BarrierObserver*>::iterator p = m_goodBarrierObservers.begin();
       p != m_goodBarrierObservers.end(); p++) {
    (*p)->operator()(types);
  }

}
/**
 * partialBarrier:
 * 
 * Fire off all partial barrier observers.
 *
 * @param types - Vector of pairs as described in goodBarrier above.
 * @param missingSources - Vector of source ids that did not have a barrier.
 */
void
CFragmentHandler::partialBarrier(std::vector<std::pair<uint32_t, uint32_t> >& types, 
				 std::vector<uint32_t>& missingSources)
{

  for (std::list<PartialBarrierObserver*>::iterator p = m_partialBarrierObservers.begin();
       p != m_partialBarrierObservers.end(); p++) {
    (*p)->operator()(types, missingSources);
  }

}
/**
 * countPresentBarriers
 *
 *  Counts the number of queues with barriers that are at their heads.
 *  
 * @return size_t - number of fragment queues that have barriers at their head.
 */
size_t
CFragmentHandler::countPresentBarriers() const
{
  size_t count(0);
  for (Sources::const_iterator p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
    const SourceQueue& queue(p->second);
    if (!queue.s_queue.empty()) {
      EVB::pFragment pFront = queue.s_queue.front().second;
      if (pFront->s_header.s_barrier) count++;
    }
  }
  return count;
}
/**
 * get the source queue associated with an id, creating it if needed
 *
 * @param id - the id of the source.
 * 
 * @return SourceQueue& reference to the (possibly new) source queue.
 */
CFragmentHandler::SourceQueue&
CFragmentHandler::getSourceQueue(uint32_t id)
{
  Sources::iterator p = m_FragmentQueues.find(id);
  if (p  == m_FragmentQueues.end()) {	       // Need to create.
    SourceQueue& queue = m_FragmentQueues[id]; // Does most of the creation.
    return queue;
  }  else {			              // already exists.
    return p->second;
  }
}

/**
 * checkBarrier
 *   See if we can emit a barrier after sorted fragments have been emitted fromt he queues.
 *   If a complete flush is underway and the queues that don't have barriers at the front
 *   are empty, that constitutes an incomplete barrier even if timing says we shouild wait
 *   for more fragments. Procesing assumes that barrier processing is infrequent:
 *
 * @param completeFlush - true if a complete flush is underway.  
 */
void
CFragmentHandler::checkBarrier(bool completeFlush)
{
  std::list<std::pair<time_t, EVB::pFragment> >& outputList(
    *(new std::list<std::pair<time_t, EVB::pFragment> >)
  );
  m_nNow = time(NULL);		// Update the time.
  size_t nBarriers = countPresentBarriers();

#ifdef DEBUG
  std::cerr << "checkBarrier(): nqueues=" << m_FragmentQueues.size();
  std::cerr << " nbarriers=" << nBarriers; 
  std::cerr << std::endl; 
#endif
  


  // Check for all queues having barriers:
  if (nBarriers == m_FragmentQueues.size()) {
    goodBarrier(outputList);
    observe(outputList);
    findOldest();
    return;
  }
  // If we got here and a complete flush is requested, we must have an incomplete barrier:

  if (completeFlush) {

    generateMalformedBarrier(outputList);
    observe(outputList);
    findOldest();
    return;
  }
  // If the least recently received barrier is older than the build window than
  // m_now we also have a malformed barrier:

  if ((nBarriers != 0) && ((m_nNow - oldestBarrier()) > (m_nBuildWindow*4))) {
    std::cerr << "Generating malformed barrier oldest received: "
	      << std::hex << oldestBarrier() 
	      << " unix time(m_nNow) " << m_nNow << std::dec << std::endl;
    generateMalformedBarrier(outputList);
    observe(outputList);
  }
  // Otherwise we can wait for more fragments to come in before making any decisions.
  // 
  findOldest();			// Ensure that we know which frag is still the oldest.
}
/**
 * oldestBarrier
 *
 *  Determines which barrier was received earliest in time.
 *
 *  @return time_t
 *  @retval oldest barrier.  If there are no barriers, m_nNow is returned.
 */
time_t
CFragmentHandler::oldestBarrier()
{
  time_t result = m_nNow;

  for (Sources::iterator p = m_FragmentQueues.begin(); p != m_FragmentQueues.end();
       p++) {
    if (!p->second.s_queue.empty()) {
      std::pair<time_t, ::EVB::pFragment> Frag = p->second.s_queue.front();
      if ((Frag.first < result) && (Frag.second->s_header.s_barrier != 0)) {
	result = Frag.first;
      }
    }
  }
  return result;
}
  
/*--------------------------------------------------------------------------------------
 * Static methods
 */

/**
 * IdlePoll
 *
 * If data are not getting pushed through we periodically call flushQueues to ensure
 * the last dribs of data are sent..this also helps if data rates are low.
 *
 * method also resschedules itselfr.
 *
 * @param obj - Pointer to the singleton.
 */
void
CFragmentHandler::IdlePoll(ClientData data)
{
  CFragmentHandler* pHandler = reinterpret_cast<CFragmentHandler*>(data);

  if (!pHandler->m_nFragmentsLastPeriod) {
    pHandler->m_nNow = time(NULL);	// Update tod.
    pHandler->findOldest();		// Update oldest fragment time.
    if (pHandler->m_nNow - timeOfFirstSubmission > pHandler->m_nStartupTimeout) { 
      // Only flush after we have given time for all sources
      // establish their queues
      // Also... timeOfFirstSubmission is guaranteed to exist 
      // because nFragmentLastPeriod is not 0
      pHandler->flushQueues();
    }
  } else {
    pHandler->m_nFragmentsLastPeriod = 0;
  }
  // Since it's possible that fragments have been output from
  // the buffer queue to the output thread while we've been
  // Xoffed -- and hence can't exactly receive data, this
  // allows that to accept data again:

  pHandler->checkXon();


  // reschedule

  pHandler->m_timer = Tcl_CreateTimerHandler(1000*IdlePollInterval,  &CFragmentHandler::IdlePoll, pHandler);
}
/**
 * inflightFragmentCont
 *    @return size_t number of fragments in flight.
 */
size_t
CFragmentHandler::inFlightFragmentCount()
{
  return m_nTotalFragmentSize + m_outputThread.getInflightCount();
}
/**
 *  checkXoff
 *    If appropriate, Xoff the senders:
 */
void
CFragmentHandler::checkXoff()
{
  if ((inFlightFragmentCount() > m_nXoffLimit) && (!m_fXoffed)) {
    Xoff();
  }
}
/**
 * checkXon
 *    If appropriate, Xon the senders.
 */
  
 void
 CFragmentHandler::checkXon()
{
  if ((inFlightFragmentCount() < m_nXonLimit) && m_fXoffed) {
    Xon();
  }
}

/**
 * findStampMark
 *    Find value of the earliest timestamp at the back of the queues.
 *    It's safe to dequeue all items from all queues with timestamps that
 *    are earlier than or equal to that stamp.
 *
 * @return uint64_t   The timestamp mark past which we should not dequeue.
 */
uint64_t
CFragmentHandler::findStampMark()
{
  uint64_t result = UINT64_MAX;
  
  for(auto p = m_FragmentQueues.begin(); p != m_FragmentQueues.end(); p++) {
    if (p->second.s_lastTimestamp < result) result = p->second.s_lastTimestamp;
  }
  
  return result;
}

/**
 *  These are the dequeue operations and the predicates they need.
 *   They rely on the CopyPopUntil templated function.
 */

/**
 * predicate to determine if a queue timestamp is larger than a specific value.
 * @note to support barrier handling, a barrier fragment always returns true.
 */

class TsLargerThan {
private:
  uint64_t m_timestamp;
public:
  TsLargerThan(uint64_t ts) : m_timestamp(ts) {}
  bool operator()(std::pair<time_t, EVB::pFragment>& qel) {
    return (m_timestamp > qel.second->s_header.s_timestamp ) ||
          (qel.second->s_header.s_barrier != 0);
  }
};

/**
 * predicate to determine if the time at which a queue element was queued
 * is larger than the specified value:
 *  @note to support barrier handling, a barrier fragment always returns true.
 */

class TimeLargerThan {
private:
  time_t m_time;
public:
  TimeLargerThan(time_t t) : m_time(t) {}
  bool operator()(std::pair<time_t, EVB::pFragment>& qel) {
    return (m_time > qel.first)     ||
       (qel.second->s_header.s_barrier != 0);
  }
};

/**
 * DequeueUntilStamp
 *    Removes queue elements from the queue  and puts them in an output list
 *    for all queue elements with a timestamp leq than the provided stamp.
 *    The assumption is the queue is ordered increasing timestamp.
 *    
 *  @param result - References the list into which elements will be appended.
 *  @param q      - Queue from which elements are taken.
 *  @param timstamp - Timestamp that indicates the boundary between the two.
 */
void
CFragmentHandler::DequeueUntilStamp(
  std::list<std::pair<time_t,  EVB::pFragment> >& result,
  std::list<std::pair<time_t,  EVB::pFragment> >& q,
  uint64_t timestamp
)
{
  TsLargerThan pred(timestamp);
  CopyPopUntil(q, result, pred);
  
  // If the front of the queue is a barrier, then we have barrier in progress.
  
  m_fBarrierPending = q.front().second->s_header.s_barrier != 0;
}
/**
 * DequeueUntilAbsTime
 *   Removes queue elements from the queue and puts them in the output list
 *   for all queue elements with a queue time leq the provided time.
 *   The assumption is that the queue is ordered by increasing enqueue time.
 *
 *  @param result - References the list into which elements will be appended.
 *  @param q      - Queue from which elements are taken.
 *  @param timstamp - Timestamp that indicates the boundary between the two.
 */
void
CFragmentHandler::DequeueUntilAbsTime(
  std::list<std::pair<time_t,  EVB::pFragment> >& result,
  std::list<std::pair<time_t,  EVB::pFragment> >& q,
  time_t time
)
{
  TimeLargerThan pred(time);
  CopyPopUntil(q, result, pred);
  
  // If the front of the queue is a barrier, then we have barrier in progress.
  
  m_fBarrierPending = q.front().second->s_header.s_barrier != 0;
}