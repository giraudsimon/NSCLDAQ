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

#ifndef __CREADOUTHARDWARE_H
#define __CREADOUTHARDWARE_H

// Forward class definitions:

class CVMUSB;
class CVMUSBReadoutList;
class CReadoutModule;

/*!
   An abstract base class for modules that can be read out.
   These modules get associated with configurations that
   they are then able to read.  Modules that can be read out
   are expected to have to do two things:
   - Prepare for activity (initialization).
   - Supply the list of things that need to be done to read them out.

   The controller we are using is a VM-USB controller. While it may
   be acceptable to do initialization via individual operations or a one-shot
   list, performance of the main readout must be done in VM-USB autonomous daq mode.
   The strategy is therefore to provide an initialization entry that is called
   just prior to the readout going active, and another entry which is handed
   the DAQ list.  The module is then supposed to add its contribution to the
   DAQ list.
*/
class CReadoutHardware
{
public:
  virtual ~CReadoutHardware() {} // Needed to chain destructors.
  
  // The following need to be implemented by concrete modules:

  virtual void onAttach(CReadoutModule& configuration) = 0;
  virtual void Initialize(CVMUSB& controller) = 0;
  virtual void addReadoutList(CVMUSBReadoutList& list) = 0;
  virtual void onEndRun(CVMUSB& ) {}
  virtual CReadoutHardware* clone() const = 0;
};

#endif
