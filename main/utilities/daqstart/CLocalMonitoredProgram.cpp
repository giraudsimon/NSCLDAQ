/*
	Implementation file for CLocalMonitoredProgram for a description of the
	class see CLocalMonitoredProgram.h
*/

////////////////////////// FILE_NAME.cpp ///////////////////////////

// Include files required:

#include <config.h>
#include "CLocalMonitoredProgram.h" 

#include <DesignByContract.h>
#include <ErrnoException.h>

#include <iostream>

#include <fcntl.h>
#include <unistd.h>   				
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/poll.h>


using namespace std;


// Static attribute storage and initialization for CLocalMonitoredProgram


static const size_t LINESIZE(100);
/*!
    Create an object of type CLocalMonitoredProgram
    \param argc  (int)
      count of command line parameters for the program.
    \param argv (char**)
      the arguments themselves.
*/
CLocalMonitoredProgram::CLocalMonitoredProgram (int argc, char** argv)
  : CMonitoredProgram(argc, argv), 
  m_nStdout(-1),
  m_nStderr(-1),
  m_fRunning(false),
  m_nPid(-1),
  m_sStdoutLine(string("")),
  m_sStderrLine(string(""))
{ 
} 

/*!
    Called to destroy an instance of CLocalMonitoredProgram
    If the program is running it is killed off and the
    pipes are closed.  This is a very bad thing... to 
    destroy this object while the program is running.

*/
 CLocalMonitoredProgram::~CLocalMonitoredProgram ( )
{
  if(m_fRunning) {
    kill(m_nPid, SIGKILL);	// Kill off child.
    close(m_nStdout);		// Close the pipes.
    close(m_nStderr);
  }
}

// Functions for class CLocalMonitoredProgram

/*! 

Description:

Starts the monitored program.
Two pipes are created using pipe(2)
The program line is then gotten, 
as well as the program parameters.
fork/exec is used to start the subprocess.

\pre m_fRunning is false.

*/
void 
CLocalMonitoredProgram::Start()  
{ 
  // Check the preconditions;
  
  REQUIRE(!m_fRunning, "The subprocess is already running!");

  // Get the pipes that will be associated with the
  // program's stdout an stderr.  The output ends of these
  // pipes will be stuffed into m_sStdoutLine and
  // m_sSeterrLine:

  int StdOutPipes[2];		// Pipes for stdout.
  int StdErrPipes[2];		// Pipes for stderr.

  if(pipe(StdOutPipes)) {
    throw CErrnoException("Creating stdout pipes");
  }
  if(pipe(StdErrPipes)) {
    throw CErrnoException("Creating stderr pipes");
  }

  m_nStdout = StdOutPipes[0];
  m_nStderr = StdErrPipes[0];

  // Fork the subprocess.  The parent process
  // just records the pid and closes the
  // output ends of the pipes.
  // The child process dups the pipes over to the appropriate
  // file ids, closes the extras and execs the program.

  int m_nPid = fork();

  if(m_nPid == -1) {		// Fork failed!!
    throw CErrnoException("Forking subprocess failed");
  }
  if(m_nPid > 0) {		// Parent process.
    //    close(StdOutPipes[1]);	// Close the write side of stdout
    //    close(StdErrPipes[1]);	// and stderr pipes.
    m_fRunning = true;		// The subprocess is running.
    m_sStdoutLine = "";		// Empty the input buffers.
    m_sStderrLine = "";
    BlockingOff(m_nStdout);
    BlockingOff(m_nStderr);

  }
  else if (m_nPid == 0) {	// Child process.
    // Dup the pipes to stderr and stdout.

    close(STDOUT_FILENO);
    dup2(StdOutPipes[1], STDOUT_FILENO);
    close(STDERR_FILENO);
    dup2(StdErrPipes[1], STDERR_FILENO);

    // Now exec the subprocess:

    char** argv = GetArgs();	// This is null terminated so just:
    execvp(argv[0], argv);	// Should not return!!
    cerr << "Unable to run " << argv[0] 
	 << " : " 
	 << errno << " : " << strerror(errno) << endl;
    exit(-1);
  }
  else {			// BUG!!
    CHECK(0, "fork evidently failed, but we didn't exit");
  }

}  

