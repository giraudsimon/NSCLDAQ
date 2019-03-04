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

// Implementation of the CAD811 Ortec AD811 module for the CC-USB DAQ


#include <config.h>
#include "CC1205.h"

#include "CReadoutModule.h"
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <string>
#include <set>

#include <iostream>

using namespace std;


/************************************************************************/
/*                        Local constants                               */
/************************************************************************/
// Configuration value constants and limits:

static XXUSB::CConfigurableObject::limit One(1); 
static XXUSB::CConfigurableObject::limit Zero(0);
static XXUSB::CConfigurableObject::limit FULL16(0xffff);
static XXUSB::CConfigurableObject::limit LastSlot(23);

static XXUSB::CConfigurableObject::Limits SlotLimits(One, LastSlot); // CAMAC crate.


//
//  Threshold and resolution limits for list parameters:

static XXUSB::CConfigurableObject::limit  FourK(0xfff);
static XXUSB::CConfigurableObject::Limits ThreshLimits(Zero, FourK);
static XXUSB::CConfigurableObject::limit Loped(0);
static XXUSB::CConfigurableObject::limit Hiped(4095);
static XXUSB::CConfigurableObject::Limits PedLimits(Loped, Hiped);

static XXUSB::CConfigurableObject::limit ChannelCount(16);
static XXUSB::CConfigurableObject::ListSizeConstraint ChannelListSize = {ChannelCount,
							          ChannelCount};
static XXUSB::CConfigurableObject::TypeCheckInfo ThresholdOk(XXUSB::CConfigurableObject::isInteger,
						      &ThreshLimits);
static XXUSB::CConfigurableObject::TypeCheckInfo PedestalOk(XXUSB::CConfigurableObject::isInteger,
						     &PedLimits);

static XXUSB::CConfigurableObject::isListParameter threshValidity = {ChannelListSize,
							      ThresholdOk};

static XXUSB::CConfigurableObject::isListParameter pedestalValidity = {ChannelListSize,
								PedestalOk};
								



static XXUSB::CConfigurableObject::limit FULL8(0xff);
static XXUSB::CConfigurableObject::Limits IdLimits(Zero, FULL8);


// The array  below contain the possible rangemode enum values, and their
// corresponding values  Termination is with a null pointer for the string.

struct enumValues {
  const char*   s_pName;
  uint32_t      s_value;
};

struct enumValues RangeModes[] = {
  {"all",    0x000},
  {"auto",   0x200},
  {"sparse", 0x600},
  {0, 0}
};

// The names  get dumped into the set below:

static set<string> rangeValues;

// Some control register bits we use:

const  uint32_t DCCalibration  = 0x00100;
const  uint32_t ENABLESlider   = 0x00800;
const  uint32_t ENABLEPedestal = 0x01000;
const  uint32_t OVFIFNonZero   = 0x02000;
const  uint32_t DIAGNOSTIC     = 0x04000;
const  uint32_t BLOCKMODE      = 0x08000;
const  uint32_t LORESMode      = 0x10000;
const  uint32_t LAMHysteresis  = 0x20000;

/*!
   Construction is largely a no-op because the configuration is
   hooked to this modules at attachment time.

*/
CC1205::CC1205()
{
  m_pConfiguration = 0;
}

/*!
   Copy construction will copy the configuration and its values,
   if they have been produces in the rhs.
*/
CC1205::CC1205(const CC1205& rhs)
{
  m_pConfiguration = 0;
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }

}
/*!
  Assignment should probably have been made illegal.  We'll keep our
  current idea of the configuration.
  \param rhs  - The module that is being assigned to us.
  \return CAD811&
  \retval *this
*/
CC1205&
CC1205::operator=(const CC1205& rhs)
{
  return *this;
}
/*!
  Destruction is a no-op as there are issues with deleteing the
  configuration that I don't want to face yet.  Since this
  this implies a small memory leak.

*/
CC1205::~CC1205()
{
}
/*************************************************************************/
/*  Implementing the CReadoutHardware Interface                          */
/*************************************************************************/


