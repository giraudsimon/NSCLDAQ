/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef CTCLCONTROLMODULE_H
#define CTCLCONTROLMODULE_H


/**
 * @file CTclControlModule.h
 * @brief Class definition for a wrapper for Tcl based control modules.
 */

#include "CControlHardware.h"	/* base class. */

#include <memory>

#include <CControlModule.h>

class CTCLObject;
class CTCLInterpreter;
class CCCUSB;

/**
 * @class CTclControlModule
 *
 *  This class allows one to define control modules that are implemented in Tcl.
 *  Tcl control modules must be command ensembles that implement the subcommands:
 *  Initialize, Update, Set, Get, addMonitorList processMonitorList and getMonitoredData
 *
 *  Typically this is all done by implementing either a snit::type or an incrTcl class
 *  Creating and configuring an instance of that entity and wrapping that entity in
 *  a CTclControlModule.  The wrapping is done by providing the command ensemble base command
 *  to the -ensemble configuration option.  e.g:
 * \verbatim
 *    snit::type mydriver {
 *    ...
 *    };                      #Assume this snit::type implements the appropriate methods.
 *    set instance [mydriver %AUTO%]
 *    Module create tcl mydriver
 *    Module config mydriver -ensemble $instance
 *
 * \endverbatim
 *
 *  @note The VMUSB and VMUSBReadoutList objects passed to the Tcl driver are the same 
 *  swig wrappers thatn are used for readout drivers.
 */
class CTclControlModule :  public CControlHardware
{
  private:
    CTCLInterpreter& m_interp;

public:
  CTclControlModule(CTCLInterpreter& interp);
  
public:
  virtual void onAttach(CControlModule& configuration);
  virtual void Initialize(CCCUSB& vme);	                     //!< Optional initialization.
  virtual std::string Update(CCCUSB& vme);
  virtual std::string Set(CCCUSB& vme, 
			  std::string parameter, 
			  std::string value);
  virtual std::string Get(CCCUSB& vme, 
			  std::string parameter);
  virtual CControlHardware* clone() const;

 

private:
  static std::string swigPointer(void* pObject, std::string type);
  static CTCLObject* marshallData(CTCLInterpreter* pInterp, void* pData, size_t nBytes);
};
#endif
