/*
	Implementation file for CDAQStart for a description of the
	class see CDAQStart.h
*/

////////////////////////// FILE_NAME.cpp ///////////////////////////

// Include files required:


#include <config.h>

#include "CDAQStart.h"    				
#include "CMonitoredProgram.h"
#include "CLocalMonitoredProgram.h"
#include "CSink.h"
#include "CSinkFactory.h"

#include <DesignByContract.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "cmdline.h"



using namespace std;


// Static attribute storage and initialization for CDAQStart

static const int OUTPUTTIMEOUT(100); // Output timeout in ms.

/*!
    Create an object of type CDAQStart
*/
CDAQStart::CDAQStart () :
  m_pProgram(0),
  m_fNotifyExit(false),
  m_nArgc(0),
  m_pArgv(0)
{ 
} 

/*!
    Called to destroy an instance of CDAQStart
*/
 CDAQStart::~CDAQStart ( )
{
  if(m_pProgram) {
    delete m_pProgram;		// This should force exit too.
  }
}


// The copy constructor, assignment and comparisons don't make sense since
// this class represents a singleton application object.


// Functions for class CDAQStart

/*! 

Description:

Parse the program parameters.
The gengetopt parser is used to do
the actual parse.



Parameters:

\param argc (int)
    Number of command line parameters.
\param argv (char**)
    The parameters themselves.

\return gengetopt_args_info*  This is dynamically allocated ad must, if not null,
     be deleted by the caller.

\retval  NULL   - The parse failed, although I'm betting 
                  the parser exits the program too.
\retval !NULL   - The arguments parsed out into the struct.



*/
gengetopt_args_info* 
CDAQStart::ParseArgs(int argc, char** argv)  
{ 
  gengetopt_args_info* pArgs = new gengetopt_args_info;
  
  int nParseStatus = cmdline_parser(argc, argv, pArgs);
  if(nParseStatus == EXIT_FAILURE) {
    delete pArgs;
    return (gengetopt_args_info*)NULL;
  }
  else if (nParseStatus == EXIT_SUCCESS) {
    // Need to have the command line as an un-named option:
    
    if(pArgs->inputs_num < 1) {
      delete pArgs;
      return (gengetopt_args_info*)NULL;
    }

    return pArgs;
  }
  else {
    return pArgs;
    CHECK(0, "Unexpected value from cmdline_parser!!");
   
  }
  return pArgs;
}  

/*! 

Description:

Prints out the program usage string.


*/
void 
CDAQStart::Usage()  
{ 
  cmdline_parser_print_help();
  
}  

/*! 

Description:

Peforms the main program logic.


Parameters:

\param argc (int)
   The count of command line parameters.
\param argv (char**)
   The command line parameters.

\return int
\retval     0 - Normal exit.
\retval other - abnormal exit.

*/
int 
CDAQStart::operator()(int argc , char** argv)  
{
  gengetopt_args_info* pArgs = ParseArgs(argc, argv);
  if(pArgs) {
    //  We need to create: The monitored program,
    //  The error and output sinks, 
    //  attach them to the monitored program.
    //  Start the program and enter the main loop:

    m_nArgc    = pArgs->inputs_num;
    m_pArgv    = pArgs->inputs;

    m_pProgram = new CLocalMonitoredProgram(pArgs->inputs_num,
					    pArgs->inputs);
    CSink* pOutSink(0);
    CSink* pErrSink(0);

    if(pArgs->output_given) {
      pOutSink =  CreateSink(pArgs->output_arg);
      if(pOutSink) {
	m_pProgram->AttachOutput(pOutSink);
      }
    }
    if(pArgs->error_given) {
      pErrSink = CreateSink(pArgs->error_arg);
      if(pErrSink) {
	m_pProgram->AttachError(pErrSink);
      }
    }
    if(pArgs->notify_given) {
      m_fNotifyExit = true;
    }
    else {
      m_fNotifyExit = false;
    }

    StartSubprocess();
    int stat = MainLoop();
    delete pOutSink;
    delete pErrSink;
    return stat;
  }
  else {
    Usage();
    return (-1);
  }
  CHECK(0, "Control should not have passed here!");

}  

/*! 

Description:

The program main loop.  While 
the subprocess is running, the I/O's and
return status are polled.


\return int status of the program.
\retval 0 success
\retval !0 failure by convention in unix like systems.


*/
int 
CDAQStart::MainLoop()  
{
  while(!MonitorExit()) {
    MonitorOutput(OUTPUTTIMEOUT);
  }
  MonitorOutput(OUTPUTTIMEOUT);	// Get the last drips from the pipe.
  if(m_fNotifyExit) {
    ReportExit();
  }
  return m_pProgram->GetFinalStatus();
}  

