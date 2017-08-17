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
#ifndef __CCHICOTRIGGERCREATOR_H
#define __CCHICOTRIGGERCREATOR_H


/**
 * @file CChicoTriggerCreator.h 
 * @brief Defines a creational for a Chico trigger control module.
 */

#ifndef __CMODULECREATOR_H
#include <CModuleCreator.h>
#endif

#include <CControlHardware.h>
#include <memory>

/**
 * Concrete CModuleCreator that creates a CChicoTrigger module.
 */
class CChicoTriggerCreator : public CModuleCreator
{
  virtual CControlHardware* operator()();
};

#endif
