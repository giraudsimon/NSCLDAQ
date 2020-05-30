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
#ifdef SWIG
#ifndef _FLATTEN_NESTED_CLASSES
#define _FLATTEN_NESTED_CLASSES
#endif
//%module CCCUSB
//%{
#define _FLATTEN_NESTED_CLASSES
//#include "CCCUSB.h"
//%}

#endif

#ifndef __CCCUSB_H
#define __CCCUSB_H


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

#ifndef __STL_STRING
#include <iosfwd>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CCCUSBREADOUTLIST_H
#include <CCCUSBReadoutList.h>
#ifndef __CCCUSBREADOUTLIST_H
#define __CCCUSBREADOUTLIST_H
#endif
#endif

class CMutex;

/*!
  This class is the low level support for the Wiener/JTEC CCUSB module.
the CCUSB is a USB CAMAC controller.  To use this class you must first locate
a module by invoking enumerate.  enumerate  returns a vector of
usb_device*s.   One of those can be used to instantiate the CCCUSB 
object which can the ben operated on.

Note there are two sets of methods defined. Those with very precise
parameter types and those with quite generic parameter types.
The precise types are intended to be used with C++ clients.
The generic types are intended to be used in SWIG wrappers since
SWIG is not able to convert int -> uint16_t nor handle references
without helper functions.

*/
class CCCUSB 
{
  public:
    typedef CCCUSBReadoutList RdoList;
    /**! Register shadow
    * Contains the most recent values of the CCUSB
    */ 
    struct ShadowRegisters {
      uint32_t firmwareID;
      uint16_t globalMode;
      uint16_t delays; 
      uint32_t scalerReadoutControl;
      uint32_t ledSources;
      uint32_t nimOutputSources; 
      uint32_t userDeviceSources; 
      uint32_t dggA; 
      uint32_t dggB; 
      uint32_t lamMask;
      uint32_t dggExtended; 
      uint32_t bulkTransferSetup;
      uint32_t broadcastMap;

      // Provide default constructor to ensure that 
      // the vector is initialized to a length of 8
      ShadowRegisters() {}
    };
    // Class member data.
    
public:
    static std::vector<struct usb_device*> enumerate();
    static std::string serialNo(struct usb_device* dev);

private:
  static CMutex *m_pGlobalMutex;

public:
    virtual ~CCCUSB();		// Although this is probably a final class.

    // Disallowed functions as described above.
private:
    CCCUSB& operator=(const CCCUSB& rhs);
    int operator==(const CCCUSB& rhs) const;
    int operator!=(const CCCUSB& rhs) const;
public:


    static CMutex& getGlobalMutex();

    virtual bool reconnect() = 0;

    virtual CCCUSBReadoutList* createReadoutList() const 
    { return new CCCUSBReadoutList; }


    // Register I/O operations.
public:
    virtual void     writeActionRegister(uint16_t value) = 0;
    void     writeActionRegister(int value) { /* Swig */
      writeActionRegister((uint16_t)value);
    }


    // The following execute single CAMAC operations.
    // note that the CC-USB defines all registers but the action register to live in
    // CAMAC space.

    virtual int simpleWrite16(int n, int a, int f, uint16_t data, uint16_t& qx);
    virtual int simpleWrite24(int n, int a, int f, uint32_t data, uint16_t& qx);
    virtual int simpleRead16( int n, int a, int f, uint16_t& data, uint16_t& qx);
    virtual int simpleRead24( int n, int a, int f, uint32_t& data, uint16_t& qx);
    virtual int simpleControl(int n, int a, int f, uint16_t& qx);

    // SWIG wrappers for simple I/O.

    int simpleWrite16(int n, int a, int f, int data) {
      uint16_t  qx16;
      checkExecuteError(simpleWrite16(n,a,f, (uint16_t)data, qx16));
      return qx16;
    }


    int simpleWrite24(int n, int a, int f, int data) {

      uint16_t  qx16;
      checkExecuteError(simpleWrite24(n,a,f, (uint16_t)data, qx16));
      return qx16;
    }

    unsigned int simpleRead16(int n, int a, int f)
    {
      uint16_t data;
      uint16_t qx;

      checkExecuteError(simpleRead16(n,a,f,data,qx));
      return (int)data | ((int)qx << 24);	/* qx in the top byte. */
    }
    unsigned int simpleRead24(int n, int a, int f)
    {
      uint32_t data;
      uint16_t qx;
      checkExecuteError(simpleRead24(n,a,f,data,qx));
      
      return data | ((int)qx << 24);
      
    }

