
#include <string>
#include <CMxDCRCBus.h>
#include <CMxDCRCBusCreator.h>
#include <memory>

///////////////////////////////////////////////////////////////////////////////
/////////////////////// CMxDCRCBusCreator ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

CControlHardware* CMxDCRCBusCreator::operator()(void* unused)
{
  CMxDCRCBus* pItem = new CMxDCRCBus;
  return pItem;
}
std::string
CMxDCRCBusCreator::describe() const
{
  return "mxdcrcbus - Mesytec RC bus controller";
}

