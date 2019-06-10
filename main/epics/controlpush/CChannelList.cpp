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



////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////


#include <config.h>
#include "CChannelList.h"    				
#include "CChannel.h"
#include "CChannelVisitor.h"

#include <algorithm>

#include <cadef.h>		// EPICS



using namespace std;


// Note epics defines assert for now.


/*!
   Constructs a channel list.  The channel list doesn't need anything special for
   construction.  
*/
CChannelList::CChannelList ()
 
{   
} 
/*!
  The destructor for a channel list must destroy all channels.
  Channels are assumed to be dynamically allocated.  This reduces
  the re-usability of this somewhat.  
  \bug Later maybe make the push_back copy construct the channel
         and then we know for sure the channel is dynamically created.
         for now we want to get a working piece of software.
*/
 CChannelList::~CChannelList ( )  //Destructor - Delete dynamic objects
{

  // Destroy all the channels in the list... incidently this also empties
  // the list as well.

  while(size() != 0) {
    CChannel* pChannel = m_Channels.front();
    delete    pChannel;	
    m_Channels.pop_front();
  }
}


// Functions for class CChannelList


/*!

Returns an iterator to the begin point of the channel list.

*/
CChannelList::ChannelIterator 
CChannelList::begin()  
{ 
  return m_Channels.begin();
}  

/*!  

Returns an end iterator to the channel list.


*/
CChannelList::ChannelIterator 
CChannelList::end()  
{
  return m_Channels.end();
}  

/*!  

Returns the number of elements in the channel list.

*/
int 
CChannelList::size()  
{
  return m_Channels.size();
}  

/*!  


Dispatches a CChannelVisitor to each elelment 
of the channel list.  The CHannelVisitor's
operator() will be called with a pointer to each of the
channels in the list.
\param rVisitor (CChannelVisitor&):
   Visitor object that will visit each node in the channel list.


*/
void 
CChannelList::foreach(CChannelVisitor& rVisitor)  
{
  for(ChannelIterator i = begin(); i != end(); i++) {
    CChannel* p = *i;
    rVisitor(p);
  }
}  

/*!  Function: 	
   void push_back(CChannel& rChannel) 
 Operation Type:
    
Purpose: 	

Adds a pointer to a channel to the back of the channel list.
\pram rChannel CChannel
  Reference to the channel to add to the channel list.
  The channel is assuemd to be dynmically allocated.
*/
void 
CChannelList::push_back(CChannel& rChannel)  
{
  m_Channels.push_back(&rChannel);
}
