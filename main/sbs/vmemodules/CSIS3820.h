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
#ifndef CSIS3820_H
#define CSIS3820_H

#include "CModule32.h"
#include <string>
#include <VmeModule.h>
#include <stdint.h>

/*!
   Controls an SIS3820 module.  The SIS3820 is a 32 channel latching
   VME scaler module.
*/
class CSIS3820 : public CModule32
{
public:
  // datatypes.

  
  enum OperatingMode {		//!< Scaler operating mode.
    LatchingScaler     =  0x00000000, //!< Latching scaler.
    ReservedScaler1    =  0x10000000,
    MultiChannelScaler =  0x20000000, //!< Multichannel scaler.
    HistogrammingScaler=  0x30000000, //!< Histogramming scaler.
    ReservedScaler2    =  0x40000000,
    ReservedScaler3    =  0x50000000,
    ReservedScaler4    =  0x60000000,
    RamTestScaler      =  0x70000000 //!< Test FIFO and ram.
  };
  
  /*!  Module input modes. See setInputMode for a full
       description of these bits.
  */
  enum InputMode {
    NoInputs                     = 0x0000000,
    InputLatchInhibitLatch       = 0x0010000,
    InputLatchInhibitAllAndLatch = 0x0020000,
    InputLatchInhibitAll         = 0x0030000,
    InhibitGroups                = 0x0040000,
    InputLNEHiscal               = 0x0050000, // FW 1.1 and above.
    InputLNEInhAllExtClear       = 0x0060000, // FW 1.5 and above. 
    InputReserved7               = 0x0070000 

  };

  /*!
    Defines the legal latch sources for the scaler.
    Note that not all of them have software support.
    see the description of setLatchSource
    for more information.
   */
  enum LNESource {
    LatchVMEOnly                = 0x00000000,
    LatchFP                     = 0x00000010,
    Latch10Mhz                  = 0x00000020,
    LatchChannelN               = 0x00000030,
    LatchPresetN                = 0x00000040,
    LatchReserved5              = 0x00000050,
    LatchReserved6              = 0x00000060,
    LatchReserved7              = 0x00000070
  };

private:

public:
  // Constructors:

  CSIS3820(unsigned long base, int crate = 0) ;
  virtual ~CSIS3820() {}

  // Copy, etc. not supported.
private:
  CSIS3820(const CSIS3820&);
  CSIS3820& operator=(const CSIS3820&);
  int       operator==(const CSIS3820&);
  int       operator!=(const CSIS3820&);
public:

  // setup functions:


  double getRevision() const;	//!< Get firmware revision.
  void   LightOn()     const;	//!< Turn on the user led.
  void   LightOff()    const;   //!< Turn off the user led.
  uint32_t
         getCsr()      const;   //!< Return value of CSR

  uint32_t getAcqMode()  const;   /*!< Return value of the ACQ
				  mode register. */

  void   Reset()      const;   //!< Reset the module.

  void   setOperatingMode(OperatingMode mode) const ;
  OperatingMode  getOperatingMode()           const;

  void      setInputMode(InputMode mode) const ;
  InputMode getInputMode() const;

  void setLatchSource(LNESource mode) const ;
  LNESource getLatchSource() const;

  void Arm()     const;		//!< Arm the module.
  bool isArmed() const;		//!< true if module is armed..

  void Enable()    const;       //!< Enable the module...
  void Disable()   const;       //!< Disable the mdoule...
  bool isEnabled() const;	//!< true if module is enabled.

  void EnableReferencePulser() const;  //!< Enable the ref pulser.
  void DisableReferencePulser() const; //!< Turn off ref pulser.
  bool isReferencePulserEnabled() const; //!< Test state of ref. pulser

  void EnableTestCounter() const;    //!< Turn on 25mhz test pulser.
  void DisableTestCounter() const;   //!< Turn off 25mhz test pulser.
  bool isTestCounterOn() const;	     //!< is test pulser on?? 

  void ClearChannels() const;	     //!< Clear all scaler chans.


  void DisableClearOnLatch() const;   //!< Turn off clear on latch.
  void EnableClearOnLatch()  const;   //!< Turn on clear on latch.

  // Read functions:

  uint32_t      ReadChannel(unsigned int num) const; //!< read live channel.
  void          ReadAllChannels(uint32_t* buffer) const; 
  uint32_t      ReadHighBits();

  void Latch() const;		                    //!< Latch scalers.
  uint32_t      ReadLatchedChannel(unsigned int num) const;
  void          ReadAllLatchedChannels(uint32_t* buffer) const;
  void          LatchAndRead(uint32_t* buffer) const;

  // Utility functions:

protected:
  uint32_t  getModuleInfo() const;
  void setAcqMode(unsigned long data) const; //!< Set acq mode reg.

  void setCSR(unsigned int long data) const;
  static void ValidChannel(unsigned int chan, const char *pWhere);
  static uint32_t  ChannelOffset(uint32_t base, 
				 uint32_t chan);
};


#endif
