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
   Provides a Tcl loadable package that exposes some
   unix  system services.
   
  The package is called Wait

   The commands exposed are
    

   wait ?-pid pid?  ?-poll?

   If -pid pid is not supplied, the wait waits for the next 
   process to complete.  Otherwise:
   - pid == -1  Implies wait for any process.
   - pid > 0    pid is the process id of a single child process to wait for.
   - pid == 0   Wait for  any child process in our process group.
   - pid < -1   Wait for any process in the process group |pid|.

   If -poll is specified, wait will return immediately even if no process
   has completed.

  Wait succeeds if either a process completed or -poll was specified and no
  matching proces completed.  In that case, the return value of wait is
  a list containing:  {ending_processid status}  where this list is
  0 0 if there was no matching process.
  On failure, an error result is returned and the error is managed as for a 
  POSIX error.

  pipe 

  Pipe creates a unix pipe (via pipe(2)) and returns a list of Tcl channels.
  the first channel in the list is the read end of the pipe, and the second 
  the write end.


*/
#include <config.h>
#include <tcl.h>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>

#include <CopyrightNotice.h>
#include <TCLResult.h>
#include <TCLApplication.h>
#include <TCLInterpreter.h>
#include <TCLProcessor.h>
#include <TCLObjectProcessor.h>
#include <TCLObject.h>


using namespace std;




static const char* version="1.0";


/*!
   CTCLWait implements the wait command.
*/
class CTCLWait : public CTCLProcessor
{
public:
  CTCLWait(CTCLInterpreter* pInterp,
	   const std::string& rsCommand = std::string("Wait"));
  virtual int operator()(CTCLInterpreter& pInterp, 
		     CTCLResult&      rResult,
		     int argc, char* argv[]);
protected:
  static string Usage();
};

/*!
   CTCLPipe implements the pipe command.
*/
class CTCLPipe : public CTCLProcessor
{
public:
  CTCLPipe(CTCLInterpreter* pInterp,
	   const std::string& rsCommand = std::string("Pipe"));
  virtual int operator()(CTCLInterpreter& pInterp, 
		     CTCLResult&      rResult,
		     int argc, char* argv[]);


};

/**
 *  CTCLGetPPId - get process parent id.
 * 
 */
class CTCLGetPPId : public CTCLObjectProcessor
{
public:
  CTCLGetPPId(CTCLInterpreter* pInterp, const char* command = "ppid");
  virtual int operator()(
    CTCLInterpreter& interp, std::vector<CTCLObject>& objv
  );
  
};

extern "C" {
  int Wait_Init(Tcl_Interp* pInterp)
  {
    CopyrightNotice::Notice(cerr, "Wait", version, "2003, 2018");
    CopyrightNotice::AuthorCredit(cerr, "Wait", "Ron Fox", NULL);

    // Wrap the interpreter into an object so we can manipulate it.:

    CTCLInterpreter& rInterp(*(new CTCLInterpreter(pInterp)));
    Tcl_PkgProvide(pInterp, "Wait", (char*)version);

    CTCLWait* pWait = new CTCLWait(&rInterp);
    pWait->Register();

    CTCLPipe* pPipe = new CTCLPipe(&rInterp);
    pPipe->Register();
    
    new CTCLGetPPId(&rInterp);

    return TCL_OK;
  }
}

void* gpTCLApplication(0);

//-----------------------------------------------------------
// Implementation of the CTCLWait command processor.

/*!
   Constructor: creates the command.
   \param pInterp (CTCLInterpreter* pInterp [in]):
     Name of the interpreter on which this command will be 
     constructed.
   \param rsCommand (const string& [in] = Wait):
      Name under which the command is registered.
*/
CTCLWait::CTCLWait(CTCLInterpreter* pInterp,
		   const string& rsCommand) :
  CTCLProcessor(rsCommand, pInterp)
{
}

