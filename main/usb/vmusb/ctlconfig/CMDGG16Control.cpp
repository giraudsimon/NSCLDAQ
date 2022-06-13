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
#include "CMDGG16Control.h"
#include "WienerMDGG16Registers.h"
#include "CWienerMDGG16.h"
#include "CControlModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sys/stat.h>
#include <unistd.h>
#include <limits>

using namespace std;

static const char* modeEnum [] = { "explicit" , "file", 0 };

static bool fileReadable(std::string, std::string value, void* arg) 
{
  int status = access(value.c_str(), R_OK);
  return (status == 0);
}


namespace WienerMDGG16
{

  CControlHdwrState ConfigFileReader::parse(std::string path) 
  {
    ifstream file(path.c_str());

    CControlHdwrState state = {0,0,0,0};
    string varname;

    // first two lines are nothing important
    file.ignore(numeric_limits<streamsize>::max(), '\n');
    file.ignore(numeric_limits<streamsize>::max(), '\n');

    // next come the masks
    file >> varname >> state.or_a;
    file >> varname >> state.or_b;
    file >> varname >> state.or_c;
    file >> varname >> state.or_d;

    // after the masks come the names... we don't care b/c they have nothing
    // to do with the device.

    return state;
  }


  /*!
    construct the beast.. The shadow registers will all get set to zero
    */
  CControlHdwr::CControlHdwr() :
    CControlHardware(), m_dev()
  {
  }

  /*!

    Copy construction:
    */
  CControlHdwr::CControlHdwr(const CControlHdwr& rhs) :
    CControlHardware(rhs), m_dev(rhs.m_dev)
  {
  }
  /*!
    While destruction could leak I seem to recall problems if I destroy
    the configuration..
    */
  CControlHdwr::~CControlHdwr()
  {
  }

  /*!
    Assignment is a clone:
    */
  CControlHdwr&
    CControlHdwr::operator=(const CControlHdwr& rhs)
    {
      if(this != &rhs) {
        m_dev = rhs.m_dev;
        CControlHardware::operator=(rhs);
      }
      return *this;
    }

  /*!
    Same configuration implies equality:
    */
  int 
    CControlHdwr::operator==(const CControlHdwr& rhs) const
    {
      return CControlHardware::operator==(rhs);
    }
  /*
     != is the logical inverse of ==
     */
  int
    CControlHdwr::operator!=(const CControlHdwr& rhs) const
    {
      return !(*this == rhs);
    }

  ///////////////////////////////////////////////////////////////////////////

  void
    CControlHdwr::onAttach(CControlModule& configuration)
    {
      m_pConfig = &configuration;
      configuration.addParameter("-base", XXUSB::CConfigurableObject::isInteger, NULL, 
          string("0"));

      configuration.addEnumParameter("-mode", modeEnum, "explicit");
      configuration.addIntegerParameter("-or_a",0,65535,65535);
      configuration.addIntegerParameter("-or_b",0,65535,65535);
      configuration.addIntegerParameter("-or_c",0,65535,65535);
      configuration.addIntegerParameter("-or_d",0,65535,65535);

      configuration.addParameter("-configfile", fileReadable, nullptr, "");
    }
  ////////////////////////////////////////////////////////////////////////////
  /*!
    Initialize the module bringing it to a known state.

    \param CVMUSB& vme
    Controller that hooks us to the VM-USB.
    */
  void
    CControlHdwr::Initialize(CVMUSB& vme)
    {
      m_dev.setBase(base());

      unique_ptr<CVMUSBReadoutList> pList(vme.createReadoutList());

      // set up the outputs and leds
      configureECLOutputs(*pList);   
      configureLEDsAndNIMOutputs(*pList);   

      // behave properly according to the mode.
      if (m_pConfig->getEnumParameter("-mode",modeEnum)==0) {
        // explicit mode 
        configureORMasks(*pList);   
      } else {
        // file mode
        configureFromConfigFile(*pList);
      }

      // execute the list
      size_t nRead=0;
      uint32_t buf[8];
      int status = vme.executeList(*pList, buf, sizeof(uint16_t), &nRead);

      if (status < 0) {
        std::stringstream errmsg;
        errmsg << "CControlHdwr::Initialize() failed while executing list ";
        errmsg << "with status = " << status;
        throw errmsg.str();
      }
    }

