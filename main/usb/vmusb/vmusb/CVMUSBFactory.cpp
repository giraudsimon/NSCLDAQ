/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include "CVMUSBFactory.h"
#include "CVMUSBusb.h"
#include "CVMUSBEthernet.h"
#include <string>
#include <vector>
/**
 * @file   CVMUSBFactory.cpp
 * @brief  implementation of VMUSB factory methods.
 */

/**
 * createUSBController
 *
 * Generic creational method.  This determines what type of controller to create.
 * based on the controller type and specifier.
 *
 * @param type       - Type of controller, locale or remote, to create.
 * @param specifier  - Specifies the controller given the type: 
 *                     - local - the serial number of the controller or NULL if you want the
 *                       software to pick the first one enumerated.
 *                     - remote - the host on which the VMUSB server is running.  In general such hosts
 *                       only manage one server.  If null, the server running in localhost is used,.
 *
 * @return CVMUSB* Pointer to the appropriate VMUSB controller object.  It's the responsibility
 *                 of the caller to delete when done.
 */

CVMUSB*
CVMUSBFactory::createUSBController(CVMUSBFactory::ControllerType type, const char* specifier)
{
  if (type == local) {
    return createLocalController(specifier);
  } else if (type == remote) {
    return createRemoteController(specifier);
  } else {
    throw std::string("Invalid controller type in createUSBController!");
  }
}
/**
 * createLocalController
 *
 *  Connect with a local controller:
 *  - Enumerate the controllers.
 *  - If serialNumber is 0 instantiatee a CVMUSBusb on the first one enumerated.
 *  - If serialNumber is not null, instantiate on the controller that matches the
 *    serial numer string.
 *
 * @param serialNumber - Desired serial number string.
 *
 * @return CVMUSB* - Pointer to the VM USB object created
 * @throw std::string - Unable to connect with the desired controller.
 */
CVMUSB*
CVMUSBFactory::createLocalController(const char* serialNumber)
{
  // enumerate the devices - if there aren't any it's an error.

  std::vector<struct usb_device*> controllers = CVMUSBusb::enumerate();

 if (controllers.size() == 0) {
    throw std::string("There are no VM-USB controllers attached to this system");
  }
  // Figure out which we want to match:

  struct usb_device* pSelected = controllers[0];

  if (serialNumber) {
    pSelected = 0;
    for (int i =0; i < controllers.size(); i++) {
      std::string serial = CVMUSBusb::serialNo(controllers[i]);
      if (serial == serialNumber) {
	pSelected = controllers[i];
	break;
      }
    }
  }
  // It's an error for there not to be any matching serial number
    
  if (!pSelected) {
    throw std::string("No matching VM-USB controller");
  }

  return new CVMUSBusb(pSelected);


}
/**
 * createRemoteController
 *
 * Create a CVMUSBethernet - that is a served VMUSB.
 *
 * @param host - the host on which the controller server is running
 *               If null, 'localhost' is used.
 */
CVMUSB*
CVMUSBFactory::createRemoteController(const char* pHost)
{
  if (!pHost) {
    pHost = "localhost";
  }
  // Warn the user that this will not work unless they have implemented the server.
  std::cerr << "WARNING! Call to CVMUSBFactory::createRemoteController(const char*) constructs ";
  std::cerr << "a CVMUSBEthernet object, which is the client portion of a server that ";
  std::cerr << "is NOT provided by NSCLDAQ. The user must supply his/her own server if ";
  std::cerr << "this feature is to work correctly or obtain a copy of the vmusbserver that is a ";
  std::cerr << "separate project.";
  std::cerr << std::endl;
  
  CVMUSBEthernet* pController =  new CVMUSBEthernet("vmusb", std::string(pHost));
  pController->reconnect();                // Force server to reconnect.
  return pController;
}
