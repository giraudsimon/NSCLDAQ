#include "stdintoringsw.h"
#include "stdintoringUtils.h"
#include "CRingBuffer.h"
#include "Exception.h"
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>
#include <stdint.h>
#include <stdio.h>
#include <signal.h>
#include <os.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <io.h>
#include <set>


using namespace std;


/********************************************************************
 * mainLoop:                                                        *
 *     Main loop that takes data from stdin and puts it in the ring *
 * Parameters:                                                      *
 *   std::string ring       - Name of the target ring.              *
 *   int         timeout    - Maximum time to wait for data on stdin*
 *   int         mindata    - Chunk size for reads.. which are done *
 *                            with blocking off.                    *
 *******************************************************************/

int
mainLoop(string ring, int timeout, int mindata)
{
  // If stdin is a socket set keepalive so we're given a SIGPIPE if the other
  // end drops off (See Bug #6248).
  
  struct stat fdInfo;
  if (fstat(STDIN_FILENO, &fdInfo)) {
    perror("Unable to fstat stdin");
    exit(EXIT_FAILURE);
  }
  if ((fdInfo.st_mode & S_IFMT)  == S_IFSOCK) {
    int keepflag=1;
    if(setsockopt(STDIN_FILENO, SOL_SOCKET, SO_KEEPALIVE, &keepflag, sizeof(keepflag))) {
      perror("Unable to enable keepalive flag on socket stdin");
      // exit(EXIT_FAILURE);
    }
    // The standard kernel keepalive times are too long - 2 hrs idle to 
    // heartbeat at 75 second intervals with 9 consecutive missed HB's indicating
    // failure.  We're going to h.b. after one minute of idle time with
    // HB's every 10 seconds and leave the 9 alone so in theory we'll drop off
    //  2.5 minutes from the remote guy exiting.
    
    int idleTime(60);
    int hbInterval(10);
    
    if (setsockopt(STDIN_FILENO, SOL_TCP, TCP_KEEPIDLE, &idleTime, sizeof(idleTime))) {
      perror("Unable to set STDIN (socket) TCP_KEEPIDLE parameter");
     // exit(EXIT_FAILURE);
    }
    
    if (setsockopt(STDIN_FILENO, SOL_TCP, TCP_KEEPINTVL, &hbInterval, sizeof(hbInterval))) {
      perror("Unable to set STDIN (socket) TCP_KEEPINTVL parameter");
      // exit(EXIT_FAILURE);
    }
  }
  
  
  
  // Attach to the ring:

  CRingBuffer* pSource;
  try {
    pSource = CRingBuffer::createAndProduce(ring);
    pSource->setPollInterval(0);
  }
  catch (CException& error) {
    cerr << "stdintoring Failed to attach to " << ring << ":\n";
    cerr << error.ReasonText() << endl;
    return (EXIT_FAILURE);
  }
  catch (string msg) {
    cerr << "stdintoring Failed to attach to " << ring << ":\n";
    cerr << msg << endl;
    return (EXIT_FAILURE);
  }
  catch (const char* msg) {
    cerr << "stdintoring Failed to attach to " << ring << ":\n";
    cerr << msg <<endl;
    return (EXIT_FAILURE);
  }
  catch (...) {
    cerr << "stdintoring Failed to attach to " << ring << endl;
    return (EXIT_FAILURE);
  }

  CRingBuffer& source(*pSource);
  CRingBuffer::Usage use = source.getUsage();
  if (mindata > use.s_bufferSpace/2) {
    mindata = use.s_bufferSpace/2;
  }
  // In order to deal with timeouts reasonably well, we need to turn off
  // blocking on stdin.

  // #define NONBLOCKING6
  long flags = fcntl(STDIN_FILENO, F_GETFL, &flags);
  flags   |= O_NONBLOCK;
  int stat= fcntl(STDIN_FILENO, F_SETFL, flags);
  if (stat == -1) {
    perror("stdintoring Failed to set stin nonblocking");
    return (EXIT_FAILURE);
  }
  uint8_t* pBuffer   = (uint8_t*)malloc(mindata);
  size_t readSize   = mindata; 
  size_t readOffset = 0; 
  size_t leftoverData = 0;
  size_t totalRead    = 0;
  
  // IF stdin is a pipe then set the pipe buffersize big:
  
  fcntl(STDIN_FILENO, F_SETPIPE_SZ, 1024*1024);

  while (1) {

    // Wait for stdin to be readable:

    fd_set readfds;
    fd_set writefds;
    fd_set exceptfds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_ZERO(&exceptfds);

    FD_SET(STDIN_FILENO, &readfds);
    timeval selectTimeout;
    selectTimeout.tv_sec = timeout;
    selectTimeout.tv_usec= 0;
    int stat = select(STDIN_FILENO+1, &readfds, &writefds, &exceptfds, NULL);
    // Three cases:
    // stat = 0... just do the next pass of the loop.
    // stat = -1   an error detected by select, but could be EINTR which is ok.
    // stat = 1 .. input is ready.

    if (stat != 0) {
      if (stat < 0) {
        if (errno != EINTR) {
          perror("Select failed");
          return (EXIT_FAILURE);
        }
      }
      else if (stat == 1) {
      ssize_t nread = read(STDIN_FILENO, pBuffer + readOffset, readSize);
      if (nread > 0) {
          totalRead += nread;
          /* If the header says the first ring item won't fit:
             - If the first ring item is bigger than the ring we can't go on.
             - If the first ring item will fit in the ring, enlarge the buffer.
          */
          struct header* pHeader = reinterpret_cast<struct header*>(pBuffer);
          uint32_t firstItemSize = computeSize(pHeader);
          if(firstItemSize > mindata) {
            if (firstItemSize > use.s_bufferSpace) {
              cerr << "Exiting because I just got an event that won't fit in the ring..enlarge the ring\n";
                    dumpWords(pHeader, 200);                     // Dump part of the ring.
              exit(EXIT_FAILURE);
            } else {
              mindata = firstItemSize + readOffset;
              pBuffer = reinterpret_cast<uint8_t*>(realloc(pBuffer, firstItemSize + readOffset));
              readOffset += nread;
              readSize    = mindata - readOffset;
            }
          } else {
            leftoverData = putData(source, pBuffer, totalRead);
            readOffset = leftoverData;
            readSize   = mindata - leftoverData;
            totalRead = leftoverData;
            if (readSize == 0) {
              exit(EXIT_FAILURE);
            }
          }
        }
        if (nread < 0) {
          perror("read failed");
          return (EXIT_FAILURE);
        }
        if (nread == 0) {
          cerr << "Exiting due to eof\n";
          return (EXIT_SUCCESS);	// eof on stdin.
        }
      } else {
        cerr << "Exiting due to select fail " << errno << endl;
        return (EXIT_FAILURE);
      }
    } 

  }
  

}

