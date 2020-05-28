
#include <string>
#include <CXLMControls.h>
#include <CXLMControlsCreator.h>
#include <memory>

namespace XLM
{

///////////////////////////////////////////////////////////////////////////////
/////////////////////// CXLMControlsCreator ///////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


  CControlHardware* CXLMControlsCreator::operator()(void* unused)
  {
    return (new CXLMControls);
  }
  std::string
  CXLMControlsCreator::describe() const
  {
    return "xml - XML control module";
  }

}
