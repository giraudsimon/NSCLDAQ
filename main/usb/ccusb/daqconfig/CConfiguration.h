/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


#ifndef __CCONFIGURATION_H
#define __CCONFIGURATION_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

class CTCLInterpreter;
class CReadoutModule;
class CTCLObjectProcessor;


/*!
   This class encapsulates the configuration of
   Readout and scaler modules.  It makes available
   services to:
   - Read a configuration.
   - To locate a module by name.
   - To add new modules to the system.

   The idea is that when a run starts, the software will
   create an instance of the configuration and then
   configure it.  At run end, we destroy that instance,
   so that the new run is reconfigured according to any 
   changes in the configuration file.
*/
class CConfiguration 
{
private:
  std::vector<CReadoutModule*>   m_Adcs; // event digitizers.
  std::vector<CReadoutModule*>   m_Scalers; // Scalers ..no  real distinction now.
  std::vector<CReadoutModule*>   m_Stacks; // This is what we load.

  CTCLInterpreter*                    m_pInterp;
  std::vector<CTCLObjectProcessor*>   m_Commands;
  //
  // Canonicals:
  //
public:
  CConfiguration();
  virtual ~CConfiguration();

  // selectors
  
  CTCLInterpreter* getInterpreter() {return m_pInterp;}

  // lazy so:
private:
  CConfiguration(const CConfiguration& rhs);
  CConfiguration& operator=(const CConfiguration& rhs);
  int operator==(const CConfiguration& rhs) const;
  int operator!=(const CConfiguration& rhs) const;
public:

  // Reading configurations:

  void processConfiguration(std::string configFile);

  CReadoutModule* findAdc(std::string name);
  CReadoutModule* findScaler(std::string name);
  CReadoutModule* findStack(std::string name);

  void addScaler(CReadoutModule*);
  void addAdc(CReadoutModule*);
  void addStack(CReadoutModule*);

  
  std::vector<CReadoutModule*> getAdcs();
  std::vector<CReadoutModule*> getScalers();
  std::vector<CReadoutModule*> getStacks();

  void setResult(std::string);



  // Utilties.

  // For STL find_if.

private:
  CReadoutModule* find(std::vector<CReadoutModule*>& modules,
		       std::string name);

  class MatchName {
  private:
    std::string m_name;
  public:
    MatchName(std::string name) :
      m_name(name) {}
    MatchName(const MatchName& rhs) :
      m_name(rhs.m_name) {}
    bool operator()(CReadoutModule* module);
  };
  
};










#endif
