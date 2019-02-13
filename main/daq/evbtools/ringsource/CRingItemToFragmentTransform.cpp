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
#include "CRingItemToFragmentTransform.h"
#include "GetOpt.h"
#include "rfcmdline.h"

#include <CRingItem.h>
#include <DataFormat.h>
#include <CRemoteAccess.h>
#include <EVBFramework.h>
#include <CRingItemFactory.h>
#include <fragment.h>
#include <iterator>
#include <algorithm>
#include <string>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <sstream>

static uint64_t lastTimestamp(NULL_TIMESTAMP);

using namespace std;


static size_t max_event(1024*128); // initial Max bytes of events in a getData

/*----------------------------------------------------------------------
 * Canonicals
 */


/**
 * constructor:
 *  
 *  Parse and save the commandline options.
 *
 * @param argc - number of command line words.
 * @param argv - array of pointers to command line words.
 */
CRingItemToFragmentTransform::CRingItemToFragmentTransform(std::uint32_t defaultSourceId) :
  m_allowedSourceIds(1,defaultSourceId),
  m_defaultSourceId(defaultSourceId),
  m_timestamp()
{

}
/**
 * destructor
 *
 *  Free the gengetopt_args_info pointer
 */
CRingItemToFragmentTransform::~CRingItemToFragmentTransform() 
{
}

/*---------------------------------------------------------------------
 * Public interface:
 */

  ClientEventFragment
CRingItemToFragmentTransform::operator()(CRingItem* pItem, uint8_t* pDest)
{
  if (pItem == nullptr) {
    throw std::runtime_error("CRingItemToFragmentTransform::operator() passed null pointer");
  }
  RingItem*  pRingItem = pItem->getItemPointer();

  // initialize the fragment -- with the assumption that the
  // item is a non-barrier with no timestamp:

  ClientEventFragment frag;
  frag.s_timestamp = NULL_TIMESTAMP;
  frag.s_sourceId  = m_defaultSourceId;
  frag.s_size      = pRingItem->s_header.s_size;
  frag.s_barrierType = 0;
  frag.s_payload   = pDest;
  std::memcpy(pDest, pRingItem,  pRingItem->s_header.s_size);

  // Now figure what to do based on the type...default is non-timestamped, non-barrier
  // If the ring item has a timesampe we can supply it right away:

  if (pItem->hasBodyHeader()) {
    frag.s_timestamp   = pItem->getEventTimestamp();
    frag.s_sourceId    = pItem->getSourceId();
    frag.s_barrierType = pItem->getBarrierType();
  } else {

    // if we are here, then all is well in the world.
    switch (pRingItem->s_header.s_type) {
      case BEGIN_RUN:
      case END_RUN:
      case PAUSE_RUN:
      case RESUME_RUN:
        frag.s_barrierType = pRingItem->s_header.s_type;
      case PERIODIC_SCALERS:	// not a barrier but no timestamp either.
        break;
      case PHYSICS_EVENT:
        if (formatPhysicsEvent(pRingItem, pItem, frag)) {
          lastTimestamp = frag.s_timestamp;
          break;
        }
      default:
        // default is to leave things alone
        // this includes the DataFormat item

        break;
    }
  }

  validateSourceId(frag.s_sourceId);

  if (frag.s_timestamp == 0ll) {
    std::cerr << "Zero timestamp in source!?!\n";
  }

  return frag;
}

/** Handle the case of a physics event without a body header.
  * 
  * This should throw if there is not tstamp extractor provided.
  * Otherwise, if there are no bodyheaders and the tstamp
  *
  * \param item ring item C structure being accessed
  * \param p    ring item object that manages the item param
  * \param frag fragment header being filled in.
  *
  * \returns boolean whether or not the ring item was non-null
  * 
  * \throws when no tstamplib is provided and --expectbodyheaders is specified 
  *
  */
bool
CRingItemToFragmentTransform::formatPhysicsEvent (pRingItem item, CRingItem* p, ClientEventFragment& frag) 
{
  bool retval = false;

  // Check if body headers are demanded.
  if (m_expectBodyHeaders == true) {
    // this is okay if we have a tstamplib and ids from the user. Just tell them
    // that they were mistaken about there being body headers on every ring item.
    if (m_timestamp) {
      cout << "tstamp is defined " << endl;
    } else {
      cout << "tstamp is NOT defined " << endl;
    }
    if ((m_allowedSourceIds.size()>0) && m_timestamp) {
      string msg = "ringFragmentSource::getEvents() : --expectbodyheaders ";
      msg += "flag passed but observed a ring item without a BodyHeader.";
      std::cerr << msg << "\n";
    } else {
      // Oh No. This is fatal. We have no way of determining the info to stick into
      // the FragmentHeader. 
      string msg = "ringFragmentSource passed --expectbodyheaders flag but observed ";
      msg += "a ring item without a BodyHeader. This is fatal because the fragment header ";
      msg += "cannot be defined without timestamp extractor.";
      throw std::runtime_error(msg);
    }  
  }

  // kludge for now - filter out null events:
  if (item->s_header.s_size > (sizeof(RingItemHeader) + sizeof(uint32_t))) {
    frag.s_timestamp = m_timestamp(reinterpret_cast<pPhysicsEventItem>(item));
    if (((frag.s_timestamp - lastTimestamp) > 0x100000000ll)  &&
        (lastTimestamp != NULL_TIMESTAMP)) {
      unique_ptr<CRingItem> pSpecificItem(CRingItemFactory::createRingItem(*p));
      std::cerr << "Timestamp skip from "  << lastTimestamp << " to " << frag.s_timestamp << endl;
      std::cerr << "Ring item: " << pSpecificItem->toString() << endl;
    }
    
    retval = true; 
  }

  return retval;
}

void
CRingItemToFragmentTransform::validateSourceId(std::uint32_t sourceId) 
{

  if ( ! isValidSourceId(sourceId) ) {
    std::stringstream errmsg;
    errmsg << "Source id found that was not provided via the --ids option: " << sourceId;
    throw runtime_error(errmsg.str());
  }
}

bool CRingItemToFragmentTransform::isValidSourceId(std::uint32_t sourceId) 
{
 auto searchResult = std::find(m_allowedSourceIds.begin(), 
                          m_allowedSourceIds.end(),
                          sourceId);
  return (searchResult != m_allowedSourceIds.end());
}