    unsigned int simpleControl(int n, int a, int f)
    {
      uint16_t qx;
      checkExecuteError(simpleControl(n,a,f,qx));
      return qx;
    }




    // Convenience function that access the CC-USB registers.
    // Each function or read/write pair of functions is
    // followed by a swig wrapper:


    virtual int readFirmware(uint32_t& value);
    unsigned  readFirmware() {
      uint32_t fw;
      readFirmware(fw);
      return fw;
    }

    virtual int readGlobalMode(uint16_t& value);
    virtual int writeGlobalMode(uint16_t value);
    unsigned readGlobalMode() {	/* swig */
      uint16_t mode;
      readGlobalMode(mode);
      return mode;
    }
    int writeGlobalMode(int value) { /* swig */
      return writeGlobalMode((uint16_t)value);
    }


    virtual int readDelays(uint32_t& value);
    virtual int writeDelays(uint16_t value);
    unsigned  readDelays() {		/* swig */
      uint32_t d;
      readDelays(d);
      return d;
    }
    int writeDelays(int value) { /* swig */
      return writeDelays((uint16_t)value);
    }

    virtual int readScalerControl(uint32_t& value);
    virtual int writeScalerControl(uint32_t value);
    unsigned  readScalerControl() {	/* swig */
      uint32_t value;
      readScalerControl(value);
      return value;
    }
    int writeScalerControl(int value) { /* swig */
      return writeScalerControl((uint32_t)value);
    }


    virtual int readLedSelector(uint32_t& value);
    virtual int writeLedSelector(uint32_t value);
    unsigned  readLedSelector() { /* swig */
      uint32_t v;
      readLedSelector(v);
      return v;
    }
    int writeLedSelector(int value) {
      return writeLedSelector((uint32_t)value);
    }
      
    virtual int readOutputSelector(uint32_t& value);
    virtual int writeOutputSelector(uint32_t value);
    unsigned readOutputSelector() {	/* swig */
      uint32_t v;
      readOutputSelector(v);
      return v;
    }
    int writeOutputSelector(int value) { /* swig */
      return writeOutputSelector((uint32_t)value);
    }
    virtual int readDeviceSourceSelectors(uint32_t& value);
    virtual int writeDeviceSourceSelectors(uint32_t value);
    unsigned readDeviceSourceSelectors() { /* swig */
      uint32_t s;
      readDeviceSourceSelectors(s);
      return s;
    }
    int writeDeviceSourceSelectors(int v) {
      return writeDeviceSourceSelectors((uint32_t)v);
    }

    virtual int readDGGA(uint32_t& value);
    virtual int readDGGB(uint32_t& value);
    virtual int readDGGExt(uint32_t& value);
    unsigned readDGGA() {		/* swig */
      uint32_t v;
      readDGGA(v);
      return v;
    }
    unsigned  readDGGB() {		/* swig */
      uint32_t v;
      readDGGB(v);
      return v;
    }
    unsigned  readDGGExt() {		/* swig */
      uint32_t v;
      readDGGExt(v);
      return v;
    }

    virtual int writeDGGA(uint32_t value);
    virtual int writeDGGB(uint32_t value);
    virtual int writeDGGExt(uint32_t value);
    int writeDGGA(int value) {	/* swig */
      return writeDGGA((uint32_t)value);
    }
    int writeDGGB(int value) {	/* swig */
      return writeDGGB((uint32_t)value);
    }
    int writeDGGExt(int value) { /* swig */
      return writeDGGExt((uint32_t)value);
    }
    

    virtual int readScalerA(uint32_t& value);
    virtual int readScalerB(uint32_t& value);
    unsigned  readScalerA() {	/* swig */
      uint32_t v;
      readScalerA(v);
      return v;
    }
    unsigned  readScalerB() {	/* swig */
      uint32_t v;
      readScalerB(v);
      return v;
    }


    virtual int readLamTriggers(uint32_t& value);
    virtual int writeLamTriggers(uint32_t value);
    unsigned readLamTriggers() {	/* swig */
      uint32_t v;
      readLamTriggers(v);
      return v;
    }
    int writeLamTriggers(int value) { /* swig */
      return writeLamTriggers((uint32_t)value);
    }
				    
    virtual int readCAMACLams(uint32_t& value);
    unsigned readCAMACLams() {	/* swig */
      uint32_t v;
      readCAMACLams(v);
      return v;
    }

