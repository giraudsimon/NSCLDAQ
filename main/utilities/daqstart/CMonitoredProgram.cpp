/*
	Implementation file for CMonitoredProgram for a description of the
	class see CMonitoredProgram.h
*/

////////////////////////// FILE_NAME.cpp ///////////////////////////

// Include files required:

#include <config.h>

#include "CMonitoredProgram.h"    				
#include "CSink.h"

#include <iostream>


#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>

using namespace std;


// Static attribute storage and initialization for CMonitoredProgram

/*!
    Create an object of type CMonitoredProgram
    \param argc (int)
      The count of command line paramters that the program monitored
      would have if started from the command line (e.g. argv[0] is
      the path to the program.
    \param argv (char**)
      The command line parameters as if the program monitored would
      have started from the command line (e.g. argv[0] is the
      path to the program.

Both argc, and argv must stay in scope for the duration of the
lifetime of this.
*/
CMonitoredProgram::CMonitoredProgram (int argc,
				      char** argv)
   : m_nExitStatus(0),
     m_sProgramName(string(argv[0])),
     m_nArgc(0),
     m_paArgv(0),
     m_pOutput(0),
     m_pError(0)
 
{ 
  CopyArgv(argc, argv);
} 

/*!
    Called to destroy an instance of CMonitoredProgram
    The m_paArgv array is dynamically allocated and must be freed:

*/
CMonitoredProgram::~CMonitoredProgram ( )
{

  FreeArgv();

}
/*!
   Called to create an instance of CMonitoredProgram that is a
   functional duplicate of another instance.
   \param rSource (const CMonitoredProgram& ):
      The object that we will dupliate.
*/
CMonitoredProgram::CMonitoredProgram (const CMonitoredProgram& aCMonitoredProgram )
{
  // There's no previously existing data so:

  CopyIn(aCMonitoredProgram);
} 
/*!
  Assign to *this from rhs so that *this becomes a functional
  duplicate of rhs.
  \param rhs (const CMonitoredProgram& rhs ):
     The object that will be functionally copied to *this.
 */
CMonitoredProgram& CMonitoredProgram::operator= (const CMonitoredProgram& rhs)
{ 
  if(this != &rhs) {
    // We already exist so first get rid of what we have:

    delete m_pOutput;
    delete m_pError;
    FreeArgv();

    // Then copy in:

    CopyIn(rhs);
    
  }
  return *this;

}
/*!
  Compare *this for functional equality with another object of
  type CMonitoredProgram.
  \param rhs (const CMonitoredProgram& rhs ):
     The object to be compared with *this.

 */
int 
CMonitoredProgram::operator== (const CMonitoredProgram& rhs) const
{
  return ((m_sProgramName == rhs.m_sProgramName)      &&
	  ArgvSame(rhs.m_nArgc, rhs.m_paArgv)         &&
	  (*m_pOutput == *(rhs.m_pOutput))            &&
	  (*m_pError  == *(rhs.m_pError)));
  
}

// Functions for class CMonitoredProgram

/*! 

Description:

Returns the value of the final status 
variable.  This is a pure selector and
therefore does not discriminate between
valid exit statuses that were set by e.g. PollStatus
and invalid/uninitialized status values.


\retval  0   If valid, the subprocess probably completed
                 normally.
\retval !0  If valid probably the subprocess completed
                 abnormally.
*/
int 
CMonitoredProgram::GetFinalStatus()  const
{
  return m_nExitStatus;
}  

/*! 

Description:

Returns the value of m_sProgram name.  
This function is a pure selector.


\return string
\retval m_sProgramName - the name of the program that
         is monitored by this object.


*/
string 
CMonitoredProgram::GetName()  const
{
  return m_sProgramName;
}  

/*! 

Description:

Pure selector for m_nArgc - the number of arguments.


Parameters:

\return int
\retval m_nArgc



*/
int 
CMonitoredProgram::GetNargs()  const
{
  return m_nArgc;
}  

/*! 

Pure selector for m_paArgv.

\return char**
\retval m_paArgv

*/
char** 
CMonitoredProgram::GetArgs()  const
{
  return m_paArgv;
}  

/*! 

Description:

This is a pure mutator that sets the
value of the m_nExitStatus variable.  It
is used by subclass PollStatus functions to
set the final exit status when process exit is
detected.
\param nStatus (int)
   The final status value


*/
void 
CMonitoredProgram::SetFinalStatus(int nStatus)
{
  m_nExitStatus = nStatus;
}  

/*! 

Description:

Attach the output sink to the constructed 
object.


Parameters:

\param pSink (CSink* [in[)
   The sink to attach as the output
   sinkl.


\return CSink*
 \retval NULL There was no previously attached output sink.
 \retval !NULL There was a previously attached output sink and
                        this is the pointer to it.

*/
CSink* 
CMonitoredProgram::AttachOutput(CSink* pSink)  
{
  CSink* pOld = m_pOutput;
  m_pOutput   = pSink;

  return pOld;
}  

