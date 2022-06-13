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



//! \class: CBuildChannelData           
//! \file:  .h
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 
/*!
Collects the data from each channel 
in the vector m_ChannelData. Each 
vector element is a pair<string,ChannelData>>
where the first element is the channel name and the
second a struct containing:
- m_sValue - The string value of the channel.
- m_sUnits - The string units of the channel or string("") if the channel has
             no units field.
- m_Updated- a time_t describing the last time the channel had a successful
             update.

 The assumption is that the channels being visited
 are all CUintChannel objects.
*/
#ifndef CBUILDCHANNELDATA_H  //Required for current class
#define CBUILDCHANNELDATA_H

//
// Include files:
//

#include "CChannelVisitor.h"
#include <string>
#include <list>
#include <time.h>

// Forward class definitions:
class CChannel;

class CBuildChannelData  : public CChannelVisitor        
{

public:

  // Data type definitions.

  typedef struct _ChannelData {
    std::string m_sValue;
    std::string m_sUnits;
    time_t m_Updated;
    struct _ChannelData& operator=(const struct _ChannelData& rhs) {
      m_sValue = rhs.m_sValue;
      m_sUnits = rhs.m_sUnits;
      m_Updated= rhs.m_Updated;
      return *this;
    }                           //!< Ensure strings are copied properly, not just bitwise.
  } ChannelData;		//!< Information about a channel.

  typedef std::pair<std::string, ChannelData> ChannelItem;	//!< Info about a chan.
  typedef std::list<ChannelItem>         ChannelItems;	//!< List of channel info.
  typedef ChannelItems::iterator    ChannelItemIterator; //!< Iterator to list.
private:
  // Member data

  ChannelItems  m_ChannelData;	//!< Data about each channel..
   
  // Constructors and other cannonical operations:

public:
  CBuildChannelData ();		//!< object construction
  ~ CBuildChannelData ( );	//!< object destruction.
  CBuildChannelData(const CBuildChannelData& that); //!< Copy construction.
  CBuildChannelData& operator=(const CBuildChannelData& rhs); //!< Assignment

  // Class operations:

public:

  virtual   void operator() (CChannel* pChannel); //!< per channel operation
  ChannelItemIterator begin ()   ;                       //!< Return begin iterator.
  ChannelItemIterator end ()   ;                         //!< Return end iterator.
  int          size ()   ;                        //!< Return number of items in the list.
  
};

#endif
