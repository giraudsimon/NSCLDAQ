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


#ifndef __CVMUSBREMOTE_H
#define __CVMUSBREMOTE_H

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
 *  Restrictions:
 *   - lists cannot be downloaded (only immediate lists can be run).
 *   - the action register is not accessible.
 *   - The IRQ Mask cannot be written as that abomination requires 
 *     action register operations
 *   - while internal registers can be manipulated be aware that
 *     # This may cuase an active readout to have undefined results.
 *     # Registers you write may be overwritten by Readout's startup 
 *     # code.
 */
class CVMUSBRemote
{

    // Class member data.
private:
  std::string      m_deviceName;
  CSocket*         m_pSocket;	/* Connection with remote system. */
  CTCLInterpreter* m_pInterp;	/* Having this makes TclList processing easier. */
  std::string      m_lastError;
public:

    // Constructors and other canonical functions.
    // Note that since destruction closes the handle and there's no
    // good way to share usb objects, copy construction and
    // assignment are forbidden.
    // Furthermore, since constructing implies a usb_claim_interface()
    // and destruction implies a usb_release_interface(),
    // equality comparison has no useful meaning either:

  CVMUSBRemote(std::string deviceName = "vmusb", 
	       std::string host = "localhost",
	       unsigned int port = 27000);
    virtual ~CVMUSBRemote();		// Although this is probably a final class.

    // Disallowed functions as described above.
private:
    CVMUSBRemote(const CVMUSBRemote& rhs);
    CVMUSBRemote& operator=(const CVMUSBRemote& rhs);
    int operator==(const CVMUSBRemote& rhs) const;
    int operator!=(const CVMUSBRemote& rhs) const;
public:
    std::string getLastError() {
      return m_lastError;
    }
    // Register I/O operations.
public:
    void writeActionRegister(uint16_t value);

    uint32_t readFirmwareID();

    void     writeGlobalMode(uint16_t value);
    uint16_t readGlobalMode();

    void     writeDAQSettings(uint32_t value);
    uint32_t readDAQSettings();

    void     writeLEDSource(uint32_t value);
    uint32_t readLEDSource();

    void     writeDeviceSource(uint32_t value);
    uint32_t readDeviceSource();

    void     writeDGG_A(uint32_t value);
    uint32_t readDGG_A();

    void     writeDGG_B(uint32_t value);
    uint32_t readDGG_B();

    void     writeDGG_Extended(uint32_t value);
    uint32_t readDGG_Extended();

    uint32_t readScalerA();
    uint32_t readScalerB();


    void     writeVector(int which, uint32_t value);
    uint32_t readVector(int which);


    void     writeBulkXferSetup(uint32_t value);
    uint32_t readBulkXferSetup();


    

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

    // Support for immediate counted VME variable block transfer operations:
    // See comments prior to CVMEReadoutList::addBlockCountMask

    int vmeReadBlockCount8(uint32_t address,  uint32_t mask, uint8_t amod);
    int vmeReadBlockCount16(uint32_t address, uint32_t mask, uint8_t amod);
    int vmeReadBlockCount32(uint32_t address, uint32_t mask, uint8_t amod);
    int vmeVariableBlockRead(uint32_t address, uint8_t amod, 
			     void* data, size_t maxCount, size_t* countTransferred);
    int vmeVariableFifoRead(uint32_t address, uint8_t amod,  
			    void* data, size_t maxCount,  size_t* countTransferred);
    
    // List operations.

public:
    int executeList(CVMUSBReadoutList& list,
		    void*               pReadBuffer,
		    size_t              readBufferSize,
		    size_t*             bytesRead);
    

    int loadList(uint8_t listNumber, CVMUSBReadoutList& list, off_t offset);

