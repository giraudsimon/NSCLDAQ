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

#ifndef CSIS3300MODULE_H
#define CSIS3300MODULE_H

#include "CConfigurableObject.h"
#include <CDigitizerModule.h>

class CSIS3300;

/*!
   This class handles scripted configuration of an
   SIS3300/3301 flash adc.  Only the needed configuration
   items are handled.  These include:
   - base       int  base address of module.
   - crate      int  VME Crate number of module.
   - clock      enum {100Mhz 50Mhz 25Mhz 12.5Mhz 6.25Mhz 3.125Mhz frontpanel p2}
   - samplesize enum {128K 16K 4K 2K 1K 512 256 128}
   - startdelay int  (in ticks, 0 implies start delay disabled).
   - stopdelay  int  (in ticks, 0 implies stop delay disabled).
   - gate       bool (for gatemode).
   - wrap       bool (for enable wrap)
   - thresholds intarray[8]
   - lt          bool Thresholds are made if signal < value.
   - freerunning bool if true, CPU starts sampling in init and clear().
   - usegroups  int   Mask of groups to read one bit per group 2 chans/group.
   - id         int   Packet id ..defaults to 0x3300
   - subpacket  bool  Enable subpackets for each group read.  If enabled,
                      subpackets will have ids that are the group number (1,2,3,4).
*/
class CSIS3300Module : public CDigitizerModule
{
  CSIS3300*    m_pModule;
  int          m_samplesPerChannel;
  bool         m_freeRunning;	// cached value of freerunning param.
  int          m_groupsRead;    // mask of groups to read.
  int          m_id;            // Packet id cached.
  bool         m_subpackets;	// Cached subpacket config parameter.
public:
  CSIS3300Module(const std::string&    name,
		 CTCLInterpreter& interpreter);
  virtual ~CSIS3300Module();

private:
  CSIS3300Module(const CSIS3300Module& rhs);
  CSIS3300Module& operator=(const CSIS3300Module& rhs);
  int operator==(const CSIS3300Module& rhs) const;
  int operator!=(const CSIS3300Module& rhs) const;

public:
  virtual   void   Initialize (); //!< Initialize the readable (pure virt.).
  virtual   void   Prepare ();    //!< Prepare module for readout (pure virt.). 
  virtual   int    Read (void*  pBuffer); //!< Read to memory. (pure virt)
  virtual   void   Clear ();      //!< Clear (after read e.g.).(Pure virt) 
  virtual   std::string getType () const;    //!< Return module type std::string (pure virt).
private:
  int       getIntValue(std::string name);
  bool      getBoolValue(std::string name);
  int       getEnumValue(std::string name);
  SConfigurableObject::ParameterIterator AddConfigParam(CConfigurationParameter* param);

				  
};


#endif

