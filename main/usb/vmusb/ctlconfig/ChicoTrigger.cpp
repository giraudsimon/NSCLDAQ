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

#include <config.h>
#include "ChicoTrigger.h"
#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

#include <TCLInterpreter.h>
#include <TCLObject.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <iostream>

using namespace std;

// Trigger module registr definitions:

#define Const(name) static const uint32_t name = 

static const uint8_t  am (CVMUSBReadoutList::a32UserData); // Address modifier.

Const(ShortWidth) 0x1010;
Const(LongWidth)  0x1014;
Const(EnableMask) 0x1018;
Const(Control)    0x101c;

/**
 * Construction pretty much does nothing
 * we are stateless.
 */
ChicoTrigger::ChicoTrigger() :
  CControlHardware(),
  m_pConfiguration(0)
{
}
/**
 * Copy construction just copies and clones us;
 */
ChicoTrigger::ChicoTrigger(const ChicoTrigger& rhs) :
  CControlHardware(rhs)
{
}
/**
 * Destructor
 */
ChicoTrigger::~ChicoTrigger()
{
}
/**
 * Assignment works like copy construction:
 *
 * @param rhs - source of assignment.
 *
 * @return ChicoTrigger&
 * @retval Referece to *this
 */
ChicoTrigger&
ChicoTrigger::operator=(const ChicoTrigger& rhs)
{
  if (this != &rhs) {
    CControlHardware::operator=(rhs);
  }
  return *this;
}
/**
 * equality holds if the two configurations match:
 * 
 * @param rhs - object we're comparing *this to.
 * 
 * @return int
 * @retval 1 - equality.
 * @retval 0 - inequality.
 */
int
ChicoTrigger::operator==(const ChicoTrigger& rhs) const
{
  return CControlHardware::operator==(rhs);
}
/**
 *  Inequality is also based on inequality
 *  of the configs
 *
 * @param rhs - Object we're being compared with
 * 
 * @retval 1 - inEquality
 * @retval 0 - not unequal.
 */
int
ChicoTrigger::operator!=(const ChicoTrigger& rhs) const
{
  return CControlHardware::operator!=(rhs);
}

//////////////////////////////////////////////////////

/**
 * Invoked when the module is attached  to its cofiguration.
 * We only have the base configuration option.
 * - Save the configuration.
 * - register the -base option.
 */
void
ChicoTrigger::onAttach(CControlModule& configuration)
{
  m_pConfiguration = &configuration;
  configuration.addParameter("-base", 
			     XXUSB::CConfigurableObject::isInteger,
			     NULL, string("0"));
}
/**
 * Initialization is not required:
 */
void
ChicoTrigger::Initialize(CVMUSB& vme)
{
}
/**
 * Update is a no-op as well.
 */
string
ChicoTrigger::Update(CVMUSB& vme)
{
  return "ERROR - Update is not implemented for ChicoTrigger";
}
/**
 * Set a parameter value. Since I'm lazy and since
 * the two widths are coupled we only support 
 * the parameter 'all' which then is followed by
 * a Tcl List that contains in order
 * {short_width long_width mask control_register}
 *
 * @param vme - VM-USB object proxy.
 * @param string parameter - must be "all"
 * @param string value     - must be the list described
 *                           above.
 * @return string
 * @retval "OK" if successful
 * @retval ERROR - error description text
 */
string
ChicoTrigger::Set(CVMUSB& vme, string parameter, string value)
{
  // Validate the parameter:

  if (parameter != "all") {
    return "ERROR - no such parameter";
  }
  // The value string is a tcl list so we can use those
  // things to unravel them:


  CTCLInterpreter interp;
  CTCLObject      valueList;
  valueList.Bind(interp);
  valueList = value;

  // Interpreting the string as a list could
  // cause errors so wrap this in a try /catch block.

  try {
    if(valueList.llength() != 4) {
      return "ERROR - Parameter list has wrong number of values";
    }
    // Use a list of VME operations to set the module:
    
    CVMUSBReadoutList l;
    l.addWrite32(base() + ShortWidth, am,
		 (uint32_t)(int)valueList.lindex(0));
    l.addWrite32(base() + LongWidth, am,
		 (uint32_t)(int)valueList.lindex(1));
    l.addWrite32(base() + EnableMask, am,
		 (uint32_t)(int)valueList.lindex(2));
    l.addWrite32(base() + Control, am,
		 (uint32_t)(int)valueList.lindex(3));

    // execute the list:

    size_t  buffer;
    int status = vme.executeList(l, &buffer,
				 sizeof(buffer),
				 &buffer);
    if (status < 0) {
      return "ERROR - VME operation failed";
    }
    return "OK";
  }
  catch(...) {
    return "ERROR - unable to process parameter value as list";
  }
  
}
/**
 * Get a parametr value.  In this case, the only parameter is 'all'
 * which returns a Tc llist that contains in order
 * {short_width long_width mask control_register}}
 *
 * @param vme   VM-USB controller object.
 * @param parameter - The parameter we are told to get, must be "all"
 * 
 * @return string
 * @retval OK {short long mask control} if OK
 * @retval ERROR - reasonfor the error if a problem.
 *
 */
string
ChicoTrigger::Get(CVMUSB& vme, string parameter)
{
  // Ensure the right paramter is being requested.

  if (parameter != "all") {
    return "ERROR - no such parameter";

  }
  // build up a VMUSB list to read the register values:
  //

  CVMUSBReadoutList l;
  l.addRead32(base() + ShortWidth, am);
  l.addRead32(base() + LongWidth, am);
  l.addRead32(base() + EnableMask, am);
  l.addRead32(base() + Control, am);

  // Execute the list:

  uint32_t buffer[4];		// Expecting to read 4 longs.
  size_t   actuallyRead;

  int status = vme.executeList(l, buffer, sizeof(buffer), &actuallyRead);

  if (status <  0) {
    return "ERROR - VME operation failed";
  }

  if (actuallyRead != sizeof(buffer)) {
    return "ERROR - Expected read count incorrect";
  }
  // Build up ther reply.   The list is just simple integers so string concatenation
  // is fine as a way to build it up.

  string reply = "OK {";
  for (int i =0; i < 4; i++) {
    char value[100];
    sprintf(value, "%d", buffer[i]);
    reply += value;
    reply += " ";
  }
  reply += "}";
  return reply;
}
/**
 * Create a new copy of the rhs into this.
 */
CControlHardware*
ChicoTrigger::clone() const
{
  return (new ChicoTrigger(*this));
}
/**
 * Return the base address.
 */
uint32_t 
ChicoTrigger::base()
{

  uint32_t b =  m_pConfiguration->getUnsignedParameter("-base");

  return b;
}
