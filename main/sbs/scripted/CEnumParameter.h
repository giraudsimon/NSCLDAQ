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


#ifndef __CENUMPARAMETER_H
#define __CENUMPARAMETER_H

#ifndef __CCONFIGURATIONPARAMETER_H
#include <CConfigurationParameter.h>
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif


#ifndef STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

class CTCLInterpreter;
class CTCLResult;


/*!
    Defines a configuration parameter that is an 'enumerator'  An
    enumerator in this case is a limited set of text keywords that
    map to integer values.
*/
class CEnumParameter : public CConfigurationParameter {
private:
  std::map<std::string, int> m_textToValue;
public:
  struct enumeratorValue {
    std::string    s_name;
    int            s_value;
    enumeratorValue(std::string name, int value) :
      s_name(name), s_value(value) {}
  }; 
public:
  CEnumParameter(std::string keyword,
		 std::vector<CEnumParameter::enumeratorValue> values,
		 std::string defaultValue);
  CEnumParameter(const CEnumParameter& rhs);
  virtual ~CEnumParameter();

  CEnumParameter& operator=(const CEnumParameter& rhs);
  int operator==(const CEnumParameter& rhs) const;
  int operator!=(const CEnumParameter& rhs) const {
    return !(*this == rhs);
  }

  // Overrides:

public:
  virtual int SetValue(CTCLInterpreter& rInterp, CTCLResult& rResult,
		       const char* pValue);
  virtual std::string GetParameterFormat();
  int     GetEnumValue();

protected:
  bool checkValue(std::string newValue) const;

};


#endif