/*! 

Attaches the error sink to the
fully constructed object.



\param pSink (CSink*)
   Pointer to the sink to attach.


\return CSink*
\retval NULL - Thre was no previously attached sink.
\retval !NULL- there was a previously attached sink,
                       returned value points to it.


*/
CSink* 
CMonitoredProgram::AttachError(CSink* pSink)  
{ 
  CSink *pOld = m_pError;
  m_pError    = pSink;
  return pOld;
}  



/*! 

Description:

This is a utility function that is called to 
indicate to the base class that output
on stdout is available.  The output is
relayed as is to stdout.  If the monitor has
a sink for stdout, the output is sent to
it as well for logging.  

Parameters:

\param line (string)
    The line of text that has been received from the monitored program.
    It is the responsibility of the subclass to put any required newlines 
   on this message as the text is sent as is to stdout and modified
   only as the sink would modify it for the sinks.


*/
void 
CMonitoredProgram::StdOut(string line)  
{ 
  Log(STDOUT_FILENO, m_pOutput, line);
}  

/*! 

Description:

This function is identical to StdOut, but the 
message is relayed to both stderr and the
stderr monitor if one is defined.


Parameters:

\param line (string)
   The line to write.


\return void

*/
void 
CMonitoredProgram::StdErr(string line)  
{
  Log(STDERR_FILENO, m_pError, line);
}
/*!
   Free the argv array:
    The first m_nArgc elements are malloce'd by strdup and must
    be free'd.  The array of pointers itself must be delete'd as
    an array:
*/
void
CMonitoredProgram::FreeArgv()
{
  for(int i =0; i < m_nArgc; i++) {
    free(m_paArgv[i]);		// These came from strdup so free() not delete
  }
  delete []m_paArgv;		// This came from a new, so delete not free()
}
/*!
  Copy a set of argv's into this's.  The assumption is that
  at this time, m_nArgc, and m_paArgv are not defined.
  \param argc (int)
    Number of parameters to copy in.
  \param argv (char**)
    Pointers to the parameters themselves.
*/
void
CMonitoredProgram::CopyArgv(int argc, char** argv) 
{
  m_nArgc = argc;
  // Copy in argv and the strings it points to.

  m_paArgv = new char*[m_nArgc+1]; // The array of pointers.
                                   // including an extra for a trailing null
  memset(m_paArgv, 0, sizeof(char*)*(m_nArgc+1)); // Set em all to null.

  for(int i = 0; i < m_nArgc; i++) {
    m_paArgv[i] = strdup(argv[i]);
  }
}
/*!
  Copy into this from another rhs.  The caller must have taken
  care of any previously existing argv by calling FreeArgv, as
  well as freeing any other resources.
  
  \param rhs (const CMonitoredProgram&)
    They object we're duplicating into this.

*/
void
CMonitoredProgram::CopyIn(const CMonitoredProgram& rhs) 
{
  m_nExitStatus = rhs.m_nExitStatus;
  m_sProgramName= rhs.m_sProgramName;
  CopyArgv(rhs.m_nArgc, rhs.m_paArgv);
  m_pOutput     = rhs.m_pOutput->clone();
  m_pError      = rhs.m_pError->clone();
}
/*!
   Comomn output logging code.   The output line is written
   to fd.  If pSink is not NULL it is also logged to the sink.
   \param fd (int)
     File descriptor that output is unconditionally written to.
   \param pSink (CSink*)
     Sink pointer.  If non-null, output is passed to that sink's
     Log function.
   \param sLine (string)
     The string to output, gathered from the program.
*/
void
CMonitoredProgram::Log(int fd, CSink* pSink, string sLine)
{
  try {
    io::writeData(fd, sLine.c_str(), sLine.size());
  }
  catch (int e) {
    if (e != 0) {
      cerr << "Unable to write a log entry " << strerror(e) << endl;
    } else {
      cerr << "end of file on output\n";
    }
  }
  if(pSink) {
    pSink->Log(sLine);
  }
}
/*!
  Compare the argv's of another object to ours.  They must
  agree in number and contents to agree.
  \param argc  (int)
     Count of arguments in rhs.
  \param argv (char**)
     Arguments in rhs.
    
  \return bool
  \retval true  argc,argv are == to m_paArgv, m_nArgc
  \retval false argc,argv are != to m_paArgv, m_nArgc.
*/
bool
CMonitoredProgram::ArgvSame(int argc, char**argv) const
{
  if(argc == m_nArgc) {
    for(int i=0; i < argc; i++) {
      if(strcmp(argv[i], m_paArgv[i])) {
	return false;
      }
    }
    return true;
  }
  else {
    return false;
  }
}
