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


/**
 *  This file contains a template device driver for the CC-USB 
 * readout framework.  While these comments are no substitute
 * for the documentation available online at http://docs.nscl.msu.edu
 * The idea is that you will have to modify several sections of this code
 * Each section you need to modify is bracketed with comments of the form
 *  MODIFY ME HERE and
 *  END MODIFICATIONS
 *
 *  Comments above eac modification describe what you are doing and to some extent why.
 *
 * The final objective of this exercise is to produce a Tcl loadable package that you
 * can incorporate into the CC-USB readout framework via a load /some/path/to/yourpackage.so
 * command in your configuration file.
 *
 * As such each driver consists of two segments:
 * The main chunk is device driver code.  Device driver code 
 *
 * - Establishes configuration parameters and their constraints.
 * - Uses the values of those configuration parameters to initialize an instance of the device.
 * - Uses the values of the configuration parameters to contribute the CCUSB instructions required
 *   to read the device to the CC-USB readout list to which the module was assigned.
 *
 *  The second chunk is package initialization code. In that chunk, we need to make your driver
 *  known to the configuration subsystem, assigning it a Tcl command ensemble that can
 *  create, configure, and  query the configuration of instances of the driver.
 */


/*
 *  This template builds a driver class named CTemplateDriver  The first thing you should do
 *  is do a global search and replace fo CTemplateDriver to a name that matches the
 *  device you are trying to support.

 * MODIFY ME HERE
 *
 *  END MODIFICATIONS>
 */

#include <CReadoutHardware.h>
#include <stdint.h>
#include <CReadoutModule.h>
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>
#include <CUserCommand.h>
#include <tcl.h>

/*
 * If you have any other headers to include,or  definitions to make add them here:
 *
 * MODIFY ME HERE
 */


/* END MODIFICATIONS */




//////////////////////////////////////////////////////////////////////////////////////////////
/* 
 * The class declaration and definition are in the same file as
 * the header declaration is not, in general needed by other compilation units.
 */

class CTemplateDriver : public CReadoutHardware
{
private:
  /*  If your class needs any per instance data declare that here:
      MODIFY ME HERE */

  /* END MODIFICATIONS */

public:
  CTemplateDriver();
  CTemplateDriver(const CTemplateDriver& rhs);
  virtual ~CTemplateDriver();

private:
  CTemplateDriver& operator=(const CTemplateDriver& rhs); // assignment not allowed.
  int operator==(const CTemplateDriver& rhs) const;	  // Comparison for == and != not suported.
  int operator!=(const CTemplateDriver& rhs) const;


public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CCCUSB& controller);
  virtual void addReadoutList(CCCUSBReadoutList& list);
  virtual CReadoutHardware* clone() const; 

private:
  /*  If your class has any internal utility functions you should declare them here:
   *  MODIFY ME HERE  */

  /* END MODIFICATIONS */

};

//////////////////////////////////////////////////////////////////////////////////////////////
// Constructors and other 'canonical' methods

/**
 * Construct an instance of the device.  Note that in this framework this will
 * typically only be used to make a 'template' instance which will be cloned to
 * create instances that are bound to configurations and actual hardware.
 */
CTemplateDriver::CTemplateDriver() 
{
  m_pConfiguration = 0;		// This will point to an instance's config base class protected).

  /*  If you have any instance data that requires initialization, initialize
      it here.  These initializations only hold for the template object.
      as other objects get created via clone() and the copy constructor below
      MODIFY ME HERE */

  /* END MODIFICATIONS */

}

/**
 * Copy construction.  This cannot be virtual by the rules of C++ the clone()
 * method normally creates a new object from an existing template object.
 * 
 * @param rhs  - Template device that is being copied to create the  new device.
 */
CTemplateDriver::CTemplateDriver(const CTemplateDriver& rhs)
{

  // If the template already has a configuration deep copy it here: 

  m_pConfiguration = 0;
  if (rhs.m_pConfiguration) {
    m_pConfiguration = new CReadoutModule(*(rhs.m_pConfiguration));'
  }

  /*  Any data you declared as instance data must be copied from the
   *  template object to this at this time:
   * 
   * MODIFY ME HERE */

  /* END MODIFICATIONS */

}
/**
 * Destruction.  If your object creatd any dynamic data it must be freed here:
 */
CTemplateDriver::~CTemplateDriver() 
{
  /* Free any dynamic data you allocated in the lifetime of your object
   *
   * MODIFY ME HERE */

  /* END MODIFICATIONS*/
}
///////////////////////////////////////////////////////////////////////////////////////
// Interfaces the driver provides to the framework.