    virtual int readUSBBulkTransferSetup(uint32_t& value);
    virtual int writeUSBBulkTransferSetup(uint32_t value);
    unsigned readUSBBulkTransferSetup(){ /* swig */
      uint32_t v;
      readUSBBulkTransferSetup(v);
      return v;
    }
    int writeUSBBulkTransferSetup(int value) { /* swig */
      return writeUSBBulkTransferSetup((uint32_t)value);
    }

    virtual int c();
    virtual int z();
    virtual int inhibit();
    virtual int uninhibit();
    virtual bool isInhibited();
    

    void dumpConfiguration(std::ostream& stream);
    // List operations.

public:
//    int executeList(CCCUSBReadoutList& list,
//		    void*               pReadBuffer,
//		    size_t              readBufferSize,
//		    size_t*             bytesRead);
    virtual int executeList(CCCUSBReadoutList& list,
                            void* pReadBuffer,
                            size_t readBufferSize,
                            size_t* bytesRead) = 0;

    std::vector<uint16_t> executeList(CCCUSBReadoutList& list,
				      int maxReadWords); /* swig */


    virtual int loadList(uint8_t listNumber,
                         CCCUSBReadoutList& list) = 0;


    int loadList(int listNumber, CCCUSBReadoutList& list) {
       return loadList((uint8_t)listNumber, list);
    }



    // Once the interface is in DAQ auntonomous mode, the application
    // should call the following function to read acquired data.

    virtual int usbRead(void* data, size_t bufferSize, size_t* transferCount,
                        int timeout = 2000) = 0;

    // Register bit definintions.

    // Local functions:
protected:
    void* addToPacket16(void* packet,   uint16_t datum);
    void* addToPacket32(void* packet,   uint32_t datum);
    void* getFromPacket16(void* packet, uint16_t* datum);
    void* getFromPacket32(void* packet, uint32_t* datum);

    uint16_t* listToOutPacket(uint16_t ta, CCCUSBReadoutList& list, size_t* outSize);


    virtual int read32(int n, int a, int f, uint32_t& data);
    virtual int read16(int n, int a, int f, uint16_t& data); /* Really just for register reads */

    virtual int write32(int n, int a, int f, uint32_t data, uint16_t& qx);
    virtual int write16(int n, int a, int f, uint16_t data, uint16_t& qx); /*  just for register writes */


  // The following are classes that define bits/fields in the registers of the CC-USB.
  // Each class is one register:
  //! Bits in the Q/X response word for e.g. simple ops.
  
public:
  static const uint16_t Q;
  static const uint16_t X;


public:

  //!  Action register - all data members are individual bits.
#ifndef FLATTEN_NESTED_CLASSES
  class ActionRegister {
  public:
#endif
    static const uint16_t startDAQ   = 1;
    static const uint16_t usbTrigger = 2;
    static const uint16_t clear      = 4;
    static const uint16_t scalerDump = 0x10;
#ifndef FLATTEN_NESTED_CLASSES
  };

#endif
  //! Firmware register *Mask are in place masks, *Shift shift the field to low order justify it
  class FirmwareRegister {
  public:
    static const uint32_t  revisionMask  = 0xff;
    static const uint32_t  revisionShift = 0;

    static const uint32_t  yearMask      = 0xf00;
    static const uint32_t  yearShift     = 8;

    static const uint32_t  monthMask     = 0xf000;
    static const uint32_t  monthShift    = 12;
  };

  //! Fields and value for the Global mode register.
  class GlobalModeRegister {
  public:
    static const uint16_t bufferLenMask    = 0xf;
    static const uint16_t bufferLenShift   = 0;
    static const uint16_t bufferLen4K      = 0;
    static const uint16_t bufferLen2K      = 1;
    static const uint16_t bufferLen1K      = 2;
    static const uint16_t bufferLen512     = 3;
    static const uint16_t bufferLen256     = 4;
    static const uint16_t bufferLen128     = 5;
    static const uint16_t bufferLen64      = 6;
    static const uint16_t bufferLenSingle  = 7;
    
    static const uint16_t mixedBuffers     = 0x20;
    
    static const uint16_t doubleHeader     = 0x100;
    
    static const uint16_t enableSecondary  = 0x1000;
    };

  //!  The Delay register sets the post trigger delay prior to starting a list, and lam timeout.
  class DelayRegister {
  public:
    static const uint16_t triggerDelayMask = 0xff;
    static const uint16_t triggerDelayShift= 0;
    static const uint16_t lamTimeoutMask   = 0xff00;
    static const uint16_t lamTimeoutShift  = 8;
  };

