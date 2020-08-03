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

#ifndef CCONTROLHARDWARE_H
#define CCONTROLHARDWARE_H

#include <CVMUSB.h>
#include <CControlHardwareT.h>
class CVMUSBReadoutList;

using CControlHardware = CControlHardwareT<CVMUSB>;
/**
 * @class CVMUSBControlHardware
 *    This specialization allows us to provide VMUSB specific
 *    utilities that all derived classes  can use.
 */
class CVMUSBControlHardware : public ::CControlHardware
{
protected:
    void doList(
        CVMUSB& ctlr, CVMUSBReadoutList& list,
        void* data, size_t expectedSize, size_t* actualSize,
        const char* msg
);
};
#endif
