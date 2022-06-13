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

#ifndef CLOOKUPVISITOR_H  //Required for current class
#define CLOOKUPVISITOR_H

//
// Include files:
//

#include "CChannelVisitor.h"
 
/*!

*/
class CLookupVisitor  : public CChannelVisitor        
{
private:


public:
  CLookupVisitor();
  void operator() (CChannel* pChannel)   ; //!< Per channel action.

};

#endif