  //!  The Scaler Control register determines when/how often scaler events are read:

  class ScalerControlRegister {
  public:
    static const uint32_t eventsCountMask   = 0xffff; // Events between readouts.
    static const uint32_t eventsCountShift  = 0;

    static const uint32_t timeIntervalMask  = 0xff0000;	// .5 second units between readouts.
    static const uint32_t timeIntervalShift = 16;
  };

  //! The LED source selector register determines which LEDs mean what.

  class LedSourceRegister {
  public:
    // Red LED:

    static const uint32_t redEventTrigger       = 0;
    static const uint32_t redBusy               = 1;
    static const uint32_t redUSBOutFifoNotEmpty = 2;
    static const uint32_t redUSBInFifoNotEmpty  = 3;
    static const uint32_t redInvert             = 0x10;
    static const uint32_t redLatch              = 0x20;
    static const uint32_t redShift              = 0;

    // Green LED:

    static const uint32_t greenShift              = 8;
    static const uint32_t greenAcquire            = (0 << greenShift);
    static const uint32_t greenNimI1              = (1 << greenShift);
    static const uint32_t greenUSBInFifoNotEmpty  = (2 << greenShift);
    static const uint32_t greenUSBTrigger         = (3 << greenShift);
    static const uint32_t greenInvert             = (0x10 << greenShift);
    static const uint32_t greenLatch              = (0x20 << greenShift);

    // Red LED:

    static const uint32_t yellowShift             = 16;
    static const uint32_t yellowNimI2             = (0 << yellowShift);
    static const uint32_t yellowNimI3             = (1 << yellowShift);
    static const uint32_t yellowBusy              = (2 << yellowShift);
    static const uint32_t yellowUsbInFifoNotEmpty = (3 << yellowShift);
    static const uint32_t yellowInvert            = (0x10 << yellowShift);
    static const uint32_t yellowLatch             = (0x20 << yellowShift);
    
  };
  //! The Output selector register determines the meaning of the NIM Outputs.

  class OutputSourceRegister {
  public:

    // NIM O1 source:

    static const uint32_t nimO1Shift              = 0;
    static const uint32_t nimO1Busy               = (0 << nimO1Shift);
    static const uint32_t nimO1Event              = (1 << nimO1Shift);
    static const uint32_t nimO1DGGA               = (2 << nimO1Shift);
    static const uint32_t nimO1DGGB               = (3 << nimO1Shift);
    static const uint32_t nimO1Invert             = (0x10 << nimO1Shift);
    static const uint32_t nimO1Latch              = (0x20 << nimO1Shift);

    // NIM O2 source:

    static const uint32_t nimO2Shift              = 8;
    static const uint32_t nimO2Acquire            = (0 << nimO2Shift);
    static const uint32_t nimO2Event              = (1 << nimO2Shift);
    static const uint32_t nimO2DGGA               = (2 << nimO2Shift);
    static const uint32_t nimO2DGGB               = (3 << nimO2Shift);
    static const uint32_t nimO2Invert             = (0x10 << nimO2Shift);
    static const uint32_t nimO2Latch              = (0x20 << nimO2Shift);


    // NIM O3 source:

    static const uint32_t nimO3Shift              = 16;
    static const uint32_t nimO3BusyEnd            = (0 << nimO3Shift);
    static const uint32_t nimO3Busy               = (1 << nimO3Shift);
    static const uint32_t nimO3DGGA               = (2 << nimO3Shift);
    static const uint32_t nimO3DGGB               = (3 << nimO3Shift);
    static const uint32_t nimO3Invert             = (0x10 << nimO3Shift);
    static const uint32_t nimO3Latch              = (0x20 << nimO3Shift);
    
  };
  //! Device source selector sets up the inputs to the internal devices:

  class DeviceSourceSelectorsRegister {
  public:
    // Scaler A source/control

    static const uint32_t scalerAShift           = 0;
    static const uint32_t scalerADisabled        = (0 << scalerAShift);
    static const uint32_t scalerANimI1           = (1 << scalerAShift);
    static const uint32_t scalerANimI2           = (2 << scalerAShift);
    static const uint32_t scalerANimI3           = (3 << scalerAShift);
    static const uint32_t scalerAEvent           = (4 << scalerAShift);
    static const uint32_t scalerAScalerBCarry    = (5 << scalerAShift);
    static const uint32_t scalerADGGA            = (6 << scalerAShift);
    static const uint32_t scalerADGGB            = (7 << scalerAShift);
    static const uint32_t scalerAEnable          = (0x10 << scalerAShift);
    static const uint32_t scalerAReset           = (0x20 << scalerAShift);
    static const uint32_t scalerAFreeze          = (0x40 << scalerAShift);

