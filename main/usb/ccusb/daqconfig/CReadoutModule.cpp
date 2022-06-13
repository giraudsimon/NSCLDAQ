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
#include "CReadoutModule.h"
#include "CReadoutHardware.h"

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

using namespace std;



///////////////////////////////////////////////////////////////////////////
//////////////////// Constructors and other Canonicals ////////////////////
///////////////////////////////////////////////////////////////////////////

/*!
    Constructing is just a matter of saving the hardware reference
    and invoking its onAttach member.
    \param name     : std::string
        Name of the module.
    \param hardware : CReadoutHardware& 
        Reference to a readout hardware object that will be 
        copy constructed to form our hardware.  This copy construction
        avoids scope/ownership/deletion issues... we own it and we 
        \em will delete it on destruction.
*/
CReadoutModule::CReadoutModule(string name, const CReadoutHardware& hardware) :
  XXUSB::CConfigurableObject(name),
  m_pHardware(hardware.clone())
{
  m_pHardware->onAttach(*this);
}
/*!
   Copy construction.. just need to 
   - copy construct a new hardware object.
   - invoke its onAttach member.
*/
CReadoutModule::CReadoutModule(const CReadoutModule& rhs) :
  XXUSB::CConfigurableObject(rhs.getName()),
  m_pHardware(rhs.m_pHardware->clone())
{
  m_pHardware->onAttach(*this);
}
/*!
   Destruction: Just destroy our hardware object
*/
CReadoutModule::~CReadoutModule()
{
  delete m_pHardware;
}
/*!
   Assignment is much like copy construction.
*/
CReadoutModule&
CReadoutModule::operator=(const CReadoutModule& rhs)
{
  if (this != &rhs) {
    delete m_pHardware;
    XXUSB::CConfigurableObject::operator=(rhs);
    m_pHardware = rhs.m_pHardware->clone();
    clearConfiguration();
    m_pHardware->onAttach(*this); // Re-setup the configuration.

  }
  return *this;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////   Selectors   /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/*!
   Return a pointer to the actual hardware
*/
CReadoutHardware*
CReadoutModule::getHardwarePointer()
{
  return m_pHardware;
}

/*!
   Return a pointer to a dynamcially allocated copy of our hardware.
   The caller is responsible for deleting this when done.
*/
CReadoutHardware*
CReadoutModule::getHardwareCopy() const
{
  return m_pHardware->clone();
}

////////////////////////////////////////////////////////////////////////////
//////////////////// Operations on the object //////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
   Initialize the module. This is supposed to make the hardware object
   ready to take data.   All that has to be done is to delegate this work
   to the object itself.  This is part of the facade that the CReadoutModule
   object maintains for its CReadoutModule object.
   \param controller : CCCUSB&
      A VMUSB controller that is connected to the VME bus in which the
      hardware resides.
*/
void
CReadoutModule::Initialize(CCCUSB& controller) 
{
  m_pHardware->Initialize(controller);
}
/*!
   Add the module's event readout requirements to the readout list.
   this is part of the facade that CReadoutModule's maintain for their
   CReadoutHardware objects. 
   \param list : CCCUSBReadoutList&
        Reference to the list object that is being built up to create the
	readout list that will be loaded into the VM USB.
*/
void
CReadoutModule::addReadoutList(CCCUSBReadoutList& list)
{
  m_pHardware->addReadoutList(list);
}