/*! 
   Executes the wait command.  Wait will be implemented in terms
   of the waitpid system service.
*/
int
CTCLWait::operator()(CTCLInterpreter& pInterp, 
		     CTCLResult&      rResult,
		     int argc, char* argv[])
{

  int pid   = -1;		// Default is wait for any pid.
  int flags =  0;		// Default flags.

  argc--;argv++;		// Skip the command name.

  // parse any switches/command arguments.

  while(argc) {
    string param(*argv);
    if(param == string("-pid")) { // Specify the PID.
      argc--; argv++;
      if(argc > 0) {
	pid = atoi(*argv);
      }
      else {
	rResult = "Missing pid parameter\n";
	rResult += Usage();
	return TCL_ERROR;
      }
    }
    else if (param == string("-poll")) {	// No hang if not done.
      flags |= WNOHANG;
    }
    else {
      rResult = "Invalid switch\n";
      rResult += Usage();
      return TCL_ERROR;
    }
    argc--; argv++;
  }

  // Execute the wait:
 
  int   status;

  int   rpid = waitpid(pid, &status, flags);


  // Compute and return result.


  if(rpid < 0) {

    Tcl_SetErrno(errno);
    rResult = Tcl_ErrnoId();
    rResult += ":";
    rResult += Tcl_ErrnoMsg(errno);
    return TCL_ERROR;
  }
  else if (rpid == 0) {
    rResult = "0 0";		// No matching return.
  }
  else {
    char Result[100];
    sprintf(Result,"%d %d", rpid, status);
    rResult = Result;
  }
  

  return TCL_OK;
}



/*!
   Returns usage string.
*/
string
CTCLWait::Usage()
{
  string result("Usage:\n");
  result      +=    "wait ?-pid pid? ?-poll?\n";
  result      +=    "   -pid pid specifies a specific process\n";
  result      +=    "   -poll Does not block until process done\n";
  return result;
}

//--------------------------------------------------------------

// Implementation of the pipe command:

CTCLPipe::CTCLPipe(CTCLInterpreter* pInterp, const std::string& rCommand) :
  CTCLProcessor(rCommand, pInterp)
{
}
/*!
   Execute the pipe command.
*/
int
CTCLPipe::operator()(CTCLInterpreter& rInterp,
		     CTCLResult&      rResult,
		     int argc, char** argv)
{
  int fds[2];
  int status = pipe(fds);

  // Make tcl channels out of the fds.

  Tcl_Channel readable = Tcl_MakeFileChannel((ClientData)(uint64_t)fds[0], 
					     TCL_READABLE);
  Tcl_Channel writable = Tcl_MakeFileChannel((ClientData)(uint64_t)fds[1], 
					     TCL_WRITABLE);

  // Register the channels with Tcl so users can use them.

  Tcl_RegisterChannel(rInterp.getInterpreter(), readable);
  Tcl_RegisterChannel(rInterp.getInterpreter(), writable);

  // Put the channel names into the result as a list:

  rResult.AppendElement(Tcl_GetChannelName(readable));
  rResult.AppendElement(Tcl_GetChannelName(writable));

  return TCL_OK;
}
/*----------------------------------------------------------------------------
 * Implementation of CTCLGetPPId
 *---------------------------------------------------------------------------*/

/**
 * constructor
 *    @param pInterp - pointer to the wrapped interpreter object.
 *    @param name    - command name.
 */
CTCLGetPPId::CTCLGetPPId(CTCLInterpreter* pInterp, const char* name) :
  CTCLObjectProcessor(*pInterp, name, true)
  {}
  
/**
 * operator()
 *    Process the command - we get unhappy if there are any
 *    command parameters.  Otherwise, just return getppid as the
 *    result.
 *
 *  @param interp - reference to the interpreter executing the command.
 *  @param objv   - Vector of parameters.
 *  @return int   - TCL_OK on success, TCL_ERROR on failure.
 */
int
CTCLGetPPId::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
  if(objv.size() != 1) {
    interp.setResult("ppid does not accept any parameters");
    return TCL_ERROR;
  }
  
  int id = getppid();
  Tcl_Obj* res = Tcl_NewIntObj(id);
  interp.setResult(res);
  return TCL_OK;
}