    // Scaler B source/control

    static const uint32_t scalerBShift           = 8;
    static const uint32_t scalerBDisabled        = (0 << scalerBShift);
    static const uint32_t scalerBNimI1           = (1 << scalerBShift);
    static const uint32_t scalerBNimI2           = (2 << scalerBShift);
    static const uint32_t scalerBNimI3           = (3 << scalerBShift);
    static const uint32_t scalerBEvent           = (4 << scalerBShift);
    static const uint32_t scalerBScalerACarry    = (5 << scalerBShift);
    static const uint32_t scalerBDGGA            = (6 << scalerBShift);
    static const uint32_t scalerBDGGB            = (7 << scalerBShift);
    static const uint32_t scalerBEnable          = (0x10 << scalerBShift);
    static const uint32_t scalerBReset           = (0x20 << scalerBShift);
    static const uint32_t scalerBFreeze          = (0x40 << scalerBShift);

    // DGGA source

    static const uint32_t DGGAShift              = 16;
    static const uint32_t DGGADisabled           = (0 << DGGAShift);
    static const uint32_t DGGANimI1              = (1 << DGGAShift);
    static const uint32_t DGGANimI2              = (2 << DGGAShift);
    static const uint32_t DGGANimI3              = (3 << DGGAShift);
    static const uint32_t DGGAEvent              = (4 << DGGAShift);
    static const uint32_t DGGABusyEnd            = (5 << DGGAShift);
    static const uint32_t DGGAUSBTrigger         = (6 << DGGAShift);
    static const uint32_t DGGAPulser             = (7 << DGGAShift);

    

    // DGGB source

    static const uint32_t DGGBShift              = 24;
    static const uint32_t DGGBDisabled           = (0 << DGGBShift);
    static const uint32_t DGGBNimI1              = (1 << DGGBShift);
    static const uint32_t DGGBNimI2              = (2 << DGGBShift);
    static const uint32_t DGGBNimI3              = (3 << DGGBShift);
    static const uint32_t DGGBEvent              = (4 << DGGBShift);
    static const uint32_t DGGBBusyEnd            = (5 << DGGBShift);
    static const uint32_t DGGBUSBTrigger         = (6 << DGGBShift);
    static const uint32_t DGGBPulser             = (7 << DGGBShift);

 
  };
  //! Bit fields for the two DGG gate width/delay registers.
  class DGGAndPulserRegister {
  public:
    static const uint32_t dggFineDelayMask        = 0xffff;
    static const uint32_t dggFineDelayShift       = 0;
    
    static const uint32_t dggGateWidthMask        = 0xffff0000;
    static const uint32_t dggGateWidthShift       = 16;
    
  };
  //! Bit fields for the extended course delay register.
  class DGGCoarseRegister {
  public:
    static const uint32_t ACoarseMask             = 0xffff;
    static const uint32_t ACoarseShift            = 0;
    
    static const uint32_t BCoarseMask             = 0xffff0000;
    static const uint32_t BCoarseShift            = 16;
  };
  //! Multibuffer/timeout setup is in the TransferSetup Register.
#ifndef FLATTEN_NESTED_CLASSES
  class TransferSetupRegister {
#endif
  public:
    static const uint32_t multiBufferCountMask   = 0xff;
    static const uint32_t multiBufferCountShift  = 0;
    
    static const uint32_t timeoutMask            = 0xf00;
    static const uint32_t timeoutShift           = 8;
#ifndef FLATTEN_NESTED_CLASSES    
  };
#endif

private:
  void checkExecuteError(int status);
};


// These functions are needed for the Swig wrappers:


inline size_t usb_device_vector_size(std::vector<struct usb_device*> devices) {
  return devices.size();
}

inline struct usb_device* usb_device_vector_get(std::vector<struct usb_device*> devices, int index) {
  return devices[index];
}
inline const char* string_to_char(std::string s) {
  return s.c_str();
}

inline int getuint16(uint16_t value) {
  return value;
}

inline size_t uint16_vector_size(std::vector<uint16_t> vec)
{
  return vec.size();
}
inline int uint16_vector_get(std::vector<uint16_t>vec, int i) {
  return vec[i];
}

#endif
