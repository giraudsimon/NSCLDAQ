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
#include "CRingSource.h"
#include "GetOpt.h"
#include "rfcmdline.h"

#include <CRingBuffer.h>
#include <CRingItem.h>
#include <DataFormat.h>
#include <CRemoteAccess.h>
#include <EVBFramework.h>
#include <CAllButPredicate.h>
#include <CRingItemFactory.h>
#include <fragment.h>
#include <string.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <io.h>
#include <fstream>
#include <iostream>

#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <cassert>

static std::ostream& logfile(std::cerr);
static uint64_t lastTimestamp(NULL_TIMESTAMP);



static size_t max_event(1024*1024*10); // initial Max bytes of events in a getData

/*----------------------------------------------------------------------
 * Canonicals
 */

CRingSource::CRingSource(CRingBuffer* pBuffer, 
    const std::vector<uint32_t>& allowedIds, 
    uint32_t defaultId, 
    tsExtractor extractor)
  : m_pArgs(nullptr),
  m_pBuffer(pBuffer),
  m_allowedSourceIds(allowedIds),
  m_defaultSourceId(defaultId),
  m_timestamp(extractor),
  m_stall(false),
  m_stallCount(0),
  m_expectBodyHeaders(false),
  m_fOneshot(false),
  m_nEndRuns(1),
  m_nEndsSeen(0),
  m_nTimeout(0),
  m_nTimeWaited(0),
    m_wrapper(0),
    m_myRing(false)
{
  m_wrapper.setTimestampExtractor(m_timestamp);
  m_wrapper.setAllowedSourceIds(m_allowedSourceIds);
  m_wrapper.setDefaultSourceId(m_defaultSourceId);
  m_wrapper.setExpectBodyHeaders(m_expectBodyHeaders);
}

/**
 * constructor:
 *  
 *  Parse and save the commandline options.
 *
 * @param argc - number of command line words.
 * @param argv - array of pointers to command line words.
 */
CRingSource::CRingSource(int argc, char** argv) :
  m_pArgs(0),
  m_pBuffer(0),
  m_timestamp(0),
  m_nEndRuns(1),
  m_nEndsSeen(0),
  m_nTimeout(0),
  m_nTimeWaited(0),
  m_wrapper(0)
{
  GetOpt parsed(argc, argv);
  m_pArgs = new gengetopt_args_info;
  memcpy(m_pArgs, parsed.getArgs(), sizeof(gengetopt_args_info));

  if (m_pArgs->oneshot_given) {
    m_fOneshot  = true;
    m_nEndRuns = m_pArgs->oneshot_arg;
  } else {
    m_fOneshot = false;
  }
  m_nTimeout = m_pArgs->timeout_arg * 1000;        // End run timeouts in ms.
  m_nTimeOffset = m_pArgs->offset_arg;             // tick time offset.
  
}
/**
 * destructor
 *
 *  Free the gengetopt_args_info pointer
 */
CRingSource::~CRingSource() 
{
  delete m_pArgs;
  if (m_myRing)delete m_pBuffer;		// Just in case we're destructed w/o shutdown.
}

/*---------------------------------------------------------------------
 * Public interface:
 */


/**
 * initialize
 *
 *  One time initialization what we need to do is:
 *  - get the URL that is the ring data source and make a consumer attachment.
 *  - Get our source id
 *  - Get a pointer to the timestamp extraction function.
 *
 * @throw std::string in the event of an error with the reason for the error
 *        as the string value.
 */
