/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University`
	     East Lansing, MI 48824-1321
*/

#ifndef __CREADOUTHARDWARE_H
#define __CREADOUTHARDWARE_H


#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRTL_STDINT_H
#include <stdint.h>
#ifndef __CRTL_STDINT_H
#define __CRTL_STDINT_H
#endif
#endif



// Forward class definitions:

class CCCUSB;
class CCCUSBReadoutList;
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
protected:
  CReadoutModule*  m_pConfiguration;
public:
  virtual ~CReadoutHardware() {} // Needed to chain destructors.
  
  // The following need to be implemented by concrete modules:

  virtual void onAttach(CReadoutModule& configuration) = 0;
  virtual void Initialize(CCCUSB& controller) = 0;
  virtual void addReadoutList(CCCUSBReadoutList& list) = 0;
  virtual void onEndRun(CCCUSB& controller) {} 
  virtual CReadoutHardware* clone() const = 0;

  // Utilities factored out of derived classes:
protected:
  unsigned int getIntegerParameter(std::string name);
  bool         getBoolParameter(std::string name);
  void         getArray(std::string name, std::vector<uint16_t>& values);
  void         checkedControl(CCCUSB& controller,
			      int n, int a, int f, std::string msgFormat,
			      bool needQ = true);
  void         checkedWrite16(CCCUSB& controller,
			      int n, int a, int f, uint16_t data,
			      std::string msgFormat,
			      bool needQ = true);
  void         check(int status, uint16_t qx, 
		     int n, int a, int f, int d,
		     std::string prefix, std::string format,
		     bool needQ = true);
};

#endif
