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
#ifndef CSIS3300CREATOR_H
#define CSIS3300CREATOR_H

#include <CModuleCreator.h>
#include <stdint.h>

/*!
   recognizer/creator within the scriptable readout framework
   for SIS 3300/3301 flash adc modules.
*/

class CSIS3300Creator : public CModuleCreator
{
public:
  CSIS3300Creator();
  virtual ~CSIS3300Creator();

  virtual CReadableObject*  Create(const char* name, CTCLInterpreter& rInterp);

};


#endif
