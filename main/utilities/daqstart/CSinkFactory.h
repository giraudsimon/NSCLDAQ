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



#ifndef CSINKFACTORY_H  //Required for current class
#define CSINKFACTORY_H

//
// Include files:
//

//   STL String class.

#include <string>
#include <map>



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
