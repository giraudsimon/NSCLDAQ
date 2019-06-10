
#ifndef CMXDCRESETCREATOR_H
#define CMXDCRESETCREATOR_H

#include <CModuleCreator.h>
#include <memory>

/**! The creator of CXLMControls for the Module command
*
* An instance of this creator object is registered to an
* object of type CModuleCommand.
*/
class CMxDCResetCreator : public ::CModuleCreator
{
  public:
   /**! The factory method */
   virtual CControlHardware* operator()(); 
};

#endif
