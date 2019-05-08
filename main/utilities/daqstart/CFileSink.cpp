/*
	Implementation file for CFileSink for a description of the
	class see CFileSink.h
*/

////////////////////////// FILE_NAME.cpp ///////////////////////////

// Include files required:

#include <config.h>

#include <iostream>

#include "CFileSink.h"    				
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>


#include <ErrnoException.h>


using namespace std;


// Static attribute storage and initialization for CFileSink

/*!
    Create an object of type CFileSink
    \param sCommand  string
     The command name of the thing being logged (required by
     base class constructor.
    \param sFilename string
     The name of the logfile. This file is opened at construction
     time and closed at destruction.
  
     \throw CErrnoException if the file cannot be opened.
*/
CFileSink::CFileSink (string sCommand,
		      string sFilename)
  : CSink(sCommand),
    m_nFd(-1)
{
  // Open the file for append, creation is allowed.

  int nFd = open(sFilename.c_str(),
		 O_CREAT | O_APPEND | O_WRONLY,
		 S_IRUSR | S_IWUSR  |
		 S_IRGRP | S_IWGRP  |
		 S_IROTH);

  if(nFd < 0) {
    throw CErrnoException(string("Opening log file ") + sFilename);
  }

  m_nFd = nFd;
} 

/*!
    Called to destroy an instance of CFileSink
*/
 CFileSink::~CFileSink ( )
{
  close(m_nFd);			// Must be open if fully constructed
}
/*!
   Called to create an instance of CFileSink that is a
   functional duplicate of another instance.
   \param rSource (const CFileSink& ):
      The object that we will dupliate.
*/
CFileSink::CFileSink (const CFileSink& aCFileSink ) 
  : CSink (aCFileSink) 
{
  m_nFd = dup(aCFileSink.m_nFd);
} 
/*!
  Assign to *this from rhs so that *this becomes a functional
  duplicate of rhs.
  \param rhs (const CFileSink& rhs ):
     The object that will be functionally copied to *this.
 */
CFileSink& CFileSink::operator= (const CFileSink& rhs)
{ 
  if(this != &rhs) {
    CSink::operator=(rhs);
    close(m_nFd);
    m_nFd = dup(rhs.m_nFd);
    
  }
  return *this;

}
/*!
  Compare *this for functional equality with another object of
  type CFileSink.
  \param rhs (const CFileSink& rhs ):
     The object to be compared with *this.

 */
int 
CFileSink::operator== (const CFileSink& rhs) const
{
  // If I wasn't so lazy I'd stat the two files and compare
  // the stat output.

  return CSink::operator==(rhs);
}


// Functions for class CFileSink

/*! 

Description:

Logs the associated message to the file.

- FormatLine is called to produce the line to write to the file
- The log file is written with the line.
- Output buffers are flushed so that the log file is immediately updated.

Parameters:

\param Message (const string & [in]
   The line that is to be written to file.  The line will already have a \n to terminate it.




\return int
\retval  >0 The number of characters written to the file.
\retval <=0 Is an error condition.


*/
int
CFileSink::Log(const std::string& Message)  
{
  std::string line = FormatLine(Message);
  int result;
  try {
    io::writeData(m_nFd, line.c_str(), line.size());
  }
  catch (int er) {
    if (er != 0) {
      cerr << "Unable to completly log a file line: " << strerror(er) << endl;
    } else {
      cerr << "End file on output in log\n";
    }
    exit(EXIT_FAILURE);
  }
  return Message.size();
  
}
/*!
   Create a functional duplicate of *this:
   \return CSink*
   \retval new'd copy of this.
*/
CSink*
CFileSink::clone()
{
  return new CFileSink(*this);
}
