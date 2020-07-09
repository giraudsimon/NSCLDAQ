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
#include <CExtensibleFactory.h>


// Forward classes

class CSinkCreator;
class CSink;

// We're encapsulating this type of factory:

using SinkFactory = CExtensibleFactory<CSink>;

// CSinkFactory definition:
//  daqdev/NSCLDAQ#510 - recast as a singleton wrapping an extensible factory.

class CSinkFactory  
{
  
private:
  static CSinkFactory* m_pInstance;  
  SinkFactory m_factory;

  // Con/destructors must be private for singletons:
private:
  CSinkFactory() {}
  ~CSinkFactory() {}


public:
  static CSinkFactory* getInstance();
  CSink* Create (std::string  sType, 
			 std::string sCommand, std::string  sName)   ; 
  void AddCreator (std::string sType, CSinkCreator* pSink);
  static  int  SplitName(char* sName, char** ppParts);


  


};

#endif
