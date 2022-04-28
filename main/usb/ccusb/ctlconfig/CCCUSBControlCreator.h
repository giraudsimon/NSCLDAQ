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

#ifndef __CCCUSBCONTROLCREATOR_H
#define __CCCUSBCONTROLCREATOR_H


/**
 * @file CCCUSBControlCreator.h
 * @brief Creates CCCUSBControl objects...wrappers for Tcl driver instances.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

#include <memory>
#include <CControlHardware.h>

#ifndef __CMODULECREATOR_H
#include <CModuleCreator.h>
#endif


/**
 * Concrete CModuleCreator that created Tcl driver wrapper objects.
 */
class CCCUSBControlCreator : public CModuleCreator
{
public:
  virtual CControlHardware* operator()();
};


#endif
