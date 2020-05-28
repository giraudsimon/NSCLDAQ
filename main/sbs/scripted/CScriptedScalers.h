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
#ifndef CSCRIPTEDSCALERS_H
#define CSCRIPTEDSCALERS_H


#include <CScaler.h>
#include <vector>
#include <string>
#include <stdint.h>
#include "CModuleCreator.h"

// Forward definitions:

class CTCLInterpreter;
class CDigitizerDictionary;
class CReadOrder;
class CModuleCommand;


struct ScriptedBundle;
/*!
   This is to scalers what CScriptedSegment is to the event by event readout.
   One differencde from the old scripted readout is that scaler definitions
   are processed from a different hardware file, named scalers.tcl
   This is hunted for in all the same places as hardware.tcl.

*/


class CScriptedScalers : public CScaler
{
  ScriptedBundle*       m_pBundle;
  CTCLInterpreter*      m_pInterp;



public:
  CScriptedScalers();

public:
  virtual void initialize();
  virtual void clear();
  virtual void disable();
  virtual std::vector<uint32_t> read();

  virtual bool isComposite() const;
protected:
  void addCreator(const char* type, CModuleCreator& creator);
  void addStandardCreators();
  virtual void addUserWrittenCreators();
  virtual std::string getConfigurationFile();
};



#endif
