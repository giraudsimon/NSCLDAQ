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

#include <config.h>
#include "CLookupVisitor.h"    
#include "CChannel.h"

#include <iostream>


using namespace std;

				

// Functions for class CLookupVisitor

/*!
   Construct a lookup visitor... just salt away the timeout.

*/
CLookupVisitor::CLookupVisitor()
{
}


/*! 
  Called for each channel in the collection.
  - If the channel is not connected a Connect is done on it.
  - The caller must at some pont invoke CChannel::doEvents
 to allow Epics to dispatch event handler.s

*/
void 
CLookupVisitor::operator()(CChannel* pChannel)  
{ 
  if(!pChannel->isConnected()) {
    pChannel->Connect();
  }

}
