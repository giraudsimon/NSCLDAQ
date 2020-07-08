
#include <string>
#include <CMDGG16Control.h>
#include <CMDGG16ControlCreator.h>
#include <memory>

///////////////////////////////////////////////////////////////////////////////
/////////////////////// CMDGG16ControlCreator ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

namespace WienerMDGG16
{

  CControlHardware* CControlCreator::operator()(void* unused)
  {
    return new CControlHdwr;
  }
  std::string
  CControlCreator::describe() const
  {
    return "mdgg16 - create a wiener mdgg logic/gate and delay module";
  }

}
