#ifndef CV1X90CREATOR_H
#define CV1X90CREATOR_H

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

#include "CModuleCreator.h"


/*!
   Creator for instances of CV1x90Module objects on behalf of the module object factory.

*/
class CV1x90Creator : public CModuleCreator
{
  // Constructors and canonicals.
public:
  CV1x90Creator();
  virtual ~CV1x90Creator();
  
  virtual CReadableObject* Create(const char* name, CTCLInterpreter& rInterp);
	 
};

#endif
