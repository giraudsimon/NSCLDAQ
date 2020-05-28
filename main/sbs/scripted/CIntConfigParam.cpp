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

static const char* Copyright = "(C) Copyright Ron Fox 2002, All rights reserved";
/*! \class CIntConfigParam   
           CLASS_PACKAGE
           Represents an integer configuration parameter
           Integer configuration parametes appear as a keyword value
           pair.  E.g.:
           \verbatim
           -vsn 5
           \endverbatim
           might set a FERA virtual slot number to 5.
           
*/

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CIntConfigParam.h"    
#include <TCLInterpreter.h>
#include <TCLResult.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


using namespace std;


/*!
   Constructor.  Creates an integer configuration parameter.
   This version of the constructor creates a parameter that
   is not range checked.
   
   \param sName const string& [in]
                keyword by which the parameter is recognized.
   \param nDefault int [in]
                 Default (initial) value of the parameter.
*/
CIntConfigParam::CIntConfigParam (const string& rName,
                                  int           nDefault) :
      CConfigurationParameter(rName),
      m_fCheckrange(false),
      m_nValue(nDefault)
{
  char svalue[100];
  sprintf(svalue, "%d", nDefault);
  setValue(string(svalue));
} 

/*!
    Constructor:  This version of the construtor creates
    a range checked parameter.  Range checked parameters refuse
    to set themselves outside of the range [m_nLow, m_nHigh].

  \param rName   const string& [in]
                  Name of the parameter to set.
  \param nLow int [in] 
                Low end of the range.
  \param nHigh int [in]
                High end of the range.
  \param nDefault int [in]
                Default (initial) parameter value.
*/
CIntConfigParam::CIntConfigParam(const string& rName,
                                 int           nLow,
                                 int           nHigh,
                                 int           nDefault) :
  CConfigurationParameter(rName),
  m_fCheckrange(true),
  m_nLow(nLow),
  m_nHigh(nHigh),
  m_nValue(nDefault)
{
}
/*!
    Destructor
*/
 CIntConfigParam::~CIntConfigParam ( )  //Destructor - Delete dynamic objects
{
}


/*!
    Copy constructor.
*/
CIntConfigParam::CIntConfigParam (const CIntConfigParam& rhs ) 
  : CConfigurationParameter (rhs),
    m_fCheckrange(rhs.m_fCheckrange),
    m_nLow(rhs.m_nLow),
    m_nHigh(rhs.m_nHigh),
    m_nValue(rhs.m_nValue)
{ 
} 

/*
   Assignment.  This is the lhs of the assignment, the 
   parameter the rhs.
  
  \param rhs const CIntConfigParam& [in]
          this gets assigned to rhs.
  \return reference to this.
*/
CIntConfigParam& CIntConfigParam::operator= (const CIntConfigParam& rhs)
{
  if(this != &rhs) {
    CConfigurationParameter::operator=(rhs);
    m_fCheckrange  = rhs.m_fCheckrange;
    m_nLow         = rhs.m_nLow;
    m_nHigh        = rhs.m_nHigh;
    m_nValue       = rhs.m_nValue;
  }
  return *this;
}

/*!
    Equality comparison.  Objects are equal iff
    all members are equal.
  \param rhs  const CIntConfigParam& [in] 
            Object we compare to.
  \return Any of:
      - true   The objects are functionally equal.
      - false  The objects are no functionally equal.
*/
int CIntConfigParam::operator== (const CIntConfigParam& rhs) const
{ 
  return (CConfigurationParameter::operator==(rhs)     &&
          (m_fCheckrange == rhs.m_fCheckrange)         &&
          (m_nLow        == rhs.m_nLow)                &&
          (m_nHigh       == rhs.m_nHigh)               &&
          (m_nValue      == rhs.m_nValue));
}

// Functions for class CIntConfigParam

/*!  

Returns the integer value of the parameter.  Note
that construction will establish a well defined default
parameter value.

  \return The value of the parameter.
*/
int 
CIntConfigParam::getOptionValue()  
{ 
    return m_nValue;
}  

/*!  

Sets the parameter value according to the next parameter
on the command line. Errors can result if:
- The value does not parse as an integer.
- The value parses as an integer, however it is outside
   of the range [m_nLow, m_nHigh] and m_fCheckrange is true.

\param rInterp - CTCLInterpreter& [in]
              The interpreter that is setting the parameter.
\param rResult - CTCLResult& [in]
              The result string that will be set in case
              of error.
\param Value   char* [in]
            The C null terminated string containing the
            potential value of the parameter.
\return 
    - TCL_OK normal completion.
    - TCL_ERROR if one of the errors described above is encountered.  In this case,
      the result string will be the error message.

*/
int 
CIntConfigParam::SetValue(CTCLInterpreter& rInterp, 
                          CTCLResult& rResult, 
                          const char* pValue)  
{
  TCLPLUS::Long_t nNewValue;
  long long llnewvalue = strtoll(pValue, NULL, 0);
  if((llnewvalue == 0) && (errno == EINVAL)) {
    string Result; 
    Result += "Attempt to configure integer parameter ";
    Result += getSwitch();
    Result += "  with non integer value: ";
    Result += pValue;
    rResult.AppendElement(Result);
    return TCL_ERROR;
  }
  else {
    nNewValue = llnewvalue;
  }
  if(m_fCheckrange) {
    if((nNewValue < m_nLow) || 
       (nNewValue > m_nHigh)) {
      char RangeString[500];
      string Result;
      Result += " Attempt to configure integer parameter ";
      Result += getSwitch();
      Result += " with an integer value out of valid range: ";
      Result += pValue;
      sprintf(RangeString, "[%d, %d]\n", m_nLow, m_nHigh);
      Result += RangeString;
      rResult.AppendElement(Result);
      return TCL_ERROR;
    }
  }
  m_nValue = nNewValue;
  return TCL_OK;
}
/*!
   \return Returns the format of the parameter.  In this case
           int
*/
string
CIntConfigParam::GetParameterFormat()
{
  return string("int");
}