/**
 * This function is called when an instance of the driver has been associated with
 * its configuration database.  The template code stores that in m_pConfiguration
 * The configuration is a CReadoutModule which in turn is derived from
 * XXUSB::CConfigurableObject which encapsulates the configuration database.
 *
 *  You need to invoke methods from XXUSB::CConfigurableObject to create configuration parameters.
 *  by convention a configuration parameter starts with a -.  To illustrate this,
 *  template code will create a -slot configuration parameter that captures the
 *  CAMAC slot number of the device. The slot will be confined to valid CAMAC slot numbers.
 *
 * @parm configuration - Reference to the configuration object for this instance of the driver.
 */
void
CTemplateDriver::onAttach(CReadoutModule& configuration)
{
  m_pConfiguration = &configuration; 

  // Define the slot configuration parameter limited between 1 and 23 (non controller slots)
  // current default is 1.
  //
  m_pConfiguration->addIntegerParameter("-slot", 1, 23, 1);

  /*
   * Add additional configuration parameters you might need here
   *
   * MODIFY ME HERE */

  /* END MODIFICATIONS */


}
/**
 * This method is called when a driver instance is being asked to initialize the hardware
 * associated with it. Usually this involves querying the configuration of the device
 * and using CCUSB controller functions and possibily building and executing
 * CCUSBReadoutList objects to initialize the device to the configuration requested.
 * 
 * @param controller - Refers to a CCUSB controller object connected to the CAMAC crate
 *                     being managed by this framework.
 *
 */
void
CTemplateDriver::Initialize(CCCUSB& controller)
{
  // We'll almost always need the slot so the line below pulls it out of the
  // configuration database for you:

  int slot = m_pConfiguration->getIntegerParameter("-slot");

  /*
   * Add code below to query the remainder of your configuration and initialize
   * the module in accordance with it:
   *
   * MODIFY ME HERE */

  /* END MODIFICATIONS */

}

/**
 * This method is called to ask a driver instance to contribute to the readout list (stack)
 * in which the module has been placed.  Normally you'll need to get some of the configuration
 * parameters and use them to add elements to the readout list using CCUSBReadoutList methods.
 *
 * @param list - A CCUSBReadoutList reference to the list that will be loaded into the
 *               CCUSB.
 */
void
CTemplateDriver::addReadoutList(CCCUSBReadoutList& list)
{
  // Functions are directed at the slot the module is in so:

  int slot = m_pConfiguration->getIntegerParameter("-slot"); // Get the value of -slot.

  /*
   * Add code here to add appropriate operations to the list
   *
   * MODIFY ME HERE */

  /* END MODIFICATIONS */
}

/**
 * This method virtualizes copy construction by providing a virtual method that
 * invokes it.  Usually you don't have to modify this code.
 *
 * @return CTemplateDriver*
 * @retval Pointer to a dynamically allocated driver instance created by copy construction
 *         from *this
 */
CReadoutHardware*
CTemplateDriver::clone() const
{
  return new CTemplateDriver(*this);
}

//////////////////////////////////////////////////////////////////////////////////////////


/*
 * This section of code provides the package initialization code.
 * The Tcl interpreter will call it when the package is loaded.
 * You need to make the driver known to the framework and indicate that you were
 * succesful doing so.  Naturally you can do whatever other initialization you might
 * require here.
 * There are a set of requirements that must be met if Tcl is to locate your package:
 *
 *  The name of the initialization function is derived from the library name you build 
 * (see the Makefile that came with this kit).  If you created libtemplatedriver.so 
 * the name of the initialization function must be Templatedriver_Init
 * The name of the Tcl package you must provide is Templatedriver in that case.
 *
 * The initialization function must also specify C language call methods
 *
 * @param pInterp - Pointer to the raw Tcl Interpreter that is loading this object.
 *
 * @return int 
 * @retval TCL_OK - success.
 * @retval TCL_ERROR - failure.
 */
extern "C" {
  /*
   * You should modify the name below to match you Makefile
   * MODIFY ME HERE */
  int Templatedriver_Init(Tcl_Interp* pInterp)
  /* END MODIFICATIONS */
  {
    // Create a Tcl package so that pkg_mkIndex could find us:

    /* Change the name (Tempatedriver) to match the initialization function.
     * as you evolve the driver you may also want to modify the package version (1.0)
     * MODFIY ME HERE */
    Tcl_PkgProvide(pInterp, "Templatedriver", "1.0");

    /* Register the driver command.  In the line below change the first parameter
     * to the correct Tcl command you want the driver bound to:
     *
     * MODIFY ME HERE */

    CUserCommand::addDriver("changeme", new CTemplateDriver);

    /* END MODIFICATIONS */
    
    return TCL_OK;
    
  }
}
