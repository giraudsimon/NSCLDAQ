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
#include "CCAMACModule.h"
#include "CCAMACBranch.h"


/*!
   Construction is just a matter of filling in the blanks:
   \param pBranch - Pointer to an already constructed camac branch object
                    all dataway operations get delegated to that object.
   \param crate   - CAMAC crate in the branch in which this module lives.
   \param slot    - Slot in the CAMAC crate in which the module lives.


*/
CCAMACModule::CCAMACModule(CCAMACBranch* pBranch, unsigned crate, unsigned slot) :
  m_crate(crate),
  m_slot(slot),
  m_pBranch(pBranch)
{}

/*!
   Do a read.. this delegates to the branch;  The read is a 24 bit read.. only the
   lower 24 bits of the returned data are meaningful.
   \param subaddress - the module subaddress at which the read is directed.
   \param function   - the read function.
*/
uint32_t
CCAMACModule::read(unsigned subaddress, unsigned function)
{
  return m_pBranch->read(m_crate, m_slot, subaddress, function);
}
/*!
   Do a 16 bit read see above
*/
uint16_t
CCAMACModule::read16(unsigned subaddress, unsigned function)
{
  return m_pBranch->read16(m_crate, m_slot, subaddress, function);
}
/*!
  Do a 24 bit write.  This delegates to the branch.  
  \param subaddress - the module subaddress at which the write goes.
  \param function   - The function to perform.
  \param data       - data to write to the module.
*/
void
CCAMACModule::write(unsigned subaddress, unsigned function, uint32_t data)
{
  m_pBranch->write(m_crate, m_slot, subaddress, function, data);
}
/*!
   Do a 16 bit write, see above.
*/
void
CCAMACModule::write16(unsigned subaddress, unsigned function, uint16_t data)
{
  m_pBranch->write16(m_crate, m_slot, subaddress, function, data);
}
/*!
   Perform a non data transfer operation.  This too is delegated to the 
   branch object.
   \param subaddresss - module subaddress.
   \param function    - control function.
*/
void
CCAMACModule::control(unsigned subaddress, unsigned function)
{
  m_pBranch->control(m_crate, m_slot, subaddress, function);
}

/*!
   Determine if the last operation on the branch this module is in
   produced a q response.  This is delegated to the branch object too.
*/
bool
CCAMACModule::q()
{
  return m_pBranch->qtest();
}
/*!
  Determine if the last module on the branch this module is in
  produced an X response.
*/
bool
CCAMACModule::x()
{
  return m_pBranch->xtest();
}
