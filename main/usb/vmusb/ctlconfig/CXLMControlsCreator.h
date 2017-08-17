
#ifndef CXLMCONTROLSCREATOR_H
#define CXLMCONTROLSCREATOR_H

#include <CModuleCreator.h>
#include <CXLMControls.h>
#include <memory>

namespace XLM
{

/**! The creator of CXLMControls for the Module command
*
* An instance of this creator object is registered to an
* object of type CModuleCommand.
*/
class CXLMControlsCreator : public ::CModuleCreator
{
  public:
   /**! The factory method */
   virtual CControlHardware* operator()(); 
};


} // end XLM namespace

#endif
