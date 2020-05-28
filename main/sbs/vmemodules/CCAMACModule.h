#ifndef CCAMACMODULE_H
#define CCAMACMODULE_H

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

#include <stdint.h>


class CCAMACBranch;

/*!
   
  CCAMACModule is a generic module.  It can be used
  directly, or it can be used as a base class for a class that has members that
  are more application specific.  The class is independent of branch type
  because it delegates its dataway methods to a CCAMACBranch concrete class
  that understands the controller.

*/
class CCAMACModule 
{
private:
  unsigned      m_crate;
  unsigned      m_slot;
  CCAMACBranch* m_pBranch;
public:

  CCAMACModule(CCAMACBranch* pBranch, unsigned crate, unsigned slot);
  
  uint32_t read(unsigned subaddress, unsigned function);
  uint16_t read16(unsigned subaddress, unsigned function);

  void write(unsigned subaddress, unsigned function, uint32_t data);
  void write16(unsigned subaddress, unsigned function, uint16_t data);

  void control(unsigned subaddress, unsigned function);

  bool q();
  bool x();
};


#endif