    // Register bit definintions.

public: 
    class RegisterOffsets {
      static const unsigned int FIDRegister = (0);       // Firmware id.
      static const unsigned int GMODERegister = (4);     // Global mode register.
      static const unsigned int DAQSetRegister = (8);    // DAQ settings register.
      static const unsigned int LEDSrcRegister = (0xc);	// LED source register.
      static const unsigned int DEVSrcRegister = (0x10);	// Device source register.
      static const unsigned int DGGARegister = (0x14);   // GDD A settings.
      static const unsigned int DGGBRegister = (0x18);   // GDD B settings.
      static const unsigned int ScalerA = (0x1c);        // Scaler A counter.
      static const unsigned int ScalerB = (0x20);        // Scaler B data.
      static const unsigned int ExtractMask = (0x24);    // CountExtract mask.
      static const unsigned int ISV12 = (0x28);          // Interrupt 1/2 dispatch.
      static const unsigned int ISV34 = (0x2c);          // Interrupt 3/4 dispatch.
      static const unsigned int ISV56 = (0x30);          // Interrupt 5/6 dispatch.
      static const unsigned int ISV78 = (0x34);          //  Interrupt 7/8 dispatch.
      static const unsigned int DGGExtended = (0x38);    // DGG Additional bits.
      static const unsigned int USBSetup = (0x3c);       // USB Bulk transfer setup. 
      static const unsigned int USBVHIGH1 = (0x40);      // High bits of ISV12/34.
      static const unsigned int USBVHIGH2 = (0x44);      // High bits of ISV56/78.

    };

    class ActionRegister {   // e.g. CVMUSBRemote::ActionRegister::startDAQ is a bit.
    public:
	static const uint16_t startDAQ   = 1;
	static const uint16_t usbTrigger = 2;
	static const uint16_t clear      = 4;
	static const uint16_t sysReset   = 8;
	static const uint16_t scalerDump = 0x10;
    };
    
    class FirmwareRegister {
    public:
	static const uint32_t minorRevMask     = 0x000000ff;
	static const uint32_t minorRevShift    = 0;
	
	static const uint32_t majorRevMask     = 0x0000ff00;
	static const uint32_t majorRevShift    = 8;


	// These are my best guesses.

	static const uint32_t betaVersionMask  = 0x00ff0000;
	static const uint32_t betaVersionShift = 16;

	static const uint32_t yearMask         = 0x0f000000;
	static const uint32_t yearShift        = 24;
       
	static const uint32_t monthMask        = 0xf0000000;
	static const uint32_t monthshift       = 27;
    };

    class GlobalModeRegister {
    public:
	static const uint16_t bufferLenMask    = 0xf;
	static const uint16_t bufferLenShift   = 0;
	static const uint16_t bufferLen13K     = 0;
	static const uint16_t bufferLen8K      = 1;
	static const uint16_t bufferLen4K      = 2;
	static const uint16_t bufferLen2K      = 3;
	static const uint16_t bufferLen1K      = 4;
	static const uint16_t bufferLen512     = 5;
	static const uint16_t bufferLen256     = 6;
	static const uint16_t bufferLen128     = 7;
	static const uint16_t bufferLen64      = 8;
	static const uint16_t bufferLenSingle  = 9;

	static const uint16_t mixedBuffers     = 0x20;
	static const uint16_t doubleSeparater  = 0x40;
	static const uint16_t align32          = 0x80;
	
	static const uint16_t doubleHeader     = 0x100;
	static const uint16_t flushScalers     = 0x200;
	static const uint16_t busReqLevelMask  = 0x7000;
	static const uint16_t busReqLevelShift = 12;
    };
    class DAQSettingsRegister {
    public:
	static const uint32_t readoutTriggerDelayMask     = 0xff;
	static const uint32_t readoutTriggerDelayShift    = 0;
	
	static const uint32_t scalerReadoutPeriodMask     = 0xff00;
	static const uint32_t scalerReadoutPeriodShift    = 8;

	static const uint32_t scalerReadoutFrequenyMask   = 0xffff0000;
	static const uint32_t scalerReadoutFrequencyShift = 16;
    };
    class LedSourceRegister {
    public:
	// Top yellow led:

