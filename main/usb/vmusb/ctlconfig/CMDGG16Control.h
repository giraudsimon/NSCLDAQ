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

#ifndef __CMDGG16Control_H
#define __CMDGG16Control_H


#ifndef __CCONTROLHARDWARE_H
#include "CControlHardware.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif


#include <CControlModule.h>
#include <CWienerMDGG16.h>
class CVMUSB;

namespace WienerMDGG16 
{


  /**! \brief Encapsulated state used for masks read from a config file */
  struct CControlHdwrState 
  {
    uint32_t or_a;
    uint32_t or_b;
    uint32_t or_c;
    uint32_t or_d;
  };

  /**! \brief Parser for config files of the CControlHdwr class */
  class ConfigFileReader {
    public:
      CControlHdwrState parse(std::string file);
  };

  /*! \brief Slow-controls plug-in for for Wiener MDGG-16
   * 
   * This provides the ability to configure an MDGG-16 device into a logical OR.
   *
   * Configuration parameters include:
   *
   *  -base   - unlimited integer.
   *  -mode   - enum parameter (either "explicit" or "file")
   *  -or_a   - bounded integer between [0,255]
   *  -or_b   - bounded integer between [0,255]
   *  -or_c   - bounded integer between [0,255]
   *  -or_d   - bounded integer between [0,255]
   *  -configfile  - path to file (must exist)
   *
   */
  class CControlHdwr : public CControlHardware
  {
    private:
      CDeviceDriver m_dev; //!< low-level driver that handle logic

    public:
      // canonicals:
      CControlHdwr();
      CControlHdwr(const CControlHdwr& rhs);
      virtual ~CControlHdwr();

      CControlHdwr& operator=(const CControlHdwr& rhs);
      int operator==(const CControlHdwr& rhs) const;
      int operator!=(const CControlHdwr& rhs) const;


    public:
      /*!
       * \brief Configures CControlModule that owns this
       *
       *  Sets up the following parameters 
       *      -base   - unlimited integer.
       *      -mode   - enum parameter (either "explicit" or "file")
       *      -or_a   - bounded integer between [0,255]
       *      -or_b   - bounded integer between [0,255]
       *      -or_c   - bounded integer between [0,255]
       *      -or_d   - bounded integer between [0,255]
       *      -configfile  - path to file (must exist)
       *
       *  \param configuration - Encapsulates the configuration for this module.
       *
       */
      virtual void onAttach(CControlModule& configuration);  //!< Create config.

      /**! \brief Intiailization routine for the device
       *
       *  Configures the device for use as a logical or. The NIM outputs are 
       *  configured to be outputs for the OR_A, OR_B, OR_C, and OR_D definitions.
       *  This also attempts to configure the ECL outputs as well but these do not
       *  currently make good sense and should not be trusted. The state of the
       *  ORs are defined via the parameters -or_a, -or_b, -or_c, and -or_d if the
       *  -mode is "explicit". If instead the -mode is "file" then the values are
       *  read from the -configfile specified.
       *
       * @param vme   a CVMUSB controller
       *
       * \throws std::string if failure occurs while communicating with device
       */
      virtual void Initialize(CVMUSB& vme);

      /**! \brief Update - currently a no-op
       *
       *  \param vme  unused
       *   
       *  \returs "OK" always
       */
      virtual std::string Update(CVMUSB& vme);               //!< Update module.

      /**! Write a parameter value
       *
       * Accepted parameter names are "or_ab" and "or_cd". These are both expected
       * to be uint32_t type values.
       *
       * \param vme       a CVMUSB controller
       * \param parameter name of parameter
       * \param value     string representation of value to write
       *
       * \returns return message
       * \retval <value> return data read from device during operation 
       * \retval ERROR - ... when a bad parameter name is passed
       * \retval ERROR - ... when a failure occurs 
       */
      virtual std::string Set(CVMUSB& vme, std::string parameter, 
          std::string value);

      /**! \brief Read a parameter value
       *
       * The accepted values are identical to those in the Set method.
       * \param vme       a CVMUSB controller
       * \param parameter name of parameter
       *
       * \returns value associated with parameter
       *
       * \returns return message
       * \retval <value> return data read from device during operation 
       * \retval ERROR - ... when a bad parameter name is passed
       * \retval ERROR - ... when a failure occurs 
       */
      virtual std::string Get(CVMUSB& vme, std::string parameter); 

      /**! \brief Virtual copy constructor
       *
       * Creates a brand new copy of this. Ownership is transferred to the caller.
       *
       * \returns copy of this object
       */
      virtual CControlHardware* clone() const;	     //!< Virtual


    private:

      /**! \brief Conveniently access value of -base parameter 
       * \returns value of -base 
       */ 
      uint32_t base();


      /**! \brief Adds operations to set up ECL outputs
       *
       * This is always the same and is not configurable by user options.
       *
       * \param list  a readout list
       */
      void configureECLOutputs(CVMUSBReadoutList& list);

      /**! \brief Adds operations to set up LEDs and NIM outpus
       *
       * This is always the same and is not configurable by user options.
       * The NIM outputs are assigned such that:
       * OUT0 --> OR_A output
       * OUT1 --> OR_B output
       * OUT2 --> OR_C output
       * OUT3 --> OR_D output
       *
       * \param list  a readout list
       */
      void configureLEDsAndNIMOutputs(CVMUSBReadoutList& list);

      /**! \brief Configure OR masks for explicit -mode
       *
       * This computes the values to write to the AB and CD OR registers and then
       * adds the operations to write to the list. The values computed are derived
       * from the -or_a, -or_b, -or_c, and -or_d parameters.
       *
       * \param list a readout list
       */
      void configureORMasks(CVMUSBReadoutList& list);

      /**! \brief Configure OR masks for file mode
       *
       * This computes the values to write to the AB and CD OR registers and then
       * adds the operations to write to the list. The values computed are derived
       * from the or_a, or_b, or_c, and or_d lines in the configfile. 
       *
       * \param list a readout list
       */
      void configureFromConfigFile(CVMUSBReadoutList& ctlr);


  };

} // end namespace


#endif
