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


#include "CInfiniteMediator.h"

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

/**! Constructor

  Constructs the mediator object. This object owns its referenced members.

  \param source a pointer to a CDataSource
  \param filter a pointer to a CFilter
  \param sink a pointer to a CDataSink

*/
CInfiniteMediator::CInfiniteMediator(CDataSource* source, CFilter* filter, CDataSink* sink)
: CMediator(source,filter,sink)
{}

/**! Destructor
  Delete the owned objects. It is possible that these were never initialized,
  so it is important to verify that they point to something other than NULL.

  Following a delete the pointers are reset to 0. This protects against double
  frees when a user decides to call an objects destructor explicitly and then
  it goes out of scope.
*/
CInfiniteMediator::~CInfiniteMediator() 
{
}

/**! The main loop
  This is the workhorse of the application. Items are retrieved 
  from the source, passed to the filter, and then the item returned 
  by the filter is sent to the
  sink. During each iteration through this process, the item retrieved
  from the source and the filter are properly managed.
*/
void CInfiniteMediator::mainLoop()
{
  
  // Dereference our pointers before entering
  // the main loop
  CDataSource& source = *getDataSource();
  CDataSink& sink = *getDataSink();

  // Set up some counters
  int tot_iter=0, proc_iter=0, nskip=0, nprocess=0;

  nskip = getSkipCount(); 
  nprocess = getProcessCount(); 

  while (1) {
    
    // Check if all has been processed that was requested
    if (proc_iter>=nprocess && nprocess>=0) {
      break;
    }

    // Get a new item
    // Exit if the item returned is null
    CRingItem* item = source.getItem();
    if (item==0) {
      break;
    }

    // only process if we have skipped the requested number
    if (tot_iter>=nskip) {

      // handle the item and get a pointer to 
      // to the new item
      CRingItem* new_item = handleItem(item);

      // Only send an item if it is not null.
      // The user could return null to prevent sending data
      // to the sink
      if (new_item!=0) {
        // Send the new item on to the sink
        sink.putItem(*new_item);
      }
    
      // It is possible that the filter did nothing more than
      // return a pointer to the same object. Therefore, only
      // delete the returned item if it is different.
      if ( new_item != item ) {
        delete new_item;
      } 
      
      // Increment the number processed
      ++proc_iter;
    }
    

    // delete original item
    // THE FILTER MUST NOT HAVE DELETED THE OBJECT PASSED IT!!!
    delete item;

    // Increment our counter
    ++tot_iter;
  }

}

void CInfiniteMediator::initialize() 
{
  getFilter()->initialize();
}

void CInfiniteMediator::finalize() 
{
  getFilter()->finalize();
}
