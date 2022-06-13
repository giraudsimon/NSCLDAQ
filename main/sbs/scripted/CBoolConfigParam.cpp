/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

static const char* Copyright = "(C) Copyright Michigan State University 1977, All rights reserved";
/*! \class CBoolConfigParam   
           CLASS_PACKAGE
           Encapsulates a boolean parameter. 
*/
#include <config.h>

#include "CBoolConfigParam.h"    		
#include <TCLInterpreter.h>
#include <TCLResult.h>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif

/*!
  Constructor.
  \param rName const string& [in]
      Name of the parameter (keyword recognized by config).
  \param fDefault bool [in]
      Default (initial) value of the parameter.
*/
CBoolConfigParam::CBoolConfigParam (const std::string& rName,
                                    bool          fDefault) :
  CConfigurationParameter(rName),
  m_fValue(fDefault)
{
  setValue(fDefault ? "true" : "false");
} 

/*!
   Destructor
*/
CBoolConfigParam::~CBoolConfigParam ( )
{
}
/*!
   Copy constructor.  Used to create temproraries such as
  for passing by value to functions.
  \param rhs const CBoolConfigParam [in]
        The reference object we are copying.
*/
CBoolConfigParam::CBoolConfigParam (const CBoolConfigParam& rhs) 
  : CConfigurationParameter (rhs),
    m_fValue(rhs.m_fValue)
{
} 

/*!
   Assignment
  \param rhs const CBoolConfigParam& [in]
      The right hand parameter of the = operator.
  \return reference to *this.
*/
CBoolConfigParam& 
CBoolConfigParam::operator= (const CBoolConfigParam& rhs)
{ 
  if(this != &rhs) {
    CConfigurationParameter::operator=(rhs);
    m_fValue = rhs.m_fValue;
  }
  return *this;
}
/*!
  Compare for functional equality.
  \param rhs const CBoolConfigParam& [in]
      Right hand operand of the == operator.
  \return Any of:
  - 0 For inequality.
  - 1 for equality.
*/
int CBoolConfigParam::operator== (const CBoolConfigParam& rhs) const
{ 
  return ( CConfigurationParameter::operator==(rhs)  &&
          (m_fValue == rhs.m_fValue));
}

// Functions for class CBoolConfigParam

/*!  
Returns the value of the parameter.

*/
bool 
CBoolConfigParam::getOptionValue()  
{ 
  return m_fValue;
}  

/*! 

Parses the parameter value.
This function can fail if:
- The parameter is not avalid bool.

\return One of:
- TCL_OK - the parse worked.
- TCL_ERROR - the pares failed.

*/
int 
CBoolConfigParam::SetValue(CTCLInterpreter& rInterp, 
                           CTCLResult& rResult, 
                           const char* pFlag)  
{ 
  // First try to parse the flag.. It seems that ExprBoolean is busted.
  // in some way, so we'll do a manual parse.. we want to expand the 
  // acceptable bool values to enabled disable too so what the heck.
  try {
    TCLPLUS::Bool_t param;
    param = ParseFlag(pFlag);
    m_fValue = param;
  }
  catch (...) {
    std::string Result;
    Result += "Boolean parameter: ";
    Result += getSwitch();
    Result += " value ";
    Result += pFlag;
    Result += " Cannot be parsed as a bool";
    rResult.AppendElement(Result);
    return TCL_ERROR;
  }
  return TCL_OK;
}
/*!
   \return The format of the configuration parameter in this
        case "on | off"
*/
std::string
CBoolConfigParam::GetParameterFormat()
{
  return std::string("on | off");
}
/*!
   Utility function to parse a boolean flag:
   \param value const char* [in]   Character value of the flag. Legal 
   values for true are:
   - true
   - on
   - enable
   Legal values for false are:
   - false
   - off
   - disable

   \return Bool_t kfTRUE if parses as true, kfFALSE if as false, and an
   exception (char*) if no parse.
*/
bool
CBoolConfigParam::ParseFlag(const char* value)
{
  std::string sValue(value);
  if(  (sValue == std::string("true"))     ||
       (sValue == std::string("on"))       ||
       (sValue == std::string("enable"))) return TCLPLUS::kfTRUE;
  else if ( (sValue == std::string("false"))   ||
	    (sValue == std::string("off"))     ||
	    (sValue == std::string("disable"))) return TCLPLUS::kfFALSE;
  else 
    throw "Invalid boolean value.";
 
}
