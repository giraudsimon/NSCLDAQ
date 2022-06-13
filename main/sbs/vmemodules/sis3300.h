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


#ifndef SIS3300_H
#define SIS3300_H




#include <string>
#include <stdint.h>

/*!
   Preliminary class to setup and read the SIS3300 flash adc.
   Supported functions are to:

   Initialize the module to work in external trigger:
   Start or stop mode, single event mode
   with specified Clock source and sample size, and pre/post trigger delays.

   This class does not support  ADC thresholds, or
   multievent mode  but they can be added later.

   Author: R. Fox
   Modified by: M.A. Famiano for use with HiRA
   Modification History: 
   - February 2, 2004: Accurately implemented random clock Mode so
   that internal clock is enabled while not sampling.RF.
   - February 2, 2004: Implemented addressing for independent ADCs.
   - January 31, 2004: Added Arming and disarming function for both memory banks.MAF.
   - January 31, 2004: Implementation for Random Clock Mode.MAF.
   - January 31, 2004: Implementation for HiRA Random Clock Mode.MAF.
   - November 2003: Added Stop Sampling functions and LED Controls for test purposes.
   MAF.
   */
class CSIS3300 {
public:
  typedef enum _ClockSource {
    Internal100Mhz = 0,
    Internal50Mhz  = 1,
    Internal25Mhz  = 2,
    Internal12_5Mhz= 3,
    Internal6_25Mhz= 4,
    Internal3_125Mhz=5,
    ExternalFp      =6,
    ExternalP2      =7 
  } ClockSource;		//!< the enum values match the bit field values
  typedef enum _SampleSize {
    Sample128K     = 0,
    Sample16K      = 1,
    Sample4K       = 2,
    Sample2K       = 3,
    Sample1K       = 4,
    Sample512      = 5,
    Sample256      = 6,
    Sample128      = 7,
  } SampleSize; 

private:
  uint32_t           m_nBase;            //!< Module abs. VME base
  // Register pointers:
  volatile uint32_t* m_pCsrs;            //!< CSR bank 0 - 0x3000.
  volatile uint32_t* m_pEi1;             //!< Event information for bank1.
  volatile uint32_t* m_pEi2;              //!< Event info for bank 2.
  volatile uint32_t* m_pEi3;             //!< Event info for bank 3.
  volatile uint32_t* m_pEi4;             //!< Event info for bank 4.

  volatile uint32_t* m_pModuleId;	      //!< Module ID register.
  volatile uint32_t* m_pCsr;	      //!< Control status register.  
  volatile uint32_t* m_pAcqReg;	     //!< Acquisition control register.
  volatile uint32_t* m_pResetKey;	      //!< Module reset key register.
  volatile uint32_t* m_pStart;	      //!< Start Key register.
  volatile uint32_t* m_pStop;	      //!< Stop Key register.
  volatile uint32_t* m_pEventConfig;    //!< Global Event config register.
  volatile uint32_t* m_pStartDelay;	//!< Start delay register.
  volatile uint32_t* m_pStopDelay;	      //!< Stop  delay register.
  volatile uint32_t* m_pAddressReg1; //!< Address register 1 (detect eov) 
  volatile uint32_t* m_pAddressReg2; //!< Address register 2 (detect eov) 
  volatile uint32_t* m_pAddressReg3; //!< Address register 3 (detect eov) 
  volatile uint32_t* m_pAddressReg4; //!< Address register 4 (detect eov) 
  volatile uint32_t* m_pEventDirectory1;   //!< Ptr to the event directory.
  volatile uint32_t* m_pEventDirectory2;   //!< Ptr to the event directory.
  volatile uint32_t* m_pEventDirectory3;   //!< Ptr to the event directory.
  volatile uint32_t* m_pEventDirectory4;   //!< Ptr to the event directory.
  volatile uint32_t* m_pBank1Buffers[4];   //!< Bank data memory.
  volatile uint32_t* m_pThresholds[4]; //!< threshold registers.

  // State which determines how the module is set up:
 
  void*         m_nFd;		//!< Fd open on the vme device driver.
  ClockSource m_eClock;		    //!< Source of sampling clock.
  bool        m_fStartDelayEnabled; //!< true if start delay register is used.
  unsigned int m_nStartDelayClocks; //!< clocks to load into start delay reg.
  bool        m_fStopDelayEnabled;  //!< true if stop delay register is used.
  unsigned int m_nStopDelayClocks;//!< clocks to load into stop delay register.

  bool       m_fGateMode;	 //! true if module in gate mode.
  bool       m_fRandomClock;     //! True if module is in Random Clock Mode.
  bool       m_fLemoStartStop;
  bool       m_fP2StartStop;

  bool       m_fHiRA_RCM;        //! True if in HiRA Random Clock Mode.
  bool       m_fStopTrigger;    //! True if stop is trigger.
  SampleSize m_ePagesize;	//!< Size of acquisition page (enum).
  unsigned int m_nPagesize;	//!< Size of acquisition page in samples
  bool       m_fPageWrap;	//!< True if allowing data to wrap the pages
  bool       m_fThresholdLt[8];	//!< True if chann  threshold is a < threshold
  unsigned int m_nThresholds[8];	//!< ADC channel threshold values.





public:
  // Constructors and other canonical member functions:

  CSIS3300(uint32_t nBaseAddress,
	   unsigned int nCrate = 0);
private:
  CSIS3300(const CSIS3300& rhs);
  CSIS3300& operator=(const CSIS3300& rhs);
  int      operator==(const CSIS3300& rhs);
public:
  ~CSIS3300();

  // Selectors:
public:
  uint32_t getBaseAddress() const {
    return m_nBase;
  }
  uint32_t getPageSize() const {
    return m_nPagesize;
  }

