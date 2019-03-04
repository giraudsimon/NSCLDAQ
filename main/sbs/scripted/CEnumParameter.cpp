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

#include <config.h>

#include "CEnumParameter.h"
#include <TCLInterpreter.h>
#include <TCLResult.h>



using namespace std;

/////////////////////////////////////////////////////////////////////////
/*!
   Construct the configuration parameter.
   - Construct the base class from the name.
   - Build the text to value map.
   - Set the current value to the defafult.
   \param keyword : string [in]
         Name of the configuration parameter.
   \param values  : vector<enumeratorValue> [in]
         Array of name/value pairs.
   \param defaultValue :string [in]
         Default value of the parameter.

*/
CEnumParameter::CEnumParameter(string keyword,
			       vector<CEnumParameter::enumeratorValue> values,
			       string defaultValue) :
  CConfigurationParameter(keyword)
{
  // Fill in the name : value map:

  for (int i =0; i < values.size(); i++) {
    m_textToValue[values[i].s_name] = values[i].s_value;
  }
  // Set the default value:

  if (checkValue(defaultValue)) {
    setValue(defaultValue);
  } else {
    // Worth an exception if the default value is not a legal 
    // enum value.
    //
    string msg("Invalid default value: ");
    msg += defaultValue;
    msg += " for parameter: ";
    msg += keyword;
    throw msg;
  }
}
/////////////////////////////////////////////////////////////////////////
/*!
   Destructor : nop for now.
*/
CEnumParameter::~CEnumParameter()
{
}
/////////////////////////////////////////////////////////////////////////
/*!
    Copy construct:
    \param rhs : const CEnumParameter& 
        Template from which *this is constructed.
*/
CEnumParameter::CEnumParameter(const CEnumParameter& rhs) :
  CConfigurationParameter(rhs),
  m_textToValue(rhs.m_textToValue)
{

}
/////////////////////////////////////////////////////////////////////////
/*!
    Assignment.  Assign to *this from rhs and return *this.
    \param rhs   : const CEnumParameter& 
        Parameter from which we assign to *this.
    \return CEnumParameter&
    \retval *this
*/
CEnumParameter&
CEnumParameter::operator=(const CEnumParameter& rhs)
{
  if (this != &rhs) {
    CConfigurationParameter::operator=(rhs);
    m_textToValue = rhs.m_textToValue;
  }
  return *this;
}
////////////////////////////////////////////////////////////////////////
/*!
   equality comparison.
   \param rhs : const CEnumParameter&
   \return int
   \retval 0   - Not equal.
   \retval 1   - equal.
*/
int
CEnumParameter::operator==(const CEnumParameter& rhs) const
{
  return ((CConfigurationParameter::operator==(rhs))     &&
	  (m_textToValue == rhs.m_textToValue));
}

/////////////////////////////////////////////////////////////////////////
/*!
    Callout to determine if setting the value is legal.
    - If the stringis not in the m_textToValue map TCL_ERROR
      is returned along with a string indicating that this is
      an invalid value along with a list of the valid values.
    - Return TCL_OK otherwise.

    \param rInterp : CTCLInterpreter&
        Refers to the interpreter on which the object is being configured.
    \param rResult : CTCLResult& [out]
        Result string.
    \param value : const char*
        Proposed new value for the item.

    \return int
    \retval TCL_OK    - value is legal.
    \retval TCL_ERROR - value is not legal.
*/
int
CEnumParameter::SetValue(CTCLInterpreter& rInterp, CTCLResult& rResult,
			 const char* pValue)
{
  string sValue(pValue);
  if(checkValue(sValue)) {
    return TCL_OK;
  } 
  else {
    rResult  = "Invalid value: ";
    rResult += sValue;
    rResult += " for configuration parameter: ";
    rResult += getSwitch();
    rResult += " legal values are: ";
    rResult += GetParameterFormat();
    return TCL_ERROR;
  }
}
////////////////////////////////////////////////////////////////////////////
/*!
     Get the parameter format.  This is just a | separated list of the
     keywords recognized.  We pull these keywords from a the map so
     they will be automatically alphabetized.

     \return string
     \retval See above.

     \bug This implementation does not remember the default.
*/
string
CEnumParameter::GetParameterFormat() 
{
  map<string, int>::iterator i = m_textToValue.begin();
  
  // Assuming this is not empty!!!

  string result;
  result += i->first;
  i++;
  while(i != m_textToValue.end()) {
    result += " | ";
    result += i->first;
    i++;
  }
  return result;
}
/////////////////////////////////////////////////////////////////////////
/*!
    Get the integer value that corresponds to the current
    enum text value.

    \return int
    \retval see above.
*/
int
CEnumParameter::GetEnumValue()
{
  string svalue = getOptionString();
  return m_textToValue[svalue];
}
///////////////////////////////////////////////////////////////////////
/*
  [protected]

   Determine if a value is a legal enum.

   Parameters:
       newValue : string
          Candidate new value for the enum.

   Returns:
      bool
           true  - The value is one of the enum values.
	   false - The value is not one of the enum values.
*/
bool
CEnumParameter::checkValue(string newValue) const
{
  map<string, int>::const_iterator i = m_textToValue.find(newValue);
  return (i != m_textToValue.end());
}