	static const uint32_t topYellowOutFifoNotEmpty    = 0;
	static const uint32_t topYellowInFifoNotEmpty     = 1;
	static const uint32_t topYellowScalerEvent        = 2;
	static const uint32_t topYellowInFifoFull         = 3;
	static const uint32_t topYellowDTACK              = 4;
	static const uint32_t topYellowBERR               = 5;
	static const uint32_t topYellowBusRequest         = 6;
	static const uint32_t topYellowBusGranted         = 7;
	static const uint32_t topYellowInvert             = 0x8;
	static const uint32_t topYellowLatch              = 0x10;

	// Red LED:

	static const uint32_t redEventTrigger           = (0 << 8);
	static const uint32_t redNimInput1              = (1 << 8);
	static const uint32_t redNimInput2              = (2 << 8);
	static const uint32_t redBusy                   = (3 << 8);
	static const uint32_t redDTACK                  = (4 << 8);
	static const uint32_t redBERR                   = (5 << 8);
	static const uint32_t redBusRequest             = (6 << 8);
	static const uint32_t redBusGranted             = (7 << 8);
	static const uint32_t redInvert                 = (8 << 8);
	static const uint32_t redLatch                  = (0x10 << 8);

	// Green led:

	static const uint32_t greenAcquire              = (0 << 16);
	static const uint32_t greenStackNotEmpty        = (1 << 16);
	static const uint32_t greenEventReady           = (2 << 16);
	static const uint32_t greenEventTrigger         = (3 << 16);
	static const uint32_t greenDTACK                = (4 << 16);
	static const uint32_t greenBERR                 = (5 << 16);
	static const uint32_t greenBusRequest           = (6 << 16);
	static const uint32_t greenBusGranted           = (7 << 16);
	static const uint32_t greenInvert               = (8 << 16);
	static const uint32_t greenLatch                = (0x10 << 16);

	// Bottom yellow LED

	static const uint32_t bottomYellowNotArbiter    = (0 << 24);
	static const uint32_t bottomYellowUsbTrigger    = (1 << 24);
	static const uint32_t bottomYellowUSBReset      = (2 << 24);
	static const uint32_t bottomYellowBERR1         = (3 << 24);
	static const uint32_t bottomYellowDTACK         = (4 << 24);
	static const uint32_t bottomYellowBERR          = (5 << 24);
	static const uint32_t bottomYellowBusRequest    = (6 << 24);
	static const uint32_t bottomYellowBusGranted    = (7 << 24);
	static const uint32_t bottomYellowInvert        = (8 << 24);
	static const uint32_t bottomYellowLatch         = (0x10 << 24);
    };
    class DeviceSourceRegister {
    public:
	static const uint32_t nimO1Busy                 = 0;
	static const uint32_t nimO1Trigger              = 1;
	static const uint32_t nimO1BusRequest           = 2;
	static const uint32_t nimO1EventToBuffer        = 3;
	static const uint32_t nimO1DGGA                 = 4;
	static const uint32_t nimO1DGGB                 = 5;
	static const uint32_t nimO1EndOfEvent           = 6;
	static const uint32_t nimO1UsbTrigger           = 7;
	static const uint32_t nimO1Invert               = 8;
	static const uint32_t nimO1Latch                = 0x10;

	static const uint32_t nimO2UsbTrigger1          = (0 << 8);
	static const uint32_t nimO2VMEExecuting         = (1 << 8);
	static const uint32_t nimO2VMEAS                = (2 << 8);
	static const uint32_t nimO2DataToUsbFIFO        = (3 << 8);
	static const uint32_t nimO2DGGA                 = (4 << 8);
	static const uint32_t nimO2DGGB                 = (5 << 8);
	static const uint32_t nimO2EndOfEvent           = (6 << 8);
	static const uint32_t nimO2UsbTrigger           = (7 << 8);
	static const uint32_t nimO2Invert               = (8 << 8);
	static const uint32_t nimO2Latch                = (0x10 << 8);

