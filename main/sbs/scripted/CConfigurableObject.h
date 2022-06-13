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



/*!

Base class of all objects that have a TCL configurable
 configuration. The configuration object autonomously processes the
config an cget subcommands to maintain a configuration parameter 
database.  Configuration consists of a set of configuration parameter 
objects.

Each of these represents a keyword/value pair. 

*/

// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef SCONFIGURABLEOBJECT_H  //Required for current class
#define SCONFIGURABLEOBJECT_H

//
// Include files:
//

#include <TCLProcessor.h>
#include <TCLResult.h>        //Required for include files  
#include <list>
#include <string>


// forward definitions. 

class CConfigurationParameter;
class CTCLInterpreter;
class CTCLResult;
class CIntArrayParam;


class SConfigurableObject : public  CTCLProcessor     
{
  // Public data types.
public:
  typedef std::list<CConfigurationParameter*> ConfigArray;
  typedef ConfigArray::iterator          ParameterIterator;
private:
  
  std::string          m_sName;	//!< Name of the command associated with the object.
  ConfigArray     m_Configuration; //!< The configuration.


  // Constructors and other canonical operations.
public:
  SConfigurableObject (const std::string& rName,
		       CTCLInterpreter& rInterp);
  virtual  ~ SConfigurableObject ( );  

  // The copy like operations are not supported on tcl command processing
  // objects:
private:
  SConfigurableObject (const SConfigurableObject& aSConfigurableObject );
  SConfigurableObject& operator= (const SConfigurableObject& aSConfigurableObject);
  int operator== (const SConfigurableObject& aSConfigurableObject) const;
public:

  // Selectors:

  //!  Retrieve a copy of the name:

  std::string getName() const
  { 
    return m_sName;
  }   



  // Member functions:

public:

  virtual  int      operator() (CTCLInterpreter& rInterp, 
				CTCLResult& rResult, 
				int nArgs, char** pArgs)   ; //!< Process commands.
  virtual  int      Configure (CTCLInterpreter& rInterp, 
			       CTCLResult& rResult, 
			       int nArgs, char** pArgs)   ; //!< config subcommand 
  virtual  int      ListConfiguration (CTCLInterpreter& rInterp, 
				       CTCLResult& rResult, 
				       int nArgs, char** pArgs); //!< list subcommand 
  ParameterIterator AddParameter(CConfigurationParameter* pConfigParam);
  ParameterIterator AddIntParam (const std::string& sParamName, 
				 int nDefault=0)   ; //!< Create an int.
  ParameterIterator AddBoolParam (const std::string& rName,
				  bool          fDefault)   ; //!< Create a boolean. 
  ParameterIterator AddStringParam (const std::string& rName)   ; //!< Create string param. 
  ParameterIterator AddIntArrayParam (const std::string&  rParameterName, 
				      int nArraySize, 
				      int nDefault=0)   ; //!< Create array of ints.
  ParameterIterator AddStringArrayParam (const std::string& rName, 
					 int nArraySize)   ; //!< Create array of strings.
  ParameterIterator AddEnumParam(std::string name,
				 std::vector<std::pair<std::string, int> > values,
				 std::string defaultValue);
  ParameterIterator Find (const std::string& rKeyword)   ; //!< Find a param 
  ParameterIterator begin ()   ; //!< Config param start iterator.
  ParameterIterator end ()   ;   //!< Config param end iterator.
  int size ()   ;                //!< Config param number of items.
  std::string ListParameters (const std::string& rPattern)   ; //!< List configuration 
  std::string ListKeywords ()   ;     //!< List keyword/type pairs.

  // Incomplete access to some configuratin parameter types by name.

  int getIntegerValue(std::string name); //!< Get value of an integer config param.
  bool getBoolValue(std::string name); //!< Get value of a bollean config param.
  CIntArrayParam* getIntArray(std::string name); //!< Get ptr to int array param.
  int             getEnumValue(std::string name);
protected:
  std::string Usage();
private:
  void              DeleteParameters ()   ; //!< Delete all parameters. 
  ParameterIterator FindOrThrow(std::string name, std::string type);
  
  
};

#endif
