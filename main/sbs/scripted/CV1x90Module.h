#ifndef CV1X90MODULE_H
#define CV1x90MODULE_H

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

//
//  Header files:

#include "CDigitizerModule.h"
#include <string>

using namespace std;

// forward class definitions:

class CTCLInterpreter;
class CCAENV1x90;

/*!
  This class encapsulates the CAENV1x90 device class. The end result of this is to provide
  support for the V1190/V1290 TDC modules.
  Configurable parameters are:
  - base         - Base address of the module.
  - vsn          - Module virtual slot number
  - crate        - VME crate in which the module is installed.
  - termination  - Defines how to termiante the module valid values are:
                   swtich on, off  The default is on.
  - tagtriggertime - on/off  defaults to on.  Determines if data are tagged with the trigger time.
  - eclprogout   - Defines what appears on the ECL programmable output, can be one of:
                   dataready, full, almostfull, error, where the default is dataready.
  - triggermode  - One of continuous and matching where the default is matching.
  - windowidth   - Trigger matching window width.
  - extramargin  - Set the extra search margin.
  - rejectmargin - Set the reject margin.
  - subtracttriggertime - Defaults to enabled.
  - edgedetectmode - Set the edge detection mode to be one of:
                     pair leading trailing or both.  Defaults to leading.
  - encapsulatetdc - defaults to true enables encapsulating the data from one tdc chip.
  - maxhits        - One of 1 2 4 8 16 32 64 128, unlimited, defaults to unlimited.
  - individuallsb  - Sets meaning of the individual least significant bits, can be one of 
                     25ps, 100ps, 200ps, or 800ps  defaults to 100ps.
  - pairleresolution - Can be one of 100ps 200ps 400ps 800ps 1600ps 3120ps 6250ps 12500ps
                       defaults to 100ps.  The leading edge resolution in pair mode.
  - pairwidthresolution - Can be one of 100ps 200ps 400ps 800ps 1600ps 3200ps 6250ps 12500ps 25ns,
                          50ns 100ns 200ns  400ns or 800ns.  Defaults to 100ps.
  - deadtime      - One of 5ns, 10ns, 30ns, or 100ns, defaults to 5ns.
                

*/
class CV1x90Module : public CDigitizerModule
{
private:
  CCAENV1x90*       m_pModule;
public:
  // constructors and other canonicals:

  CV1x90Module(std::string rCommand,
	       CTCLInterpreter& interp);
  virtual ~CV1x90Module();

private:
  CV1x90Module(const CV1x90Module& rhs);
  CV1x90Module& operator=(const CV1x90Module& rhs);
  int operator==(const CV1x90Module& rhs) const;
  int operator!=(const CV1x90Module& rhs) const;

  // The module digitizer module interface:

  virtual   void Initialize ();  
  virtual   void Prepare ();  
  virtual   int  Read(void*  pBuffer);
  virtual   void Clear ();
  virtual   std::string getType() const;

};

#endif
