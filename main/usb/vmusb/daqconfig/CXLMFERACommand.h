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

#ifndef __CXLMFERACOMMAND_H
#define __CXLMFERACOMMAND_H


#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
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



class CTCLInterpreter;
class CTCLObject;
class CConfiguration;
class CReadoutModule;

/*!
   This class creates and configure marker stack entries.  A marker stack
   entry adds a literal 16 bit word to the output buffer for an event.

   The command supports the usual syntaxes for module generating commands:

\verbatim
  marker create name value
  marker config name -value new_value
  marker cget   name

\endverbatim

  As you can see, the only configuration option supported is

  - -value   Sets a new value for the marker.

  Requiring the value on the creation command line is how we ensure that
the value is mandatory
*/

class CXLMFERACommand : public CTCLObjectProcessor
{
private:
  CConfiguration& m_Config;

public:
  CXLMFERACommand(CTCLInterpreter&     interp,
		 CConfiguration&      config,
		 std::string          commandName = std::string("XLMFERA"));
  virtual ~CXLMFERACommand();

private:
  CXLMFERACommand(const CXLMFERACommand& rhs);
  CXLMFERACommand& operator=(const CXLMFERACommand& rhs);
  int operator==(const CXLMFERACommand& rhs) const;
  int operator!=(const CXLMFERACommand& rhs) const;
public:


  // Command entry point:

protected:


  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

private:
  virtual int create(CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);
  virtual int config(CTCLInterpreter& interp,
		     std::vector<CTCLObject>& objv);
  virtual int cget(CTCLInterpreter& interp,
		   std::vector<CTCLObject>& objv);
  virtual void Usage(std::string msg, std::vector<CTCLObject>& objv);

  int    configure(CTCLInterpreter&         interp,
		   CReadoutModule*          pModule,
		   std::vector<CTCLObject>& config,
		   int                      firstPair = 3);
  std::string configMessage(std::string base,
			    std::string key,
			    std::string value,
			    std::string errorMessage);
};

#endif