  volatile uint32_t* getAddressRegister1() {
    return m_pAddressReg1;
  }
   volatile uint32_t* getAddressRegister2() {
    return m_pAddressReg2;
  }
  volatile uint32_t* getAddressRegister3() {
    return m_pAddressReg3;
  }
  volatile uint32_t* getAddressRegister4() {
    return m_pAddressReg4;
  }
  /*! This function is deprecated in favor 
    of getStatusRegiseter() and setControlRegister()
     It exists only for 
   compatibility with old code where the
   entire module was mapped:
  */
  volatile uint32_t* getModuleBase() { 
    return m_pCsrs;
  }
  /*! This function is deprecated in favor 
    of getStatusRegiseter() and setControlRegister()
     It exists only for 
   compatibility with old code where the
   entire module was mapped:
  */
  volatile uint32_t* getCsr() {
    return m_pCsr;
  }
  //! Get value of status register.
  uint32_t getStatusRegister() const {  
    return *m_pCsrs;
  }
  //! Set the control register.
  void setControlRegister(uint32_t value) {
    *m_pCsrs = value;
  }
  /*!
    This function is deprecated in favor of:
    - getModelNumber() - retrieve the model number
    - getFirmwareMajor() - get major rev level.
    - getFirmwareMinor() - get minor rev level.
  */
  volatile const uint32_t* getModuleId() const{
    return m_pModuleId;
  }
  //! Get the model number:
  unsigned int getModelNumber() const {
    return (*m_pModuleId >> 16) & 0xffff;
  }
  //! Get the firmware major revlevel:
  unsigned int getFirmwareMajor() const {
    return (*m_pModuleId >> 8) & 0xff;
  }
  //! Get the firmware minor rev level:
  unsigned int getFirmwareMinor() const {
    return (*m_pModuleId) & 0xff;
  }

  //! Read access to the Acquisition register.
  volatile const uint32_t* getAcquisitionRegister() const {
    return m_pAcqReg;
  }
  /*!  This function is deprecated in favor of the
       Reset function
   */
  volatile uint32_t* getResetKeyRegister() {
    return m_pResetKey;
  }
  volatile uint32_t* getStartKeyRegister() {
    return m_pStart;
  }
  volatile uint32_t* getEventConfigRegister() {
    return m_pEventConfig;
  }
  volatile uint32_t* getStartDelayRegister() {
    return m_pStartDelay;
  }
  volatile uint32_t* getStopDelayRegister() {
    return m_pStopDelay;
  }
  volatile uint32_t* getEventDirectory1() {
    return m_pEventDirectory1;
  }

  volatile uint32_t* getEventDirectory2() {
    return m_pEventDirectory2;
  }
  volatile uint32_t* getEventDirectory3() {
    return m_pEventDirectory3;
  }
  volatile uint32_t* getEventDirectory4() {
    return m_pEventDirectory4;
  }
  void* getFd() {
    return m_nFd;
  }
  // These functions really only make sense inside the class body.
  // they return VME addresses not process virtual addresses.
private:
  volatile uint32_t getGroup1Address() {
    return getBaseAddress() + 0x400000;
  }
  volatile uint32_t getGroup2Address() {
    return getBaseAddress() + 0x480000;
  }
  volatile uint32_t getGroup3Address() {
    return getBaseAddress() + 0x500000;
  }
  volatile uint32_t getGroup4Address() {
    return getBaseAddress()+0x580000;
  }
public:

  // Configuration functions:
 
  void Reset();			//!<  Reset the module.
  void SetClock(ClockSource eSource);
  void SetStartDelay(bool Enable = false, unsigned int nClocks = 0);
  void SetStopDelay(bool Enable  = false, unsigned int nClocks = 0);
  void GateMode(bool Enable = false);
  void RandomClock(bool Enable = false);
  void LemoStartStop(bool Enable = false);
  void P2StartStop(bool   Enable = false);
  void HiRA_RCM(bool Enable = false);
  void TriggerOnStop(bool Enable=true);
  void SetSampleSize(SampleSize eSamples);
  void EnableWrap(bool Enable=true);
  unsigned int GetUserInput();

  void SetThresholds(bool* pLessThan,
		     unsigned int* pValues);

  // Configuration inquiry functions.

  CSIS3300::ClockSource  getCurrentClockSource();
  bool         isStartDelayEnabled();
  unsigned int getStartDelayClocks();
  bool         isStopDelayEnabled();
  unsigned int getStopDelayClocks();
  unsigned int getThresholdValue(unsigned int channel) const {
    return m_nThresholds[channel];
  }
  bool         isLtThreshold(unsigned int channel) const {
    return m_fThresholdLt[channel];
  }
  bool haveHiRAFirmware() const;


  //  Data taking functions. 
public:
  void LightOn();
  void LightOff();
  void InitDaq();
  void Arm1();
  void Arm2();
  void DisArm1();
  void DisArm2();
  void StartSampling();
  void StopSampling();
  void EnableUserOut();
  void DisableUserOut();
  void StrobeUserOut(int time);
  bool WaitUntilDone(int timeout);
  uint32_t EventNumber(int bank);

  // New members to do I/O to ordinary buffers.

  unsigned int ReadGroup1(void* pBuffer);
  unsigned int ReadGroup2(void* pBuffer);
  unsigned int ReadGroup3(void* pBuffer);
  unsigned int ReadGroup4(void* pBuffer);
  unsigned int ReadAllGroups(void * pBuffer);
 

  void ClearDaq();
protected:

  /// For reading to normal memory buffers.
  //
  unsigned int ReadAGroup(void* pbuffer,
			  volatile uint32_t* pAddressReg1,
			  uint32_t           pBase);
};



#endif
