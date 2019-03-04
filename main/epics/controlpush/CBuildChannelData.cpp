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



/*! \file CBuildChannelData.cpp
  Implementation of the CBuildChannelData class.  See the class header
  file for more information about what's in this file.
*/

// Headers:
#include <config.h>
#include "CBuildChannelData.h"    				
#include "CUnitChannel.h"


using namespace std;



/*!
  The constructor creates a new visitor with an empty channel data list.
  A new object should be constructed for each gathering of the data in 
  an update pass.
*/
CBuildChannelData::CBuildChannelData ()
{   
    
} 
/*!
   Called on object exit.  Since we are using some nice data types,
   we don't need to empty the list.
*/
 CBuildChannelData::~CBuildChannelData ( )
{
}

/*!  
  Copy constrution is used to create e.g. temporaries, from the contents
  of a pre-existing object.
  This is necessary because the list copy construction must be done rather
  than a bitwise copy of member data:
  \param aCBuildChannelData (const CBuildChannelData&)
     Reference to the existing object from which construction is templated.
*/ 
CBuildChannelData::CBuildChannelData (const CBuildChannelData& aCBuildChannelData ) 
  : CChannelVisitor (aCBuildChannelData) ,
    m_ChannelData(aCBuildChannelData.m_ChannelData)
{
  
} 
/*!
  Assignment,  *this is assigned to from rhs.   Again,we want the list
  operator= to run in order to be sure that there is not going to be just
  a bit by bit assignment.
  \param rhs (const CBuildChannelData&):
      Reference to the object that we will be assigned from.
  \return CBuildChannelData&
    reference to *this.

  \note  We assume that the list assignment operator properly destroys any
         existing list storage.
*/
CBuildChannelData& 
CBuildChannelData::operator= (const CBuildChannelData& rhs)
{

  if(this != &rhs) {
    CChannelVisitor::operator=(rhs); // Base class...
    m_ChannelData = rhs.m_ChannelData;
  }

  return *this;
}



// Functions for class CBuildChannelData

/*!

Visits a channel and extracts its
current information:

The information is extracted into a 
pair<string, ChannelData> where the string is just the channel name and:
- ChannelData.m_sValue - Is the most recent sucessfully retrieved channel value.
- ChannelData.m_sUnits - is "" if no units exist or the units value.
- ChannelData.m_Updated- is the last time the channel was updated.
*/
void 
CBuildChannelData::operator()(CChannel* pChannel)  
{ 
  CUnitChannel* pC = dynamic_cast<CUnitChannel*>(pChannel);

  ChannelData Info;		// Data about the channel.

  Info.m_sValue   = pC->getValue();
  Info.m_sUnits   = pC->getUnits();
  Info.m_Updated = pC->getLastUpdate();

  m_ChannelData.push_back(ChannelItem(pC->getName(), Info));

}  

/*!  Function: 	
   ChannelItemIterator begin() 
 Operation Type:
    
Purpose: 	

Returns an interator into the vector
of channel data that is a begin iterator.

*/
CBuildChannelData::ChannelItemIterator 
CBuildChannelData::begin()  
{ 
  return m_ChannelData.begin();
}  

/*!  Function: 	
   ChannelItemIterator end() 
 Operation Type:
    
Purpose: 	

Returns an end item iterator for the channel data
items.

*/
CBuildChannelData::ChannelItemIterator 
CBuildChannelData::end()  
{ 
  return m_ChannelData.end();
}  

/*!  Function: 	
   int size() 
 Operation Type:
    
Purpose: 	

Returns the number of channel data
items.

*/
int 
CBuildChannelData::size()  
{
  return m_ChannelData.size();
}
