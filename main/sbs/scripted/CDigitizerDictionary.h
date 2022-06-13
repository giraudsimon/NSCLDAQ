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
Encapsulates a set of modules that are managed by a
CModuleCommand and can be inserted into a CReadOrder
objects.  This is really a small wrapper for 
a map<string, CReadableObject*>
*/
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//

#ifndef CDIGITIZERDICTIONARY_H  //Required for current class
#define CDIGITIZERDICTIONARY_H

//
// Include files:
//


#include <string>
#include <map>


// forward class definitions

class CReadableObject;

//

class CDigitizerDictionary      
{
public:
   typedef std::map<std::string, CReadableObject*> DigitizerMap;
   typedef DigitizerMap::iterator ModuleIterator;
private:
	DigitizerMap m_Modules;

public:
   //	
  CDigitizerDictionary ();
  ~CDigitizerDictionary ( );
private:
  CDigitizerDictionary (const CDigitizerDictionary& rhs);
  CDigitizerDictionary& operator=(const CDigitizerDictionary& rhs);
  int operator== (const CDigitizerDictionary& rhs) const;
  int operator!= (const CDigitizerDictionary& rhs) const;
public:

// Selectors:

public:
   DigitizerMap getMap() const {
      return m_Modules;
   }
// Mutators:

protected:
   void setMap(DigitizerMap& rMap) {
      m_Modules = rMap;
   }
public:

   ModuleIterator DigitizerBegin ()   ; // 
   ModuleIterator DigitizerEnd ()   ; // 
   int DigitizerSize ()   ; // 
   void DigitizerAdd (CReadableObject* pDigitizer)   ; // 
   ModuleIterator DigitizerFind (const std::string& rName)   ; // 
   void Remove(ModuleIterator p) {
      m_Modules.erase(p);
   }

   // Utility functions:
protected:
   void DestroyMap();
};

#endif
