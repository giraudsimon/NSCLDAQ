/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CMarkerObject.cpp
# @brief  Implement the marker object.
# @author <fox@nscl.msu.edu>
*/


#include "CMarkerObject.h"
#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"

#include <stdint.h>

/**
 * constructor
 * @param name - Name of the object.
 */
CMarkerObject::CMarkerObject() :
    CControlHardware(),
    m_pConfiguration(0)
    {}

/**
 * copy construction
 *
 * @param rhs - what we are copying to construct.
 */
CMarkerObject::CMarkerObject(const CMarkerObject& rhs) :
    CControlHardware(rhs)
{
  onAttach(*new CControlModule(rhs.m_pConfiguration->getName(), this));

}
/**
 * destructor
 */
CMarkerObject::~CMarkerObject()
{

}
/**
 * assignment
 */
CMarkerObject&
CMarkerObject::operator=(const CMarkerObject& rhs)
{
 if (this != &rhs) {
   CControlHardware::operator=(rhs);
    copyIn(rhs);
  }
  return *this;    
}

/**
 * equality compare
 */
int
CMarkerObject::operator==(const CMarkerObject& rhs) const
{
    return CControlHardware::operator==(rhs);
}
/**
 * inequality
 */
int
CMarkerObject::operator!=(const CMarkerObject& rhs) const
{
    return !(*this == rhs);
}
/*----------------------------------------------------------------------------
 * CControlHardware operations we need to implement
 */

/**
 * onAttach
 *   Save the configuration object and register the -value switch.
 * @param config - the configuration
 */
void
CMarkerObject::onAttach(CControlModule& config)
{
    m_pConfiguration = &config;

    m_pConfiguration->addIntegerParameter("-value", 0, 65535, 0);
}

/**
 * Update
 *    Always ok
 * @param vme - VMUSB rference.
 */
std::string
CMarkerObject::Update(CVMUSB& vme)
{
    return "OK";
}
/**
 * Set
 *    Always an error as there are no settable parameters.
 *
 *  @param vme    - reference to the VMUSB object.
 *  @param what   - parameter to set.
 *  @param to     - What to set the parameter to.
 *
 * @return std::string : "ERROR - No such parameter"
*/
std::string
CMarkerObject::Set(CVMUSB& vme, std::string what, std::string to)
{
    return "ERROR - No such parameter.";
}
/**
 * Get
 *   Always an error as there are no gettable parameters.
 *
 * @param vme    VMUSB controller object reference.
 * @param what   Parameter to get.
 * @return std::string "ERROR - No such parameter"
 */
std::string
CMarkerObject::Get(CVMUSB& vme, std::string what)
{
    return "ERROR - No such parameter.";
}
/**
 * addMonitorList
 *    Add a marker to the list that is executed for
 *    The value of the marker comes from the -value
 *    configuration parameter.
 * @param list - VMUSB readout list to add the marker to.
 */
void
CMarkerObject::addMonitorList(CVMUSBReadoutList& list)
{
    int value = m_pConfiguration->getIntegerParameter("-value");
    list.addMarker(static_cast<uint16_t>(value));
}
/**
 * processMonitorList
 *   Process our slice of the monitor list... just skip the
 *   marker.
 *
 *  @param pData     - Pointer to the marker word.
 *  @param remaining - Number of bytes remaining in the list.
 *  @return void*    - Pointer past the marker word.
 */
void*
CMarkerObject::processMonitorList(void* pData, size_t remaining)
{
    uint16_t* p = reinterpret_cast<uint16_t*>(pData);
    if (*p != m_pConfiguration->getIntegerParameter("-value")) {
        throw std::string("Marker value mismatch!!");
    }
    p++;
    return p;
}
/**
 * clone
 *   Virtual copy constructor.
 *
 *  @param rhs - Item we are trying to build a clone of.
 */
CControlHardware*
CMarkerObject::clone() const
{
  return new CMarkerObject(*this);

}
/**
 * copyIn
 *    Copy rhs (with appropriate duplication) to this.
 *
 */
void
CMarkerObject::copyIn(const CMarkerObject& rhs)
{
  onAttach(*(new CControlModule(rhs.m_pConfiguration->getName(), this)));
}