  string CControlHdwr::Update(CVMUSB& vme)
  {
    return string("OK");
  }


  string
    CControlHdwr::Set(CVMUSB& vme, string parameter, string value)
    {
      // to ensure that we use the most recent base address.
      m_dev.setBase(base());

      // convert string to unsigned long integer (auto detects base)
      uint32_t val = std::stoul(value,0,0);

      unique_ptr<CVMUSBReadoutList> pList( vme.createReadoutList() );

      // dispatch parameter to specific operation
      if (parameter == "or_ab") {
        m_dev.addWriteLogicalORMaskAB(*pList, val);
      } else if (parameter == "or_cd") {
        m_dev.addWriteLogicalORMaskCD(*pList, val);
      } else {
        std::string retval("ERROR - invalid parameter name \"");
        retval += parameter + "\"";
        return retval;
      }


      // Execute the list.
      size_t nRead=0;
      uint32_t buf[8];
      int status = vme.executeList(*pList, buf, sizeof(uint16_t), &nRead);
      if (status<0) {
        std::string retval("ERROR - executeList returned status = ");
        retval += std::to_string(status);
        return retval;
      }

      // Format the output
      std::string result;
      size_t nLongs = nRead/sizeof(uint32_t);
      if (nLongs>0) {
        for (size_t i=0; i<nLongs; ++i) {
          result += (std::to_string(buf[i]) + " ");
        } 
      }
      return result;
    }

  //////////////////////////////////////////////////////////////////////////////
  //
  string CControlHdwr::Get(CVMUSB& vme, string parameter)
  {
    // make sure that the logical device is configured properly before using it
    m_dev.setBase(base());

    unique_ptr<CVMUSBReadoutList> pList(vme.createReadoutList());

    // Dispatch functionality for parameter type
    //  .. if lots more functionality is required, this might be made a bit less
    //  .. crude than a simple if-else scheme.
    if (parameter=="or_ab") {
      m_dev.addReadLogicalORMaskAB(*pList);
    } else if (parameter=="or_cd") {
      m_dev.addReadLogicalORMaskCD(*pList);
    } else {
      std::string retval("ERROR - invalid parameter name \"");
      retval += parameter + "\"";
      return retval;
    } 

    // execute list
    size_t nRead=0;
    uint32_t buf;
    int status = vme.executeList(*pList, &buf, sizeof(buf), &nRead);
    if (status<0) {
      std::string retval("ERROR - executeList returned status = ");
      retval += std::to_string(status);
      return retval;
    }

    string result = "OK";
    // convert returned data into a list
    size_t nLongs = nRead/sizeof(uint32_t);
    if (nLongs>0) {
      // reset the value
      result = "";
      result += std::to_string(buf);
      
    }
    return result;

  }

  ///////////////////////////////////////////////////////////////////////////////////////
  /*!
    At present, cloning is a no-op.
    */
  CControlHardware*
  CControlHdwr::clone() const
  {
    return (new CControlHdwr(*this));
  }
  
  //////////////////////////////////////////////////////////////////////////////////
  ///////////////// private utilities //////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////

  /*
     Return the base address of the device:

*/
  uint32_t 
    CControlHdwr::base()
    {
      if (m_pConfig) {
        string strBase = m_pConfig->cget("-base");
        unsigned long base;
        base = strtoul(strBase.c_str(), NULL, 0);
        return static_cast<uint32_t>(base);
      }
      else {
        return static_cast<uint32_t>(0);
      }
    }

