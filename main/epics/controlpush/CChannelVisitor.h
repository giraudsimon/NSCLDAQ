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


// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef CCHANNELVISITOR_H  //Required for current class
#define CCHANNELVISITOR_H

// Forward definitions:

class CChannel;

/*!
  ABC for a channel visitor.
Channel visitors are the parameters to
CChannelList::foreach.  A channel visitor is
a function object that implements an
operator()(CChannel*).

*/
class CChannelVisitor      
{
  // Class operations:
  
  // Need to virtualize the destuctor.

public:
  virtual ~CChannelVisitor() {}
  
public:
  
  virtual void operator() (CChannel* pChannel)   = 0 ; //!< Per channel action.
  
};

#endif
