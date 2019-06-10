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
#ifndef __CSCRIPTEDSCALERS_H
#define __CSCRIPTEDSCALERS_H


#ifndef __CSCALER_H
#include <CScaler.h>
#endif


#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#include <stdint.h>

// Forward definitions:

class CTCLInterpreter;
class CDigitizerDictionary;
class CReadOrder;
class CModuleCommand;
class CModuleCreator;

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
  void addCreator(CModuleCreator& creator);
  void addStandardCreators();
  virtual void addUserWrittenCreators();
  virtual std::string getConfigurationFile();
};



#endif
