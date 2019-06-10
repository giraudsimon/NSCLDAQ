
#include <string.h>

#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <CControlModule.h>
#include <CXLMControls.h>
#include <CXLM.h>

#include <sstream>
#include <iomanip>
#include <stdexcept>

using namespace std;


namespace XLM
{

///////////////////////////////////////////////////////////////////////////////
/////////////////////// SLOW CONTROLS FOR FIRMWARE LOADS //////////////////////
///////////////////////////////////////////////////////////////////////////////

CXLMControls::CXLMControls()
  : CControlHardware()
{}

void CXLMControls::onAttach(CControlModule& config)
{
  m_pConfig = &config;
  m_pConfig->addParameter("-base", XXUSB::CConfigurableObject::isInteger, NULL, "0");
  m_pConfig->addParameter("-firmware", Utils::validFirmwareFile, NULL, "");
  m_pConfig->addBooleanParameter("-validate", false);
  m_pConfig->addParameter("-signature", XXUSB::CConfigurableObject::isInteger, NULL, "0");
}

void CXLMControls::Initialize(CVMUSB& controller)
{
  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  string path = m_pConfig->cget("-firmware");
  XLM::CFirmwareLoader loader(controller, base);
  loader(path);

  if ( m_pConfig->getBoolParameter("-validate") ) {
    uint32_t expectedSignature = m_pConfig->getUnsignedParameter("-signature");
    uint32_t signatureAddress = loader.getBaseAddress() + XLM::FPGABase;
    if ( ! loader.validate(signatureAddress, expectedSignature) ) {
      stringstream errmsg; errmsg << hex << setfill('0');
      errmsg << "Failed to validate firmware.";
      errmsg << "Expected signature = 0x" << setw(8) << expectedSignature;
      errmsg <<" not found @ 0x" << setw(8) << signatureAddress;

      throw std::runtime_error(errmsg.str());
    }
  }

}

CControlHardware* 
CXLMControls::clone()  const
{
  return (new CXLMControls(*this));
}

string CXLMControls::Update(CVMUSB& controller)
{
  return "ERROR - Update is not implemented for CXLMControls";
}

string CXLMControls::Set(CVMUSB& controller, string what, string val) 
{
  return "ERROR - Set is not implemented for CXLMControls";
}

string CXLMControls::Get(CVMUSB& controller, string what)
{
  return "ERROR - Get is not implemented for CXLMControls";
}

} // end XLM namespace
