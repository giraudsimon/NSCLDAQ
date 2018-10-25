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


#ifndef __CVMUSBETHERNET_H
#define __CVMUSBETHERNET_H
#ifndef __CVMUSB
#include "CVMUSB.h"
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __CRT_SYS_TYPES_H
#include <sys/types.h>
#ifndef __CRT_SYS_TYPES_H
#define __CRT_SYS_TYPES_H
#endif
#endif


#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

// Forward Class definitions:

class CVMUSBReadoutList;
class CSocket;
class CTCLInterpreter;

/**
 *  This class provides remote access to the VM-USB through the
 *  VMUSBModule support in the tcl server.
 *  An external program can link to libVMUSBRemote.so, instantiate
 *  this class and pretty much have its way with the VM-USB
 *  as if it owned it.  
 */
class CVMUSBEthernet : public CVMUSB
{

    // Class member data.
private:
  std::string      m_deviceName;
  CSocket*         m_pSocket;	/* Connection with remote system. */
  CTCLInterpreter* m_pInterp;	/* Having this makes TclList processing easier. */
  std::string      m_lastError;
  uint16_t         m_irqMask; // interrupt mask shadow register.
  std::string      m_host;
  unsigned int     m_port;

public:

    // Constructors and other canonical functions.
    // Note that since destruction closes the handle and there's no
    // good way to share usb objects, copy construction and
    // assignment are forbidden.
    // Furthermore, since constructing implies a usb_claim_interface()
    // and destruction implies a usb_release_interface(),
    // equality comparison has no useful meaning either:

  CVMUSBEthernet(std::string deviceName = "vmusb", 
	       std::string host = "localhost",
	       unsigned int port = 20000);
    virtual ~CVMUSBEthernet();		// Although this is probably a final class.

    // Disallowed functions as described above.
private:
    CVMUSBEthernet(const CVMUSBEthernet& rhs);
    CVMUSBEthernet& operator=(const CVMUSBEthernet& rhs);
    int operator==(const CVMUSBEthernet& rhs) const;
    int operator!=(const CVMUSBEthernet& rhs) const;
public:
    bool reconnect();

    std::string getLastError() {
      return m_lastError;
    }
    // Register I/O operations.
public:
    void writeActionRegister(uint16_t value);
    void  writeRegister(unsigned int address, uint32_t data);
    uint32_t readRegister(unsigned int address);

    int readFirmwareID();

    void     writeGlobalMode(uint16_t value);
    int readGlobalMode();

    void     writeDAQSettings(uint32_t value);
    int readDAQSettings();

    void     writeLEDSource(uint32_t value);
    int readLEDSource();

    void     writeDeviceSource(uint32_t value);
    int readDeviceSource();

    void     writeDGG_A(uint32_t value);
    uint32_t readDGG_A();

    void     writeDGG_B(uint32_t value);
    uint32_t readDGG_B();

    void     writeDGG_Extended(uint32_t value);
    uint32_t readDGG_Extended();

    uint32_t readScalerA();
    uint32_t readScalerB();


    void     writeVector(int which, uint32_t value);
    int readVector(int which);

    void     writeIrqMask(uint8_t mask);
    int      readIrqMask();


    void     writeBulkXferSetup(uint32_t value);
    int readBulkXferSetup();


    

    // VME transfer operations (1 shot)

    int vmeWrite32(uint32_t address, uint8_t aModifier, uint32_t data);
    int vmeRead32(uint32_t address, uint8_t aModifier, uint32_t* data);

    int vmeWrite16(uint32_t address, uint8_t aModifier, uint16_t data);
    int vmeRead16(uint32_t address, uint8_t aModifier, uint16_t* data);

    int vmeWrite8(uint32_t address, uint8_t aModifier, uint8_t data);
    int vmeRead8(uint32_t address, uint8_t aModifier, uint8_t* data);


    int vmeBlockRead(uint32_t baseAddress, uint8_t aModifier,
		     void* data,  size_t transferCount, size_t* countTransferred);
    int vmeFifoRead(uint32_t address, uint8_t aModifier,
		    void* data, size_t transferCount, size_t* countTransferred);

    // List operations.

public:
    int executeList(CVMUSBReadoutList& list,
		    void*               pReadBuffer,
		    size_t              readBufferSize,
		    size_t*             bytesRead);
    

    int loadList(uint8_t listNumber, CVMUSBReadoutList& list, off_t offset);
    
    virtual int usbRead(void* data, size_t bufferSize, size_t* transferCount,
			int timeout = 2000);

 
private:


    std::string marshallList(CVMUSBReadoutList& list);
    size_t      marshallOutputData(void* pOutputBuffer, const char* reply, size_t maxOutputSize);
    void openServer();


};



#endif