/*! 


Starts the subprocess.

*/
void 
CDAQStart::StartSubprocess()
{ 
  m_pProgram->Start();
}  

/*! 


Called by the main loop to monitor the output of the
prorgram.



\param nMs (int)
   # milleseconds to wait for output from the program.



*/
void 
CDAQStart::MonitorOutput(int nMs)  
{
  m_pProgram->PollIO(nMs);
}  

/*! 

Description:

Called by MainLoop to monitor the exit
status of the subprocess.  If the program 
has exited, it's sink is written to to
log this along with the status of the error.
If the DISPLAY env variable is set, similar
iformation is output to a pop up dialog.


\return bool
\retval true   - program exited
\retval false - program is still running.


*/
bool 
CDAQStart::MonitorExit()  
{
  if(m_pProgram->PollStatus()) { // Program exited.
    return true;
  } 
  else {			// Program still running
    return false;
  }
}
/*!
   Return the type field of a sink. Splits the name at the colon.
   and returns the first field to the left of the colon.

   \param name (const char*)
     The sink name to parse.
   
   \return char*
   \retval - NULL if there is no left half.
   \retval - non null if not.

*/
char*
CDAQStart::SinkType(char* name)
{
  char *(elements[2]);
  CSinkFactory::SplitName(name, elements);
  return elements[0];
}
/*!
   Return the name field of a sink.  The name is split at the first colon.
   the right half of the string is returned.

   \param name (const char*)
       Sink specification.

   \return char*
   \retval the right half of the colon separated descriptor.
          if there is no right half, an empty string is returned.
*/
char*
CDAQStart::SinkName(char* name)
{
  char *(elements[2]);
  if(CSinkFactory::SplitName(name, elements) != 2) {
    return const_cast<char*>("");
  }
  else {
    return elements[1];
  }
}

/*!
   Create a sink of the specified type.
   The sink name is split into type and name, and then the
   sink factory is asked to create the sink.
   
   \param name (const char*)
     The name of the sink.  This should be of the form:
       type:name   e.g. file:/home/fox/stuff.log 
    It is not necessarily a fatal problem for there only to be
    on element (e.g. logserver:) if that sinktype has no need for
    any further qualification.  Therefore, the two halves are passed
    to the factory without any further interpreation to creat the sink.

    \return CSink*
    \retval The newly created sink or NULL If the factory cannot create 
            one.


*/
CSink*
CDAQStart::CreateSink(const char* name)
{
  char* name1 = strdup(name);
  char* name2 = strdup(name);

  char  *pType = SinkType(name1);
  if(!pType) {
    free(name1);
    free(name2);
    return (CSink*)NULL;	// The only failure we can provide
  }
  string sType = pType;
  string sName(SinkName(name2));

    free(name1);
    free(name2);

  return CSinkFactory::getInstance()->Create(sType, string(*m_pArgv), sName);
}
/*!
   Report the exit of a process.
*/
void
CDAQStart::ReportExit()
{
  string ExitMessage;
  char   formatted[100];
  int status = m_pProgram->GetFinalStatus();

  for(int i =0; i < m_nArgc; i++) {
    ExitMessage += m_pArgv[i];
    ExitMessage += ' ';
  }

  ExitMessage += ": Program exited.  Final status: ";


  sprintf(formatted, " %d ", WEXITSTATUS(status));
  ExitMessage += formatted;
  
  if(WIFSIGNALED(status)) { // Exit due to signal:
    char* pSignalMessage = strsignal(WTERMSIG(status));
    sprintf(formatted, " Signal \"%s\" caught", 
	    pSignalMessage ? pSignalMessage :
	    "Unknown signal");
    ExitMessage += formatted;
  }
  ExitMessage+="\n";
  m_pProgram->StdErr(ExitMessage);
  
  // If the Display env var is present, pop up a dialog:
  // We're assuming the existence of a program named
  // PopUp in the bin director of the installation
  // tree. This program will accept as a parameter
  // a string to display as the error.
  //
  if(getenv("DISPLAY")) {	// Have X11 display...
    string Command("PopUp \"");
    Command += ExitMessage;
    Command += "\"";
    if (system(Command.c_str())) {
      int err = errno;
      fprintf(stderr,"%s %s : %s\n",
		    "Unable to start error dialog program ",
		    Command.c_str(),
		    strerror(err));
      exit(-1);
    }
  }
}