/*!
   This function is called when the module is attached to its configuration.
   It is responsible for defining the set of parameters that can be
   set, their types and validity checkers that enforce any value/type
   constraints on the configuration values, as well as parameter initial
   values.

   See the header file for a definition of the set of parameters, types
   and defaults that will be defined for this module.

   \param configuration - reference to the configuration object that
                          will be associated with this module.

*/
void
CC1205::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = & configuration;

  // Create the parameters and validations:

  configuration.addParameter("-slot", 
			     XXUSB::CConfigurableObject::isInteger,
			     &SlotLimits, "0");
  configuration.addParameter("-id",
			     XXUSB::CConfigurableObject::isInteger,
			     &IdLimits, "0");

  configuration.addParameter("-usepedestals",
			     XXUSB::CConfigurableObject::isBool,
			     NULL, "true");
  configuration.addParameter("-hires",
			     XXUSB::CConfigurableObject::isBool,
			     NULL, "true");

  // The threshold, lopedestals, midpedestals and hipedestals are arrays

  configuration.addParameter("-thresholds",
			     XXUSB::CConfigurableObject::isIntList,
			     &threshValidity,
			     "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");

  configuration.addParameter("-lopedestals",
			     XXUSB::CConfigurableObject::isIntList,
			     &pedestalValidity,
			     "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
  configuration.addParameter("-midpedestals",
			     XXUSB::CConfigurableObject::isIntList,
			     &pedestalValidity,
			      "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");
  configuration.addParameter("-hipedestals",
			     XXUSB::CConfigurableObject::isIntList,
			     &pedestalValidity,
			     "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0");


  // -rangemode is an enum and needs a bit of attention to set up correctly.
  
  // If the valid values have not yet been set, set them up:

  if(rangeValues.empty()) {
    for(const enumValues* pRange = RangeModes; pRange->s_pName != 0; pRange++) {
      string rangeName(pRange->s_pName);
      rangeValues.insert(rangeName);
    }
  }
  // Now define the enum:

  configuration.addParameter("-rangemode",
			     XXUSB::CConfigurableObject::isEnum,
			     &rangeValues,
			     "auto");
			     
  
}
/*!
   Just prior to data taking, the module is initialized in accordance
   with its configuration. 

   \param controller  a reference to a CCCUSB& controller object that
                      is connected to the CCUSB we are using.


*/
void
CC1205::Initialize(CCCUSB& controller)
{

  // Fetch all of the configuration parameters:

  vector<uint16_t>  thresholds;
  vector<uint16_t>  lowpedestals;
  vector<uint16_t>  midpedestals;
  vector<uint16_t>  hipedestals;

  int      slot  =  getIntegerParameter("-slot");
  int      creg  =  configurationRegister(); // A few params conspire to make this.
  
  getArray("-thresholds",   thresholds);
  getArray("-lopedestals",  lowpedestals);
  getArray("-midpedestals", midpedestals);
  getArray("-hipedestals",  hipedestals);

  // Now program the module for initialization.

  uint16_t xq;
  checkedControl(controller, slot, 1, 24,
		 "CC1205 is evidently not plugged into the correct slot",
		 false);	// Turn off the gates.

  controller.simpleWrite24(slot, 1, 16, creg, xq); // Control register.
  
  // The thresholds:

  for (int i =0; i < 16; i++) {
    controller.simpleWrite16(slot, i, 17, thresholds[i], xq);
  }
  // Low pedestals: 

  for (int i=0; i < 16; i++) {
    controller.simpleWrite16(slot, i, 18, lowpedestals[i], xq);
  }
 
  // Mid pedestals:

  for (int i=0; i < 16; i++) {
    controller.simpleWrite16(slot, i , 19, midpedestals[i], xq);
  }
  // High range pedestals:

  for (int i =0; i < 16; i++) {
    controller.simpleWrite16(slot, i, 20, hipedestals[i], xq);
  }

  // Clear data buffers, Re-nable the gates:

  controller.simpleControl(slot, 1, 9, xq);
  controller.simpleControl(slot, 1, 26, xq);

}
/*!
   Called to setup the readout list.  In our case, this is simple:
   - insert the id as the marker in the event.
   - Use a q-stop to read at most 50 data words.  This will ensure the Q-stop 
     terminates with a q = 0 and a terminator word (the module is unconditionally
     not in block mode.

   @param list  - The CCUSBReadout list we are adding our reads to.
*/
void
CC1205::addReadoutList(CCCUSBReadoutList& list)
{
  int slot   = getIntegerParameter("-slot");
  int vsn    = getIntegerParameter("-id");

  // add the marker:

  list.addMarker(vsn);

  // Read the module:

  list.addQStop24(slot, 0, 0, 50);

  // The CC-usb will not transfer the trailer word because it is the one
  // that removes the Q rather than being the last one with Q valid.
  // the next list item adds a 'fake' trailer word.

  list.addMarker(0x00ff);	// Low 16 bits.
  list.addMarker(0x0040);	// Top 16 bits.
  
}


/*!

   Clone ourself.  This is essentially a virtual contstructor.
   We just do a new *this

*/
CReadoutHardware*
CC1205::clone() const
{
  return new CC1205(*this);
}

/////////////////////////////////////////////////////////////////////////////
//
//                   Utility functions.


/*
 *   Determine the correct range mode in terms of bits in the
 *   control register.  The rangemode enum is gotten and the
 *   string looked up in the rang-emodes table.  An exception
 *   is tossed if the range mode does not have a match in the table.
 */
uint32_t
CC1205::rangeMode()
{
  string rangeValue = m_pConfiguration->cget("-rangemode");
  for (enumValues* p = RangeModes; p->s_pName != 0; p++) {
    if (rangeValue == string(p->s_pName)) {
      return p->s_value;
    }
  }
  string exception = "C1205 -rangemode parameter value: ";
  exception       += rangeValue;
  exception       += " does not translate to a known register value in module ";

  throw exception;
}
/*
 * Use the module configuration parameters to return the correct valud for the
 * configuration register.
 */
uint32_t
CC1205::configurationRegister()
{
  // Set the hard coded bits:

  uint32_t reg   = DCCalibration | 
                   ENABLESlider;

  reg |= rangeMode();		// Set the operating mode bits.
  
  if (getBoolParameter("-usepedestals")) {
    reg |= ENABLEPedestal;
  }

  if (!getBoolParameter("-hires")) {
    reg |= LORESMode;
  }

  // Fold in the bottom 8 bits of the -id:

  uint32_t id = getIntegerParameter("-id");
  reg        |= (id & 0xff);

  cout << hex << " Control register: " << reg << endl << dec;

  return reg;
                   
}
