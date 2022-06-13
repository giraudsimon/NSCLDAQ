%module CCCUSB


%include <std_string.i>
%include <stdint.i>

%ignore CCCUSB::executeList(CCCUSBReadoutList& list, void* pReadBuffer, size_t readBufferSize, size_t* bytesRead);
%ignore CCCUSBusb::executeList(CCCUSBReadoutList& list, void* pReadBuffer, size_t readBufferSize, size_t* bytesRead);

%{
  #include <CCCUSB.h> 
  #include <CCCUSBusb.h> 
  #include <CCCUSBRemote.h> 

  class CTCLApplication;
  CTCLApplication *gpTCLApplication = 0;
%}

%include "CCCUSB.h"
%include "CCCUSBusb.h"
%include "CCCUSBRemote.h"

