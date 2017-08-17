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

#ifndef __CV6533_H
#define __CV6533_H

#ifndef __CCONTROLHARDWARE_H
#include "CControlHardware.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif


#include <CControlModule.h>
class CVMUSB;
class CVMUSBReadoutList;
/**
 *  Controls the CAEN V 6533 6 channel HV module.  Parameters are:
 *   - globalmaxv    - Board maximum voltage
 *   - globalmaxI    - Board maximum current.
 *   - vn            - requested voltage for channel n.
 *   - in            - requested current max for channel n.
 *   - onn           - On/Off for channel n
 *   - vactn         - Actual voltage for channel n.
 *   - iactn         - Actual current for channel n
 *   - statusn       - Read only - get status for channel n.
 *   - ttripn        - Trip  time for channel n.
 *   - svmaxn        - Voltage max for channel n.
 *   - rdownn        - Ramp down rate for channel n.
 *   - rupn          - Ramp up rate for channel n.
 *   - pdownmoden    - Set the power down mode for channel n (ramp or kill).
 *   - polarityn     - Polarity for channel n.
 *   - tempn         - Temperature of channel n.
 */

class CV6533 : public CControlHardware
{
private:
  CControlModule*    m_pConfiguration;

  // The following registers are read via our periodic monitor list:

  uint16_t          m_globalStatus;
  uint16_t          m_channelStatus[6];
  uint16_t          m_voltages[6];
  uint16_t          m_currents[6];
  uint16_t          m_temperatures[6];

  // Canonical methods:
public:
  CV6533();
  CV6533(const CV6533& rhs);
  virtual ~CV6533();

  CV6533& operator=(const CV6533& rhs);
  int operator==(const CV6533& rhs) const;
  int operator!=(const CV6533& rhs) const;

  // Overrides:

  virtual void onAttach(CControlModule& configuration) ;  //!< Create config.
  virtual void Initialize(CVMUSB& vme);	                     //!< Optional initialization.
  virtual std::string Update(CVMUSB& vme) ;               //!< Update module.
  virtual std::string Set(CVMUSB& vme, 
			  std::string parameter, 
			  std::string value) ;            //!< Set parameter value
  virtual std::string Get(CVMUSB& vme, 
			  std::string parameter) ;        //!< Get parameter value.
  virtual CControlHardware*clone() const;	     //!< Virtual copy constr.

  // Interface to support monitoring.

  virtual void addMonitorList(CVMUSBReadoutList& vmeList);     //!< add items to the monitor list.
  virtual void* processMonitorList(void* pData, size_t remaining);  

  virtual std::string getMonitoredData();

  // Local utilities: 

private:
  void copy(const CV6533& rhs);	   // copy logic 
  uint32_t getBase();		// Get the base address of the module from config

  // Functions used to set the module parameters.

  void turnOff(CVMUSBReadoutList& list, unsigned int channel);
  void setRequestVoltage(CVMUSBReadoutList& list, unsigned int channel, float value);
  void setRequestCurrent(CVMUSBReadoutList& list, unsigned int channel, float value);
  void setChannelOnOff(CVMUSBReadoutList& list, unsigned int channel, bool value);
  void setTripTime(CVMUSBReadoutList& list, unsigned int channel, float time);
  void setMaxVoltage(CVMUSBReadoutList& list, unsigned int channel, float value);
  void setRampDownRate(CVMUSBReadoutList& list, unsigned int channel, float rate);
  void setRampUpRate(CVMUSBReadoutList& list, unsigned int channel, float rate);
  void setPowerDownMode(CVMUSBReadoutList& list, unsigned channel, std::string mode);

  // functions used to read module parameters.

  std::string getGlobalMaxV(CVMUSB& vme);
  std::string getGlobalMaxI(CVMUSB& vme);
  std::string getChannelVoltages(CVMUSB& vme);
  std::string getChannelCurrents(CVMUSB& vme);
  std::string getOnOffRequests(CVMUSB& vme);
  std::string getActualVoltages(CVMUSB& vme);
  std::string getActualCurrents(CVMUSB& vme);
  std::string getChannelStatuses(CVMUSB& vme);
  std::string getTripTimes(CVMUSB& vme);
  std::string getSoftwareVmax(CVMUSB& vme);
  std::string getRampDownRates(CVMUSB& vme);
  std::string getRampUpRates(CVMUSB& vme);
  std::string getPowerdownModes(CVMUSB& vme);
  std::string getTemperatures(CVMUSB& vme);
  std::string getPolarities(CVMUSB& vme);

  bool strToBool(std::string value);
  std::string fToString(float value);
  std::string scaledIToString(uint16_t* values, float scaleFactor);


};


#endif
