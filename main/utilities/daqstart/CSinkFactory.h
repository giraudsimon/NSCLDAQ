// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

/*!
  \class CSinkFactory

  Factory class that creates sinks.  All member functions are static
  as we rely a list of creators to do the dirty work for us.

*/



#ifndef __CSINKFACTORY_H  //Required for current class
#define __CSINKFACTORY_H

//
// Include files:
//

//   STL String class.

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

//  STL map class - holds the creators:

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif



// Forward classes

class CSinkCreator;
class CSink;

// CSinkFactory definition:

class CSinkFactory      
{
  // Type definitions:
public:
  typedef std::map<std::string, CSinkCreator*> SinkCreatorDictionary;
  typedef SinkCreatorDictionary::iterator SinkIterator;
private:
  
  static SinkCreatorDictionary   m_SinkCreators;


public:


public:

  static  CSink* Create (std::string  sType, 
			 std::string sCommand, std::string  sName)   ; 
  static  void AddCreator (std::string sType, CSinkCreator* pSink)   ; 
  static  int  SplitName(char* sName, char** ppParts);

protected:
  static  CSinkCreator* LocateCreator (std::string sType)   ; 
  


};

#endif
