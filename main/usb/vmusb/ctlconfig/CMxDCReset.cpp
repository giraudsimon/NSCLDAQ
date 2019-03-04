
#include <string.h>

#include <CVMUSB.h>
#include <CControlModule.h>
#include <CMxDCReset.h>
#include <MADC32Registers.h>
#include <VMEAddressModifier.h>
#include <unistd.h>

CMxDCReset::CMxDCReset()
  : CControlHardware()
{}

void CMxDCReset::onAttach(CControlModule& config)
{
  m_pConfig = &config;
  m_pConfig->addParameter("-base", XXUSB::CConfigurableObject::isInteger, NULL, "0");
}

void CMxDCReset::Initialize(CVMUSB& controller)
{
  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  std::cout << "Resetting the MxDC device at base 0x";
  std::cout << std::hex << base << std::dec << std::endl;
  // reset counters ctra and ctrb
  controller.vmeWrite16(base+TimestampReset, VMEAMod::a32UserData,0x3);
  // soft reset
  controller.vmeWrite16(base+Reset,VMEAMod::a32UserData,0);
  // give time for the reset to take effect
  sleep(1);
}

CControlHardware*
CMxDCReset::clone()  const
{
  return (new CMxDCReset(*this));
}

string CMxDCReset::Update(CVMUSB& controller)
{
  return "ERROR - Update is not implemented for CMxDCReset";
}

string CMxDCReset::Set(CVMUSB& controller, string what, string val) 
{
  return "ERROR - Set is not implemented for CMxDCReset";
}

string CMxDCReset::Get(CVMUSB& controller, string what)
{
  return "ERROR - Get is not implemented for CMxDCReset";
}

