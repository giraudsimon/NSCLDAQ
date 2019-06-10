/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
     Jeromy Tompkins
     NSCL
     Michigan State University
     East Lansing, MI 48824-1321
*/


#ifndef CCTLCONFIGURATION_H
#define CCTLCONFIGURATION_H

#include <CControlModule.h>
#include <CControlHardware.h>

#include <string>
#include <vector>
#include <memory>

class CTCLInterpreter;
class CTCLObjectProcessor;
class CVMUSBReadoutList;

/*! \brief Manager for the commands and modules that extend the ctlconfig
 *
 * This provides a similar functionality to the CConfiguration class used
 * in the daqconfig.tcl but slimmed down a bit. It is now what processes the
 * configuration script but rather just holds all of the extensions to the 
 * interpreter. It does not come prestocked with commands either, it is the
 * responsibility of the TclServer to load it with commands. 
 *
 */
class CCtlConfiguration 
{
private:
  std::vector<std::unique_ptr<CTCLObjectProcessor> > m_Commands; //!< tcl command extensions
  std::vector<CControlModule*>    m_Modules;  //!< instantiated control modules

  //
  // Canonicals:
  //
public:
  CCtlConfiguration();
  virtual ~CCtlConfiguration();

private:
  CCtlConfiguration(const CCtlConfiguration& rhs);
  CCtlConfiguration& operator=(const CCtlConfiguration& rhs);
  int operator==(const CCtlConfiguration& rhs) const;
  int operator!=(const CCtlConfiguration& rhs) const;
public:


  /*! \brief Register a new tcl command
   *
   * This configuration takes ownership of the object whose pointer is passed 
   * in as an argument.  There is no attempt to prevent multiple instances of 
   * a command name to be present. Rather the last one registered to the Tcl 
   * interperet will be by the interpreter. 
   *
   * \param command   pointer to command
   */
  void addCommand(std::unique_ptr<CTCLObjectProcessor> command);

  /*! \brief Register a module for use
   *
   * After a user's ctlconfig script is parsed, all of the modules it created
   * should have been added to the configuration by this commmand. Each module
   * is passed in and the configuration takes ownership of the resources.
   *
   * \param module  pointer to the module
   */
  void addModule(CControlModule* module);

  /*! \brief Retrieve a register module by its name
   *
   * Utility method for searching the list of registered modules
   * for the one that matches the given name.
   *
   * \param name    name of the module to search for
   *
   * \retval nullptr - no module found
   *         non-owning pointer to module if found
   *
   */
  CControlModule* findModule(const std::string& name);

  /*! \brief Retrieve the list of modules */
  std::vector<CControlModule* >& getModules() { return m_Modules; }

private:
  /*! \brief Utility method to help finding modules
   *
   *  \param modules  list of modules to search
   *  \param name     name of module to search for
   */
  CControlModule* find(const std::vector<CControlModule*>& modules,
                       const std::string& name);

  // A callable class that is matches the name of a CControlModule
  // It is useful for std::find
  class MatchName {
    private:
      std::string m_name;
    public:
      MatchName(std::string name) :
        m_name(name) {}
      MatchName(const MatchName& rhs) :
        m_name(rhs.m_name) {}
      bool operator()(const CControlModule* pModule);
  };

};

#endif