/*!
   Entry point.
*/

int main(int argc, char** argv)
{
  // Close all files but the std ones:
  
  std::set<int> keepOpen;
  keepOpen.insert(STDIN_FILENO);
  keepOpen.insert(STDOUT_FILENO);
  keepOpen.insert(STDERR_FILENO);
  
  io::closeUnusedFiles(keepOpen);
  
  // Turn off pipe signal:

  if (Os::blockSignal(SIGPIPE)) {
    perror("Failed to block pipe signals");
  }
  struct gengetopt_args_info parsed;

  int status = cmdline_parser(argc, argv, &parsed);
  if (status == EXIT_FAILURE) {
    return (status);
  }

  // There should be exactly one parameter, that is not a switch,
  // the ring name:

  if (parsed.inputs_num != 1) {
    cmdline_parser_print_help();
    exit (EXIT_FAILURE);
  }
  // extract the rin name, the timeout and the size from the 
  // parameters/defaults:
  //

  string ringname = parsed.inputs[0];
  int    timeout  = parsed.timeout_arg;
  size_t mindata  = integerize(parsed.mindata_arg);

  int exitStatus;
  try {
    exitStatus = mainLoop(ringname, timeout, mindata);
  }
  catch (std::string msg) {
    std::cerr << "string exception caught: " << msg << std::endl;
  }

  // If requested, delete the ring on exit:

  if (parsed.deleteonexit_given) {
    CRingBuffer::remove(ringname);
  }
  exit(exitStatus);
}
void* gpTCLApplication(0);
