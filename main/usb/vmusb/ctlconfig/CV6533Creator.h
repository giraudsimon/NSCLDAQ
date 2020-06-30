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
#ifndef CV6553CREATOR_H
#define CV6553CREATOR_H

/**
 * @file CV6553Creator.h
 * @brief Defines a creational for the V6553 HV Module.
 */

#include <CModuleCreator.h>
#include <CControlHardware.h>
#include <memory>

/**
 * Concrete CModuleCreator that creates a CV6553 module.
 */
class CV6533Creator : public CModuleCreator
{
public:
  virtual CControlHardware* operator()();
};


#endif