  void CControlHdwr::configureECLOutputs(CVMUSBReadoutList& list)
  {
    if (m_pConfig) {
      // there is currently only 1 option and that is to have all
      // of the logical or outputs provided. 
      using namespace ECL_Output;
      using ECL_Output::Logical_OR;

      uint32_t outputBits = 0;

      // I believe this is supposed to direct 
      // OR_A to ECL9 and ECL13
      // OR_B to ECL10 and ECL14
      // OR_C to ECL11 and ECL15
      // OR_D to ECL12 and ECL16
      // .. but it is not clear that it is doing so.
      outputBits |= (Logical_OR << ECL9_Offset);
      outputBits |= (Logical_OR << ECL10_Offset);
      outputBits |= (Logical_OR << ECL11_Offset);
      outputBits |= (Logical_OR << ECL12_Offset);
      outputBits |= (Logical_OR << ECL13_Offset);
      outputBits |= (Logical_OR << ECL14_Offset);
      outputBits |= (Logical_OR << ECL15_Offset);
      outputBits |= (Logical_OR << ECL16_Offset);
      m_dev.addWriteECLOutput(list, outputBits);
    }
  }


  void CControlHdwr::configureLEDsAndNIMOutputs(CVMUSBReadoutList& list)
  {
    if (m_pConfig) {
      // there is currently only 1 option and that is to have all
      // of the logical or outputs provided. 
      using namespace ::WienerMDGG16::LEDNIM_Output;
      uint32_t outputBits = 0;

      // set up the NIM outputs 
      outputBits |= (NIM_Logical_OR << NIM1_Shift);
      outputBits |= (NIM_Logical_OR << NIM2_Shift);
      outputBits |= (NIM_Logical_OR << NIM3_Shift);
      outputBits |= (NIM_Logical_OR << NIM4_Shift);

      // set up the LEDs
      outputBits |= (LED_ECL_OR_1234<< LEDGreen_Lt_Shift);
      outputBits |= (LED_ECL_OR_12 << LEDGreen_Rt_Shift);
      outputBits |= (LED_ECL_OR_23 << LEDYellow_Lt_Shift);
      outputBits |= (LED_ECL_OR_34 << LEDYellow_Rt_Shift);
      m_dev.addWriteLEDNIMOutput(list, outputBits);
    }
  }


  void CControlHdwr::configureORMasks(CVMUSBReadoutList& list)
  {
    if (m_pConfig) {
      uint32_t or_a = m_pConfig->getUnsignedParameter("-or_a"); 
      uint32_t or_b = m_pConfig->getUnsignedParameter("-or_b"); 
      uint32_t or_c = m_pConfig->getUnsignedParameter("-or_c"); 
      uint32_t or_d = m_pConfig->getUnsignedParameter("-or_d"); 

      using namespace ::WienerMDGG16::Logical_OR;

      uint32_t or_ab = (or_b<<B_Offset)|(or_a<<A_Offset);
      uint32_t or_cd = (or_d<<D_Offset)|(or_c<<C_Offset);
      m_dev.addWriteLogicalORMaskAB(list, or_ab); 
      m_dev.addWriteLogicalORMaskCD(list, or_cd);
    }
  }

  void CControlHdwr::configureFromConfigFile(CVMUSBReadoutList& list)
  {
    if (m_pConfig) {
      std::string path = m_pConfig->cget("-configfile");

      // read in the config file
      CControlHdwrState state = ConfigFileReader().parse(path);

      using namespace ::WienerMDGG16::Logical_OR;

      // build the values to write
      uint32_t or_ab = (state.or_b<<B_Offset)|(state.or_a<<A_Offset);
      uint32_t or_cd = (state.or_d<<D_Offset)|(state.or_c<<C_Offset);

      // add the writes to the readout list for later execution
      m_dev.addWriteLogicalORMaskAB(list, or_ab); 
      m_dev.addWriteLogicalORMaskCD(list, or_cd);
    }
  }

} // end namespace
