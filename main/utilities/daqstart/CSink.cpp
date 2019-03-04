// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 
/*
	Implementation file for CSink for a description of the
	class see CSink.h
*/

////////////////////////// FILE_NAME.cpp ///////////////////////////

// Include files required:

#include <config.h>

#include "CSink.h"    				

#include <time.h>
#include <string.h>
#include <stdlib.h>


using namespace std;


/*!
    Create an object of type CSink
*/
CSink::CSink (string sLine)
   : m_sCommand(sLine)
 
{ 
} 

/*!
    Called to destroy an instance of CSink
*/
 CSink::~CSink ( )
{
}
/*!
   Called to create an instance of CSink that is a
   functional duplicate of another instance.
   \param rSource (const CSink& ):
      The object that we will dupliate.
*/
CSink::CSink (const CSink& aCSink ) :
  m_sCommand(aCSink.m_sCommand)
{

} 
/*!
  Assign to *this from rhs so that *this becomes a functional
  duplicate of rhs.
  \param rhs (const CSink& rhs ):
     The object that will be functionally copied to *this.
 */
CSink& CSink::operator= (const CSink& rhs)
{ 
  if(this != &rhs) {
    m_sCommand = rhs.m_sCommand;
  }
  return *this;

}
/*!
  Compare *this for functional equality with another object of
  type CSink.
  \param rhs (const CSink& rhs ):
     The object to be compared with *this.

 */
int 
CSink::operator== (const CSink& rhs) const
{
  return m_sCommand == rhs.m_sCommand;
}


/*! 

Description:

Overridable function that takes a raw line of text and produces
the default form of a logfile line.  The logfile line looks like:

[timestamp] [command] message



Parameters:

\param sMessage (const string& [in])
    The base message that is decorated with the timestampe and the command string information.
    If the caller wants a host identifier in the command they must put it in.

*/
string 
CSink::FormatLine(string sMessage)  
{ 
  string output;		// Output string gets built up here.

  // Get the timestamp appended:

  time_t t;
  time(&t);
  output += '[';
  char* pTime = strdup(ctime(&t));
  pTime[strlen(pTime)-1] = '\0'; // Kill \n.
  output += pTime;
  free(pTime);
  output += "] ";
  
  // Put in the command string too:

  output += '[';
  output += m_sCommand;
  output += "] ";

  // Finally put in the client's message and return:

  output += sMessage;

  return output;
}

