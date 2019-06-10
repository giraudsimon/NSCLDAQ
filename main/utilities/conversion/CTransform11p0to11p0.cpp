/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#include <CTransform11p0to11p0.h>
#include <V11/CRingItemFactory.h>
#include <V11/CRingItem.h>
#include <CFilter.h>

#include <iostream>
using namespace std;

namespace DAQ {
  namespace Transform {

    //
CTransform11p0to11p0::CTransform11p0to11p0(unique_ptr<CFilter> pFilter)
  : m_pFilter(move(pFilter))
{}

CTransform11p0to11p0::FinalType 
CTransform11p0to11p0::operator()(InitialType& item)
{
  InitialType* pItem = V11::CRingItemFactory::createRingItem(item);

  return dispatch(pItem);
}

CTransform11p0to11p0::FinalType 
CTransform11p0to11p0::dispatch(InitialType* pItem)
{
    using namespace V11;

    // initial pointer to filtered item
  InitialType* fitem = pItem;

  switch(pItem->type()) {

    // State change items
    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
      fitem = m_pFilter->handleStateChangeItem(static_cast<CRingStateChangeItem*>(pItem));
      break;

      // Documentation items
    case PACKET_TYPES:
    case MONITORED_VARIABLES:
      fitem = m_pFilter->handleTextItem(static_cast<CRingTextItem*>(pItem));
      break;

      // Scaler items
    case PERIODIC_SCALERS:
      fitem = m_pFilter->handleScalerItem(static_cast<CRingScalerItem*>(pItem));
      break;

      // Physics event item
    case PHYSICS_EVENT:
      fitem = m_pFilter->handlePhysicsEventItem(static_cast<CPhysicsEventItem*>(pItem));
      break;

      // Physics event count
    case PHYSICS_EVENT_COUNT:
      fitem = m_pFilter->handlePhysicsEventCountItem(static_cast<CRingPhysicsEventCountItem*>(pItem));
      break;

      // Event builder fragment handlers
    case EVB_FRAGMENT:
    case EVB_UNKNOWN_PAYLOAD:
      fitem = m_pFilter->handleFragmentItem(static_cast<CRingFragmentItem*>(pItem));
      break;

      // Handle any other generic ring item...this can be 
      // the hook for handling user-defined items
    default:
      fitem = m_pFilter->handleRingItem(pItem);
      break;
  }

  return CRingItem(*fitem);
}

  } // end Transform
} // end DAQ