	static const uint32_t scalerADisabled           = (0   << 16);
	static const uint32_t scalerANIMI1              = (1   << 16);
	static const uint32_t scalerANIMI2              = (2   << 16);
	static const uint32_t scalerAEvent              = (2   << 16);
	static const uint32_t scalerAEnable             = (8   << 16);
	static const uint32_t scalerAReset              = (0x10 << 16);

	static const uint32_t scalerBDisabled           = (0   << 20);
	static const uint32_t scalerBNIMI1              = (1   << 20);
	static const uint32_t scalerBNIMI2              = (2   << 20);
	static const uint32_t scalerBEvent              = (2   << 20);
	static const uint32_t scalerBEnable             = (8   << 20);
	static const uint32_t scalerBReset              = (0x10 << 20);

	static const uint32_t dggADisabled              = (0   << 24);
	static const uint32_t dggANIMI1                 = (1   << 24);
	static const uint32_t dggANIMI2                 = (2   << 24);
	static const uint32_t dggAEventTrigger          = (3   << 24);
	static const uint32_t dggAEndOfEvent            = (4   << 24);
	static const uint32_t dggAUsbTrigger            = (5   << 24);
	static const uint32_t dggAPulser                = (6   << 24);

	static const uint32_t dggBDisabled              = (0   << 28);
	static const uint32_t dggBNIMI1                 = (1   << 28);
	static const uint32_t dggBNIMI2                 = (2   << 28);
	static const uint32_t dggBEventTrigger          = (3   << 28);
	static const uint32_t dggBEndOfEvent            = (4   << 28);
	static const uint32_t dggBUsbTrigger            = (5   << 28);
	static const uint32_t dggBPulser                = (6   << 28);


    };
    
    class DGGAndPulserRegister {
    public:
	static const uint32_t dggFineDelayMask        = 0xffff;
	static const uint32_t dggFineDelayShift       = 0;
	
	static const uint32_t dggGateWidthMask        = 0xffff0000;
	static const uint32_t dggGateWidthShift       = 16;

    };

    class DGGCoarseRegister {
    public:
	static const uint32_t ACoarseMask             = 0xffff;
	static const uint32_t ACoarseShift            = 0;
	
	static const uint32_t BCoarseMask             = 0xffff0000;
	static const uint32_t BCoarseShift            = 16;

    };
    // There are two vectors per register called A/B in this set of 

    class ISVRegister {
    public:
	static const uint32_t AVectorMask             = 0xff;
	static const uint32_t AVectorShift            = 0;
	static const uint32_t AIPLMask                = 0x700;
	static const uint32_t AIPLShift               = 8;
	static const uint32_t AStackIDMask            = 0x7000;
	static const uint32_t AStackIDShift           = 12;

	static const uint32_t BVectorMask             = 0xff0000;
	static const uint32_t BVectorShift            = 16;
	static const uint32_t BIPLMask                = 0x7000000;
	static const uint32_t BIPLShift               = 24;
	static const uint32_t BStackIDMask            = 0x70000000;
	static const uint32_t BStackIDShift           = 28;
    };

    class TransferSetupRegister {
    public:
	static const uint32_t multiBufferCountMask   = 0xff;
	static const uint32_t multiBufferCountShift  = 0;

	static const uint32_t timeoutMask            = 0xf00;
	static const uint32_t timeoutShift           = 8;

    };
    // Local functions:
private:

    void  writeRegister(unsigned int address, uint32_t data);
    uint32_t readRegister(unsigned int address);
    unsigned int whichToISV(int which);
    int   doVMEWrite(CVMUSBReadoutList& list);
    int   doVMERead(CVMUSBReadoutList&  list, uint32_t* datum);

    std::string marshallList(CVMUSBReadoutList& list);
    size_t      marshallOutputData(void* pOutputBuffer, const char* reply, size_t maxOutputSize);


};



#endif
