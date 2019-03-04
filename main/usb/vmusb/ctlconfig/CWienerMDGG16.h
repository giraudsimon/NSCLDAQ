//
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
//

#ifndef __CWienerMDGG16_H
#define __CWienerMDGG16_H

#include <stdint.h>

#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>

namespace WienerMDGG16
{

  /**! \brief Low-level driver for handling interactions with an MDGG-16 
   *
   * This provides methods to add operations to readout lists and some methods
   * for performing straight single-shot commands. It does not implement any
   * interface and is intended to be useful for implementing other classes that
   * control the device. For example, should the device become useful as a
   * scaler device, one could derive a CReadoutHardware method that delegates
   * the logical operations of communication with the device to this class. At
   * the moment of this writing, the only use of this class is in the
   * WienerMDGG16::CControlHdwr class that implements the interface required for
   * a slow-controls server module. 
   */
  class CDeviceDriver {

    private:
      uint32_t m_base; //!< base address of object

    public:
      // uses the default implementations of the ctors and dtors
      CDeviceDriver() = default;
      CDeviceDriver(const CDeviceDriver& ) = default;
      ~CDeviceDriver() = default;

    public:
      /**! \brief Set the base address
       *
       * All operations must include the base address of the device in the VME
       * address. This is the mechanism to provide that information. 
       *
       * \param base  address value
       */
      void setBase(uint32_t base) { m_base = base; }

      /**! \brief Retrieve the current base address
       * \returns base address
       */
      uint32_t getBase() const { return m_base; }

      /////////////////////////////////////////////////////////////////////////
      //
      //         Stack manipulating method
      //

      /**! \brief Adds operations to rdolist for writing to Logical OR AB
       *          register
       *
       * \param list    readout list to manipulate
       * \param mask    value to write
       */
      void addWriteLogicalORMaskAB(CVMUSBReadoutList& list, uint32_t mask);

      /**! \brief Adds operations to rdolist for writing to Logical OR CD
       *          register
       *
       * \param list    readout list to manipulate
       * \param mask    value to write
       */
      void addWriteLogicalORMaskCD(CVMUSBReadoutList& list, uint32_t mask);


      /**! \brief Same as addWriteLogicalORMaskAB, but for a read
       *
       * \param list    readout list to manipulate
       */
      void addReadLogicalORMaskAB(CVMUSBReadoutList& list);

      /**! \brief Same as addWriteLogicalORMaskCD, but for a read
       *
       * \param list    readout list to manipulate
       */
      void addReadLogicalORMaskCD(CVMUSBReadoutList& list);

      /**! \brief Adds operations to rdolist for configuring ECL Output 
       *          register
       *
       * \param list    readout list to manipulate
       * \param val     value to write
       */
      void addWriteECLOutput(CVMUSBReadoutList& list, uint32_t val);

      /**! \brief Same as addWriteECLOutput, but for a read
       *
       * \param list    readout list to manipulate
       */
      void addReadECLOutput(CVMUSBReadoutList& list);

      /**! \brief Adds operations to rdolist for configuring LED/NIM Output 
       *          register
       *
       * \param list    readout list to manipulate
       * \param val     value to write
       */
      void addWriteLEDNIMOutput(CVMUSBReadoutList& list, uint32_t val);

      /**! \brief Same as addWriteLEDNIMOutput, but for a read
       *
       * \param list    readout list to manipulate
       */
      void addReadLEDNIMOutput(CVMUSBReadoutList& list);

      /**! \brief Adds operations to rdolist for reading firmware register
       *
       * \param list    readout list to manipulate
       */
      void addReadFirmware(CVMUSBReadoutList& list);

      /**! \brief Adds operations to rdolist for reading global register
       *
       * \param list    readout list to manipulate
       */
      void addReadGlobal(CVMUSBReadoutList& list);

      /////////////////////////////////////////////////////////////////////////
      //
      //           Interactive commands on a CVMUSB
      //

      /**! \brief Interactively reads firmware register content
       *
       * This used addReadFirmware to build a stack, execute it, and then return
       * the result.
       *
       * \param ctlr    a vme controller
       *
       * \returns the value of the firmware register
       *
       * \throws std::string if executeList returns negative status
       */
      uint32_t readFirmware(CVMUSB& ctlr);

      /**! \brief Interactively reads global register content
       *
       * This used addReadGlobal to build a stack, execute it, and then return
       * the result.
       *
       * \param ctlr    a vme controller
       *
       * \returns the value of the global register
       *
       * \throws std::string if executeList returns negative status
       */
      uint32_t readGlobal(CVMUSB& ctlr);

    private:

      /**!  \brief Utility method for executing a rdolist
       *
       * This will execute the list and return an amount of data the size of 
       * template parameter. 
       *
       * \param ctlr    a VMUSB controller
       * \param list    a readout list
       *
       * \returns value read from the device
       *
       * \throws std::string if operation returned different amount data than
       *         size of template parameter type
       */
      template <class T> T executeList(CVMUSB& ctlr, CVMUSBReadoutList& list);

  };



  // Implementation of the executeList method
  template <class T>
    T CDeviceDriver::executeList(CVMUSB& ctlr, CVMUSBReadoutList& list)
    {
      size_t nRead=0;
      T buffer;
      int status = ctlr.executeList(list, &buffer, sizeof(buffer), &nRead);
      if (status<0) {
        std::string errmsg ("CDeviceDriver::readGlobal() failed during ");
        errmsg += "executeList() with status " + std::to_string(status);
        throw errmsg;
      }

      if (nRead != sizeof(buffer)) {
        std::string errmsg ("CDeviceDriver::executeList() read back fewer");
        errmsg += " bytes than were expected.";
        throw errmsg;
      }

      return buffer;
    }

} // end namespace

#endif
