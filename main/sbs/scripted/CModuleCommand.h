/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

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
Executes the module command.  The module command has the following form:
module newname type ?configuration data?
module -list ?pattern
module -delete name
module -types

The module command relies on the recognizer pattern.
m_Creators is a list of module type creators.  The creational
form of the module command iterates through the set of
creators looking for one that matches the type keword.
When one is found it is used to create the actual module.

 This object is a singleton object.
*/
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu

#ifndef CMODULECOMMAND_H  //Required for current class
#define CMODULECOMMAND_H


#include <CDigitizerDictionary.h>
#include <TCLProcessor.h>
#include <string>
#include <list>
#include <map>
#include <assert.h>

// forward class definitions:

class CModuleCreator;
class CTCLInterpreter;
class CTCLResult;
class CReadOrder;
 

class CModuleCommand  : public CTCLProcessor
{
	// Type definitions:
public:
typedef std::list<CModuleCreator*>  CreatorList;
typedef CreatorList::iterator  CreatorIterator;


private:
  
  // Private Member data:
   CDigitizerDictionary*           m_pModules;  //!< Created module lookup. 
   CreatorList                     m_Creators; //!< List of Creators.


private:
// Helper classes.

// Function class to build up the output of module -list:
  
  class ListGatherer
  {
  private:
    CTCLResult& m_rResult;
    const char*       m_pMatch;
  public:
    ListGatherer(CTCLResult& rResult, const char* pMatch) :
      m_rResult(rResult),
      m_pMatch(pMatch)
    {}
    void operator()(std::pair<std::string, CReadableObject*> p);

  };
  // Function class to build up the output of module -types:
  class TypesGatherer
  {
  private:
    CTCLResult& m_rResult;
  public:
    TypesGatherer(CTCLResult& rResult) :
      m_rResult(rResult)
    {}
    void operator()(CModuleCreator* pModule);

  };
  

public:
     // Constructors and other canonical functions:
     
   CModuleCommand (CTCLInterpreter* pInterp,
		   CDigitizerDictionary* pDictionary,
		   const std::string& command = std::string("module"));
   virtual ~CModuleCommand ( ); 
private:
   CModuleCommand (const CModuleCommand& aCModuleCommand );
   CModuleCommand& operator= (const CModuleCommand& rhs);
   int operator== (const CModuleCommand& rhs) const;
   int operator!= (const CModuleCommand& rhs) const;
public:

public:
// Selectors:
   CDigitizerDictionary& getModules()
   { 
     return *m_pModules;
   }
   CreatorList getCreators() const
   {
      return m_Creators;
   }

protected:
   // Mutators:

   void setModules(CDigitizerDictionary* pModules)
   {
      m_pModules = pModules;
   }
   void setCreators(const CreatorList& creators)
   {
      m_Creators = creators;
   }
public:

   virtual int operator() (CTCLInterpreter& rInterp, 
			   CTCLResult& rResult, 
			   int nArgs, char** pArgs); 
   void AddCreator (CModuleCreator* pCreator);
   CreatorIterator CreatorBegin();
   CreatorIterator CreatorEnd();
   int             CreatorSize();
   
   CDigitizerDictionary::ModuleIterator DigitizerBegin();
   CDigitizerDictionary::ModuleIterator DigitizerEnd();
   int                                  DigitizerSize();
   CDigitizerDictionary::ModuleIterator DigitizerFind(const std::string& rName)
   {
	return m_pModules->DigitizerFind(rName);
   }
   std::string            Usage();

 protected:
      // Protected functions: these will be called from
      // within the comand parser:

   int Create (CTCLInterpreter& rInterp, CTCLResult& rResult,
	       int  nArgs, char** pArgs);
   int List (CTCLInterpreter& rInterp, CTCLResult& rResult, 
	     int nArgs, char** pArgs);
   int Delete (CTCLInterpreter& rInterp, CTCLResult& rResult, 
	       int nArgs, char** pArgs);
   int ListTypes (CTCLInterpreter& rinterp, 
		  CTCLResult& rResult, 
		  int nArgs, char** pArgs); 

  CreatorIterator FindCreator(const std::string& ModuleType);
};

#endif
