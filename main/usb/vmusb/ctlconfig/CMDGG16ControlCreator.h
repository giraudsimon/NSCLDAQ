
#ifndef CMDGG16CONTROLCREATOR_H
#define CMDGG16CONTROLCREATOR_H

#include <CModuleCreator.h>
#include <CMDGG16Control.h>
#include <memory>

namespace WienerMDGG16
{

  /**! The creator of ::WienerMDGG16::CControlHdwr for the Module command
   *
   * An instance of this creator object is registered to an
   * object of type CModuleCommand.
   */
  class CControlCreator : public ::CModuleCreator
  {
    public:
      /**! The factory method */
      virtual CControlHardware* operator()(); 
  };

} // end namespace 

#endif