void
CRingSource::initialize()
{
  std::string url = m_pArgs->ring_arg;
  
  // Process the source id and body headers flags:

  if (m_pArgs->ids_given==0 && !m_pArgs->expectbodyheaders_given) {
    throw std::string("The source id (--ids) is required for this source!");
  } 

  if (m_pArgs->ids_given > 0) {
    m_allowedSourceIds.insert(m_allowedSourceIds.end(),
			      m_pArgs->ids_arg, 
			      m_pArgs->ids_arg + m_pArgs->ids_given);
  } else if (m_pArgs->ids_given==0) {
    if (!m_pArgs->expectbodyheaders_given) {
      throw std::string("The list of source ids (--ids) are required for this source!");
    }
  }
  if (m_pArgs->default_id_given) {
    m_defaultSourceId = m_pArgs->default_id_arg;
  } else {
    if (m_pArgs->ids_given > 0) {
      m_defaultSourceId = m_pArgs->ids_arg[0];
    } else {
      // we should not get here but it is worth adding the check in case gengetopt gets
      // changed.
      throw std::string("Cannot set a default source id! Neither --default-id nor --ids option specified.");
    }
  }
  


  if (m_pArgs->timestampextractor_given) {
    std::string dlName = m_pArgs->timestampextractor_arg;
    // note that in order to allow the .so to be rebuilt while we're running without
    // us potentially dying, the .so will be copied to a temporary file.
    // Once the image is mapped it is unlinked so that it vanishes once the source exits.

    dlName = copyLib(dlName);

    // Load the DLL and look up the timestamp function (putting i in m_timestamp;
    // we never do a dlclose so the DLL remains loaded in all OS's.

    void* pDLL = dlopen(dlName.c_str(), RTLD_NOW);
    if (!pDLL) {
      int e = errno;
      std::string msg = "Failed to load shared lib ";
      msg += dlName;
      msg += " ";
      msg += strerror(e);
      throw msg;
    }
    m_timestamp = reinterpret_cast<tsExtractor>(dlsym(pDLL, "timestamp"));
    if (!m_timestamp) {
      int e errno;
      std::string msg = "Failed to locate timestamp function in ";
      msg += dlName;
      msg += " ";
      msg += strerror(e);
      m_wrapper.setTimestampExtractor(m_timestamp);

      throw msg;
    }
    unlink(dlName.c_str());	// Marks this for destruction.
  } else {
    // the tstamplib is not provided. Has the expectbodyheaders flag
    // been provided? If not, this is not allowed and the program
    // be stopped (or the issue addressed somehow).
    if (!m_pArgs->expectbodyheaders_given) {
      std::string msg = "This source may have ring items with insufficient data to ";
      msg += "create fragments. Either specify --expectbodyheaders or set the ";
      msg += "--timestampextractor and -ids options.";
      throw msg;
    }

  }
 
  m_wrapper.setAllowedSourceIds(m_allowedSourceIds);
  m_wrapper.setDefaultSourceId(m_defaultSourceId);
  m_wrapper.setExpectBodyHeaders(m_pArgs->expectbodyheaders_flag);
 
  logfile << std::hex;

  // Attach the ring.

  if (m_pBuffer && m_myRing) {
    delete m_pBuffer;                // Don't leak rings
  }
  m_pBuffer = CRingAccess::daqConsumeFrom(url);
  m_myRing = true;
  
}
/**
 * dataReady
 *
 *   Waits until there is data in the ring for at most the specified number
 *   of ms.
 *
 * @param ms - Number of milliseconds to block.
 *
 * @return bool - true if there was data after the time.
 */
bool
CRingSource::dataReady(int ms)
{
  struct timespec initial;
  struct timespec now;

  clock_gettime(CLOCK_MONOTONIC, &initial);

  do {
    if (m_pBuffer->availableData()) {
      m_nTimeWaited = 0;
      return true;
    }
    m_pBuffer->pollblock();	// block a while.

    clock_gettime(CLOCK_MONOTONIC, &now);
  } while(timedifMs(now, initial) < ms);
  m_nTimeWaited += ms;
  if (m_fOneshot && (m_nEndsSeen > 0) && (m_nTimeWaited > m_nTimeout)) {
    std::cerr << "End run timeout expired exiting\n";
    exit(EXIT_FAILURE);
  }
  
  return false;			// timed out.
}
/**
 * getEvents
 *
 *  Takes data from the ring buffer and builds event fragment lists.
 *  - the event source comes from m_allowedSourceIds.
 *  - scaler and trigger count events become untimestamped fragments.
 *  - State transition events become barriers whose type is the same as
 *    the type in their ring type
 *  - The payload of each fragment is the entire ring item (header and all).
 *
 *
 */
void
CRingSource::getEvents()
{
  m_frags.clear(); // start fresh

  uint8_t* pBuffer = reinterpret_cast<uint8_t*>(malloc(max_event*2));
  // transforms avail data to fragments and adds to m_frags
  transformAvailableData(pBuffer);
  
  // Send those fragments to the event builder:

  if (m_frags.size()) {
    CEVBClientFramework::submitFragmentList(m_frags);
  }

  if (oneshotComplete()) {
    exit(EXIT_SUCCESS);
  }

  delete [] pBuffer;
}

