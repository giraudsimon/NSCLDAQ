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



//! \class: CChannelList           
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef CCHANNELLIST_H  //Required for current class
#define CCHANNELLIST_H

//
// Include files:
//

#include <list>        //Required for include files  

// Forward channel definitions.

class CChannel;
class CChannelVisitor;
 
/*!
This class maintains and updates a liist
of channels.   Channels are stored as
an ordered list (list<CChannel*>).  The main
functions of this class are:
- Managing the channel list.
- Managing the update of channels
  in the list. 
Updates are attempted as
a sychronous group.  However if that
fails, the channels are individually updated
in order to determine which ones replied
and which ones didn't respond.
*/
class CChannelList      
{
public:
  // Data types:

  typedef std::list<CChannel*> ChannelPointers;
  typedef ChannelPointers::iterator ChannelIterator;

private:
  ChannelPointers m_Channels;

public:
  
  CChannelList ();		//!< Constructs the list.
  ~ CChannelList ( );  		//!< Destroys the list.

  // We are not interestedi nthe channel list being copied
  // compared or otherwise doing things that imply more than
  // one copy of the list.

private:
  CChannelList(const CChannelList& rhs);
  CChannelList& operator=(const CChannelList& rhs);
  int operator==(CChannelList& rhs) const;
  int operator!=(CChannelList& rhs) const;
public:

  // Class operations:

public:

  ChannelIterator begin ()   ;              //!< Begin iterator for list. 
  ChannelIterator end ()   ;                //!< End iterator for list. 
  int size ()   ;                           //!< size of list. 
  void foreach (CChannelVisitor& rVisitor); //!< Visit list. 
  void push_back (CChannel& rChannel)   ;   //!< Add to list.
  
};

#endif