/*! 

Description:

Does a poll with timeout on the file
descriptors for the process's stdout 
and stderr.  If any data are received they
are appended to the appropriate 
m_sStdoutLine
m_sStderrLine

If a newline is read, the StdOut and StdErr members
are called.

\pre m_fRunning

Parameters:

\param nMs (int)
   Number of milliseconds to block for fileIO.

*/
void 
CLocalMonitoredProgram::PollIO(int nMs)  
{
  // Check the precondition.

  REQUIRE(m_fRunning, "Sub process is not yet running!");
  
  //

  struct pollfd PollInfo[2];	// One for stdout and another stderr.
  
  // The flags will be the same for both polls:

  short nEventMask = POLLIN | POLLPRI;

  // Setup the stdout ([0]) element of pollinfo:

  PollInfo[0].fd     = m_nStdout;
  PollInfo[0].events = nEventMask;

  // Setup the stderr ([1]) element of pollinfo.

  PollInfo[1].fd     = m_nStderr;
  PollInfo[1].events = nEventMask;

  // Now do the poll. 

  int nPollStatus = poll(PollInfo, 2, nMs);

  // We're interested in nonzero cases. >0 means there's data
  // to read.  < 0 means that the poll somehow failed an that needs
  // to be thrown to report.
  // 0's just a timeout and is ignored.

  if(nPollStatus < 0) {		// Error from poll
    if(errno != EINTR) {
      throw CErrnoException("PollIO poll failed");
    }
    else {
      ;				// EINTR is a no-op.
    }
  } 
  else if(nPollStatus > 0) {	// At least one fd with stuff.
    // Flag checks:
    // POLLLIN | POLLPRI indicate input.
    // POLLERR | POLLHUP | POLLINVAL indicate a hangup and will
    // result in our own log message.
    //
    if(PollInfo[0].revents) {	// Stdout has something
      if(ProcessInputFd(m_nStdout, m_sStdoutLine,  
			PollInfo[0].revents)) {	// full line..
	StdOut(m_sStdoutLine);
	m_sStdoutLine = "";
      }
    }
    if(PollInfo[1].revents) {	// Stderr has something
      if(ProcessInputFd(m_nStderr, m_sStderrLine,
			PollInfo[1].revents)) {
	StdErr(m_sStderrLine);
	m_sStderrLine = "";
      }
    }
  }
  else {			// Timeout.
    ;				// Just a timeout; do nothing.
  }
}  

/*! 

Description:

Does a waitpid without blocking.  If the
subprocess is completed, the exit
status is set and the completion flag is
set (subsequent calls will just return).

\pre m_fRunning


\return bool
\retval true   - The subprocess has exited, its return status
                      can be gotten via a call to GetFinalStatus
\retval false  - The subprocess is still running.


*/
bool 
CLocalMonitoredProgram::PollStatus()  
{ 
  int nExitStatus;

  // Check precondition(s)

  REQUIRE(m_fRunning, "SubProcess is not running at this time");

  // Do the wait and look at what we get:

  pid_t pid = waitpid(m_nPid, &nExitStatus, WNOHANG);

  if(pid > 0) {			// Info on pid returned.. could be stopped.
    if(WIFEXITED(nExitStatus) ||
       WIFSIGNALED(nExitStatus)) { // So be sure the process exited before
      m_fRunning = false;	// and indicating the process is not running
      SetFinalStatus(nExitStatus); // Save status in base class.
      return true;
    }
    else {			// Not exited.
      return false;
    }
  }
  else if (pid == 0) {		// not exited or stopped.
    return false;
  }
  else {			// Pid < 0 is an error:
    throw CErrnoException("Waitpid failed");
  }

}
// Utility functions:
//  They don't deserve doxygen entries.  Just good comments.
		      
/*
  BlockingOff - This function turns off blocking on a file descriptor.
                We do this in order to ensure that when we read data
		from a file descriptor to get the current line,
		we will not be blocking if the full read cannot be
		satisfied.
  Parameters:
  int nFd - The file descriptor to alter.

 */		     
void
CLocalMonitoredProgram::BlockingOff(int nFd)
{
  // Need to get the fd flags for the file descriptor,
  // or in O_NONBLOCK
  // and set them back.. Either fcntl in theory could fail
  // failure results in a CErrnoException throw.
  //

  long flags;
  flags = fcntl(nFd, F_GETFL, 0);
  if(flags == -1) {		// Special error state
    throw CErrnoException("Unable to get file descriptor flags");
  }
  flags |= O_NONBLOCK;

  if(fcntl(nFd, F_SETFL, flags)) {
    throw CErrnoException("Unable to set file descriptor flags");
  }

}
/*
  ProcessInputFd  - Takes the status state flags from a poll on a
  file descriptor and processes them:
  if POLLIN or POLLPRI are set, the file is read for data and the
  data is appended to the associated line buffer.

  if POLLERR, POLLHUP or POLLINVAL are set, this is an error that
  is thrown as a String exception.

  Parameters:
      int nFd            - The file descriptor that's being handled.
      string& sLine      - The file's output buffer.
      short   nPollFlags - The flags from poll.

  Return:
     true   - The line has at least one \n in it.
     false  - the line has no \n's in it.

 */
bool
CLocalMonitoredProgram::ProcessInputFd(int nFd, string& sOutline,
				     short nPollFlags)
{
  if(nPollFlags & (POLLERR | POLLHUP | POLLNVAL)) {
    StdErr("Poll returned flags other than POLLIN, POLLOUT");
  }
  if(nPollFlags & (POLLIN | POLLPRI)) {
    //
    // The data are read into buffer and then appended to
    // the line.  The gymnastics below assure there's a 
    // null terminator on the string.
    //
    char buffer[(int)LINESIZE];
    memset(buffer,0, LINESIZE);	

    int nRead = read(nFd, buffer, LINESIZE-1); // Ensure's null terminator
    
    if(nRead < 0) {		// Read error...
      throw CErrnoException("Read failed after poll was good");
    }
    else if (nRead > 0) {	// Characters available.
      sOutline += buffer;	// Append the string...
      if(index(buffer, '\n')) {
	return true;
      } else {
	return false;
      }
    }
    else {			// No chars somehow.
      ;				// Grant forbearance even tho should be chars.
    }
  }
  else {
    throw string("Poll returned unexpected flags.");
  }
  // Control should not land here.

  ENSURE(0, "Bug in control logic for ProcessInputFd");
  return false;
}

