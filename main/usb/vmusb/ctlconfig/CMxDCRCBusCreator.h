
#ifndef CMXDCRCBUSCREATOR_H
#define CMXDCRCBUSCREATOR_H

#include <CModuleCreator.h>
#include <CMxDCRCBus.h>
#include <memory>

/**! The creator of CMxDCRCBus for the Module command
*
* An instance of this creator object is registered to an
* object of type CModuleCommand.
*/
class CMxDCRCBusCreator : public ::CModuleCreator
{
  public:
   /**! The factory method */
   virtual CControlHardware* operator()(); 
};


#endif