void CRingSource::transformAvailableData(uint8_t*& pFragments)
{
  size_t bytesPackaged(0);
  CAllButPredicate all;		// Predicate to selecdt all ring items.
  uint8_t*         pDest = pFragments;
  if (pFragments == 0) {
    throw std::string("CRingSource::getEvents - memory allocation failed");
  }

  while ((bytesPackaged < max_event) && m_pBuffer->availableData()) {
    std::unique_ptr<CRingItem> p(CRingItem::getFromRing(*m_pBuffer, all)); // should not block.
    RingItem*  pRingItem = p->getItemPointer();

    // check for end runs for oneshot logic
    if (pRingItem->s_header.s_type == END_RUN) {
      m_nEndsSeen++;
    }

    // If we got here but the data is bigger than our safety margin
    //we need to resize pFragments:

    if (pRingItem->s_header.s_size > (max_event*2 - bytesPackaged)) {
      size_t offset = pDest - pFragments; // pFragments willchange.
      max_event = pRingItem->s_header.s_size + bytesPackaged;
      pFragments = reinterpret_cast<uint8_t*>(realloc(pFragments, max_event*2));
      pDest      = pFragments + offset;

    }

    ClientEventFragment frag = m_wrapper(p.get(), pDest);
    frag.s_timestamp += m_nTimeOffset;
    pDest += frag.s_size;
    bytesPackaged += frag.s_size;

    m_frags.push_back(frag);

  }
}

bool CRingSource::oneshotComplete()
{
     return (m_fOneshot && (m_nEndsSeen >= m_nEndRuns));
}


/**
 * shutdown 
 *
 * Shuts the data sourcd down.  For us that's just killing off the
 * m_pBuffer.
 */
void
CRingSource::shutdown()
{
  delete m_pBuffer;
  m_pBuffer = 0;
}


/*----------------------------------------------------------------------
** Private utilities:
*/

/**
 * timedifMs 
 *
 * Returns the difference between two times as timespec structs in milliseconds.
 *
 * @param later - The 'later time.'
 * @param earlier - The 'earlier time'.  This is subtracted from later to give the answer.
 *
 * @return uint64_t
 * @retval (later - earlier) in millisecond units.
 *
 * @throw - negative time differences throw an std::string (BUG).
 */
uint64_t
CRingSource::timedifMs(struct timespec& later, struct timespec& earlier)
{
  struct timeval l = {later.tv_sec, later.tv_nsec/1000};
  struct timeval e = {earlier.tv_sec, earlier.tv_nsec/1000};
  struct timeval res;
  timersub(&l, &e, &res);
  
  
  uint64_t result = res.tv_sec * 1000;
  result         += res.tv_usec/1000;
  
  
  return result;
  

}
/**
 * copyLib
 *
 *  In order to allow the shared library that is the timestamp extractor to be
 * altered as we run (e.g. via make), it is copied to a temporary file and
 * that tempfile is what's mapped.
 * There are some timing holes we need to accept here:
 *  - Between the time the temp name is created and we actually create the file 
 *    someon else could duplicate the filename and we'll smash that file.
 *  - Between the time we copy the .so into the tempfile and actually map it
 *    someone else could come along and smash the copied so.
 *  - Between the time we create the file and finish copying it someone could be
 *    altering our source file.
 *
 * Life's truly a bitch if you think too hard about it.
 *
 * @param original - the name of the shared object to copy.
 *
 * @return std::string - name of the copied file.
 *
 * @throw std::string on any error.
 */
std::string
CRingSource::copyLib(std::string original)
{
  std::string destName;
  int dest;

  // First try to open the original.  If that's not possible throw up.

  int from = open(original.c_str(), O_RDONLY);
  if (from < 0) {
    int err = errno;   // In case string ops smash errno.
    std::string msg("CRingSource: Time extractor shared library: ");
    msg += original;
    msg += " cannot be opened: ";
    msg += strerror(err);
    throw msg;
  }

  // All of the try/catch things here are intended to be sure that any open 
  // fds also get closed.  This prevents fd leakage in the unlikely event
  // the caller tries to continue after failure.

  try {
    // Make the temp name:
    
    char* pDestName = tmpnam(NULL);
    if (!pDestName) {
      int err = errno;
      std::string msg("CRingSource: Failed to create temporary shared library filename: ");
      msg += strerror(err);
      throw msg;
    }
    destName = pDestName;

    
    // Open the dest.
    
    dest = open(destName.c_str(), O_CREAT | O_WRONLY, S_IRWXU);
    if (!dest) {
      int err;
      std::string msg = "CRingSource: Faile to create a temp file for the shared  library: ";
      msg += strerror(err);
      throw msg;
    }
    
    // Copy the file.
    try {
      char buffer[8192];	// Or some such suitably large block  of storage.

      while (1) {

	// For reads we just get what we can but for writes we ensure the write is total.
	
	ssize_t nRead = read(from, buffer, sizeof(buffer));
	if (nRead == 0) break;

	io::writeData(dest, buffer, nRead);

      }
	  
    }
    catch(...) {
      close(dest);
      throw;
    }
    // Return the result if we survived all of this:
  }
  catch(...) {
    close(from);
    throw;
  }
  close(from);
  close(dest);
  return destName;
}

