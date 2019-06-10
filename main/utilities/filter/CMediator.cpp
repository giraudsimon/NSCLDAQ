/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";


#include "CMediator.h"

#include <iostream>
#include <CDataSource.h>
#include <CDataSink.h>
#include <CFilter.h>

#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CPhysicsEventItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingFragmentItem.h>

using namespace std;

/**! Constructor

  Constructs the mediator object. This object owns its referenced members.

  \param source a pointer to a CDataSource
  \param filter a pointer to a CFilter
  \param sink a pointer to a CDataSink

*/
CMediator::CMediator(CDataSource* source, CFilter* filter, CDataSink* sink)
: CBaseMediator(unique_ptr<CDataSource>(source), unique_ptr<CDataSink>(sink)), 
  m_pFilter(unique_ptr<CFilter>(filter)),
  m_nToProcess(-1),
  m_nToSkip(-1)
{}
CMediator::CMediator(unique_ptr<CDataSource> source, unique_ptr<CFilter> filter, unique_ptr<CDataSink> sink)
: CBaseMediator(move(source), move(sink)),
  m_pFilter(move(filter)),
  m_nToProcess(-1),
  m_nToSkip(-1)
{}

/**! Destructor
  Delete the owned objects. It is possible that these were never initialized,
  so it is important to verify that they point to something other than NULL.

  Following a delete the pointers are reset to 0. This protects against double
  frees when a user decides to call an objects destructor explicitly and then
  it goes out of scope.
*/
CMediator::~CMediator() 
{

  m_nToProcess = -1;
  m_nToSkip = -1;
}

CRingItem* CMediator::handleItem(CRingItem* item)
{
  // initial pointer to filtered item
  CRingItem* fitem = item;
  switch(item->type()) {

    // State change items
    case BEGIN_RUN:
    case END_RUN:
    case PAUSE_RUN:
    case RESUME_RUN:
      fitem = m_pFilter->handleStateChangeItem(static_cast<CRingStateChangeItem*>(item));
      break;

      // Documentation items
    case PACKET_TYPES:
    case MONITORED_VARIABLES:
      fitem = m_pFilter->handleTextItem(static_cast<CRingTextItem*>(item));
      break;

      // Scaler items
    case PERIODIC_SCALERS:
      fitem = m_pFilter->handleScalerItem(static_cast<CRingScalerItem*>(item));
      break;

      // Physics event item
    case PHYSICS_EVENT:
      fitem = m_pFilter->handlePhysicsEventItem(static_cast<CPhysicsEventItem*>(item));
      break;

      // Physics event count
    case PHYSICS_EVENT_COUNT:
      fitem = m_pFilter->handlePhysicsEventCountItem(static_cast<CRingPhysicsEventCountItem*>(item));
      break;

      // Event builder fragment handlers
    case EVB_FRAGMENT:
    case EVB_UNKNOWN_PAYLOAD:
      fitem = m_pFilter->handleFragmentItem(static_cast<CRingFragmentItem*>(item));
      break;

      // Handle any other generic ring item...this can be 
      // the hook for handling user-defined items
    default:
      fitem = m_pFilter->handleRingItem(item);
      break;
  }

  return fitem;
}


