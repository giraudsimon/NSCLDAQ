   

/*
	Implementation file for CSinkFactory for a description of the
	class see CSinkFactory.h
*/

////////////////////////// FILE_NAME.cpp ///////////////////////////

// Include files required:

#include <config.h>

#include "CSinkFactory.h"    				
#include "CSinkCreator.h"
#include "CSink.h"

#include <string.h>
#include <utility>

using namespace std;

// Static attribute storage and initialization for CSinkFactory

CSinkFactory*  CSinkFactory::m_pInstance(nullptr);

// static methods:

/**
 * getInstance
 *  @return CSinkFactory - pointer to the singleton instance -- which is created
 *                         if necessary.
 */
CSinkFactory*
CSinkFactory::getInstance()
{
	// Not threadsafe.
	
	if (!m_pInstance) {
		m_pInstance = new CSinkFactory;
	}
	return m_pInstance;
}

// Functions for class CSinkFactory

/*! 

Description:

Creates a sink given a description of the sink to create.

Parameters:

\param type (const string& [in])
    The type of sink to create (e.g. "file").

\param name [const string& [in])
    The name of the entity the sink is connected to (e.g. for a
    socket perhaps host:port).

\return CSink*
\retval 0   - The sink could not be created.  Most likely illegal sink type 
              or could be illegal sink name.
\retval != 0 - Pointer to the newly created sink.


*/
CSink* 
CSinkFactory::Create(string  sType,
		     string sCommand, string  sName)  
{
  // Find the sink creator
	
	std::pair<const char*, const char*> userData(sCommand.c_str(), sName.c_str());
	return m_factory.create(sType, &userData);
}  


/*

Add a new sink creator to the supported set.
Note that any existing creator with the same name will
vanish from the face of the earth, possibly  causing 
memory leaks.

Parameters:

\param sType (const string  [in])
   The type of string creator to add.
\param pCreator (CSinkCreator* pSInk [in])
   The creator to add

\return void

*/
void 
CSinkFactory::AddCreator(string sType, CSinkCreator* pSink)  
{
  m_factory.addCreator(sType, pSink);
}
/*!
   Split a sink name into it's component pieces.  The pieces are
   assumed to be separated by colons.  A sink name is identified
   by a type and an optional connection name. For example:

   file:/home/fox/testing.log

   or 

   syslog:user1

   or

   logserver:

  \param sName (string):
     The name of the sink.
  \param ppParts  (Actually char *([2])).
     Storage for the pointers to the two parts.

  \return int
  \retval the number of pieces in the name (0,1 or 2).

*/
int
CSinkFactory::SplitName(char* pName, char** ppParts)
{
  // Init the ppParts array since I'm not sure strsplit does that.

  ppParts[0] = (char*)NULL;
  ppParts[1] = (char*)NULL;

  ppParts[0] = strtok(pName, ":");
  ppParts[1] = strtok(NULL, ":");

  int nItems = 0;
  if(ppParts[0]) nItems++;
  if(ppParts[1]) nItems++;
  return nItems;
}
