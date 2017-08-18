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

// Implementation of the C785 class VM-USB support for the CAEN V785.


#include <config.h>

#include "CMarker.h"

#include "CReadoutModule.h"
#include <CCCUSBReadoutList.h>
#include <stdlib.h>

using namespace std;


static XXUSB::CConfigurableObject::limit valueLow(0);
static XXUSB::CConfigurableObject::limit valueHigh(0xffff);
static XXUSB::CConfigurableObject::Limits valueLimits(valueLow, valueHigh);


///////////////////////////////////////////////////////////////////////////////////////
////////////////////////// Canonical class/object operations /////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

CMarker::CMarker() :
  m_pConfiguration(0)
{}

CMarker::CMarker(const CMarker& rhs) :
  m_pConfiguration(0)
{
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));
  }
}

CMarker::~CMarker() {}

CMarker&
CMarker::operator=(const CMarker& rhs)
{
  return *this;
}
/////////////////////////////////////////////////////////////////////////////////
//////////////////////// Overridable operations /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*!
    Attach the module to its configuration by  storing the config reference and
    adding the -value parameter to the config.
    \param CReadoutModule& configuration

*/
void
CMarker::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration;

  m_pConfiguration->addParameter(
    "-value", XXUSB::CConfigurableObject::isInteger, &valueLimits, "0"
  );

}
/*!
   The device does not need to be initialized.
*/
void 
CMarker::Initialize(CCCUSB& controller)
{}

/*!
    The module is added to the readout list by fetching the value parameter as a 16 bit integer
    and adding the marker command to the readout list.
*/
void
CMarker::addReadoutList(CCCUSBReadoutList& list)
{
  unsigned int value = getIntegerParameter("-value");

  list.addMarker(static_cast<uint16_t>(value));
}

/*!
  Virtual constructor:

*/
CReadoutHardware* 
CMarker::clone() const
{
  return new CMarker(*this);
}
/////////////////////////////////////////////////////////////////////
//////////////////// Private utility functions //////////////////////
/////////////////////////////////////////////////////////////////////

// Return the value of an integer parameter.
// Parameters:
//    std::string name - name of the parameter.
// Returns:
//    value
// Throws a string exception (from cget) if there is no such parameter.
// caller is responsible for ensuring the parameter is an int.
//
unsigned int
CMarker::getIntegerParameter(string name)
{
  string sValue =  m_pConfiguration->cget(name);
  unsigned int    value  = strtoul(sValue.c_str(), NULL, 0);

  return value;
}
