/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef __CV812CREATOR_H
#define __CV812CREATOR_H

/**
 * @file CV812Creator.h
 * @brief define a creator for CV812 in the module factory.
 */

#ifndef __CMODULECREATOR_H
#include <CModuleCreator.h>
#endif

#include <CControlHardware.h>
#include <memory>

/**
 * Concrete CModuleCreator that creates a CV812 module.
 */
class CV812Creator : public CModuleCreator
{
public:
  virtual CControlHardware* operator()();
};

#endif
