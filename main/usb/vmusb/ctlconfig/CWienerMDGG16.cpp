//  This software is Copyright by the Board of Trustees of Michigan
//  State University (c) Copyright 2014.
//  
//  You may use this software under the terms of the GNU public license
//  (GPL).  The terms of this license are described at:
//  
//  http://www.gnu.org/licenses/gpl.txt
//  
//  Author:
//  Jeromy Tompkins
//  NSCL
//  Michigan State University
//  East Lansing, MI 48824-1321

#include <memory>
#include <CWienerMDGG16.h>
#include <CVMUSBReadoutList.h>
#include <VMEAddressModifier.h>
#include <WienerMDGG16Registers.h>

using std::unique_ptr;

namespace WienerMDGG16 
{

  //
  void CDeviceDriver::addWriteLogicalORMaskAB(CVMUSBReadoutList& list, 
      uint32_t mask)
  {
    list.addWrite32(m_base+Regs::Logical_OR_AB, VMEAMod::a24UserData, mask);
  }

  //
  void CDeviceDriver::addWriteLogicalORMaskCD(CVMUSBReadoutList& list, 
      uint32_t mask)
  {
    list.addWrite32(m_base+Regs::Logical_OR_CD, VMEAMod::a24UserData, mask);
  }

  //
  void CDeviceDriver::addReadLogicalORMaskAB(CVMUSBReadoutList& list)
  {
    list.addRead32(m_base+Regs::Logical_OR_AB, VMEAMod::a24UserData);
  }

  //
  void CDeviceDriver::addReadLogicalORMaskCD(CVMUSBReadoutList& list)
  {
    list.addRead32(m_base+Regs::Logical_OR_CD, VMEAMod::a24UserData);
  }

  //
  void CDeviceDriver::addWriteECLOutput(CVMUSBReadoutList& list, 
      uint32_t value)
  {
    list.addWrite32(m_base+Regs::ECL_Output, VMEAMod::a24UserData, value);
  }

  //
  void CDeviceDriver::addReadECLOutput(CVMUSBReadoutList& list)
  {
    list.addRead32(m_base+Regs::ECL_Output, VMEAMod::a24UserData);
  }

  //
  void CDeviceDriver::addReadFirmware(CVMUSBReadoutList& list)
  {
    list.addRead32(m_base+Regs::FirmwareID, VMEAMod::a24UserData);
  }

  //
  void CDeviceDriver::addReadGlobal(CVMUSBReadoutList& list)
  {
    list.addRead32(m_base+Regs::Global, VMEAMod::a24UserData);
  }

  //
  uint32_t CDeviceDriver::readFirmware(CVMUSB& ctlr) 
  {
    unique_ptr<CVMUSBReadoutList> pList(ctlr.createReadoutList());
    addReadFirmware(*pList);

    return executeList<uint32_t>(ctlr,*pList);
  }

  //
  uint32_t CDeviceDriver::readGlobal(CVMUSB& ctlr) 
  {
    unique_ptr<CVMUSBReadoutList> pList(ctlr.createReadoutList());
    addReadGlobal(*pList);

    return executeList<uint32_t>(ctlr,*pList);
  }

  //
  void CDeviceDriver::addWriteLEDNIMOutput(CVMUSBReadoutList& list, 
      uint32_t value)
  {
    list.addWrite32(m_base+Regs::LEDNIM_Output, VMEAMod::a24UserData, value);
  }

  //
  void CDeviceDriver::addReadLEDNIMOutput(CVMUSBReadoutList& list)
  {
    list.addRead32(m_base+Regs::LEDNIM_Output, VMEAMod::a24UserData);
  }


} // end of namespace
