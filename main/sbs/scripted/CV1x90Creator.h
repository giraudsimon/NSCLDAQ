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
  CV1x90Creator(std::string type);
  virtual ~CV1x90Creator();
  CV1x90Creator(const CV1x90Creator& rhs);
  CV1x90Creator& operator=(const CV1x90Creator& rhs);
  int operator==(const CV1x90Creator& rhs) const;
  int operator!=(const CV1x90Creator& rhs) const;

  // The virtual interface defined by CModuleCreator:

  virtual CReadableObject*
  Create(CTCLInterpreter& rInterp,
	 CTCLResult&      rResult,
	 int              nArgs,
	 char**           pArgs);

  virtual std::string Help();
    
};

#endif
