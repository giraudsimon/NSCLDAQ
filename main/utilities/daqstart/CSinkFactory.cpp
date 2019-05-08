   

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

using namespace std;



// Static attribute storage and initialization for CSinkFactory


// The map below holds the set of sink creators that have been
// registered:

CSinkFactory::SinkCreatorDictionary CSinkFactory::m_SinkCreators;


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

  CSinkCreator* pSinkCreator = LocateCreator(sType);
  if(!pSinkCreator) {
    return (CSink*)NULL;
  }

  // Ask if the sink name is legal...

  if(pSinkCreator->isNameLegal(sName)) {
    return pSinkCreator->Create(sCommand,
				sName);	// Legal sink name.
  }
  else {
    return (CSink*)NULL;	// Illegal sink name.
  }
}  

/*! 

Description:

Returns a pointer to the sink creator that will create the sink of type
requested by the caller:


Parameters:

\param sType (const string [in])
      Type of string to create.

\return CSinkCreator*
\retval 0  - String creation failed.
\retval != 0 - Pointer to the  new sink.

*/
CSinkCreator* 
CSinkFactory::LocateCreator(string sType)  
{
  SinkIterator i = m_SinkCreators.find(sType);
  if(i == m_SinkCreators.end()) {
    return (CSinkCreator*)NULL;	// No such sink creator.
  }
  return i->second;

}  

/*! 



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
  m_SinkCreators[sType] = pSink;
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
