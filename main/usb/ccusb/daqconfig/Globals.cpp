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
#include <string>
#include <stdint.h>
#include <TCLInterpreter.h>
#include <tcl.h>


using std::string;

class CConfiguration;
class CCCUSB;
class TclServer;
class CTheApplication;

namespace Globals {
  CConfiguration*    pConfig;
  string             configurationFilename;
  string             controlConfigFilename;
  CCCUSB*            pUSBController;
  uint32_t           bufferSize(4*1024*sizeof(uint16_t)); 
  unsigned           scalerPeriod;
  unsigned           sourceId;
  char*              pTimestampExtractor;
  TclServer*         pTclServer;
  CTCLInterpreter*   pMainInterpreter(0);
  Tcl_ThreadId           mainThread;
  CTheApplication*   pApplication;
};
