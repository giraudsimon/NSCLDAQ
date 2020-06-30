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

/*!
   recognizer/creator within the scriptable readout framework
   for SIS 3300/3301 flash adc modules.
*/

class CSIS3300Creator : public CModuleCreator
{
public:
  CSIS3300Creator(const std::string& type = std::string("sis3300"));
  virtual ~CSIS3300Creator();

  CSIS3300Creator(const CSIS3300Creator& rhs);
  CSIS3300Creator& operator=(const CSIS3300Creator& rhs);
  int operator==(const CSIS3300Creator& rhs) const;
  int operator!=(const CSIS3300Creator& rhs) const;

  // Base class overrrides:
public:
  virtual CReadableObject*  Create(CTCLInterpreter& rInterp, 
				  CTCLResult& rResult,
				  int nargs, char** pargs);
  virtual std::string Help();
};


#endif
