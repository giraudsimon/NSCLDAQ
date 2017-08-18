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

#ifndef __CREADOUTMODULE_H
#define __CREADOUTMODULE_H

#ifndef __CCONFIGURABLEOBJECT_h
#include "XXUSBConfigurableObject.h"
#endif

// Forward classes:

class CReadoutHardware;
class CVMUSB;
class CVMUSBReadoutList;

/*!
   CReadoutModule objects are specialized versions of CConfigurable objects.
   What specializes them is that:
   - They contain a CReadoutHardware object which represents an object that will
     require readout in response to a trigger.
   - They provide a facade to their CReadoutHardware objects for initialization, and
     to setup the readout list.
   - On construction, they invoke the onAttach member of their CReadoutHardware object
     so that object can define the configuration parameters it needs.

   \note these objects are fully copyable, as the hardware objects
         are suposed to also be copyable.
*/
class CReadoutModule : public XXUSB::CConfigurableObject
{
  CReadoutHardware* m_pHardware;	        //!< The actual hardware object.
public:
  CReadoutModule(std::string name, const CReadoutHardware& hardware);
  CReadoutModule(const CReadoutModule& rhs);
  virtual ~CReadoutModule();
  CReadoutModule& operator=(const CReadoutModule& rhs);
private:
  int operator==(const CReadoutModule& rhs) const;
  int operator!=(const CReadoutModule& rhs) const;

public:
  CReadoutHardware* getHardwarePointer();
  CReadoutHardware* getHardwareCopy() const;
  

public:
  void Initialize(CVMUSB& controller);
  void addReadoutList(CVMUSBReadoutList& list);
};


#endif
