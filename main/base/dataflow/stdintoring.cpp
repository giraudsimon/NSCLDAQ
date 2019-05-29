#include "stdintoringsw.h"
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

// Item header:

struct header {
  uint32_t s_size;
  uint32_t s_type;
}; 

/**************************************************************
 * integerize:                                                *
 *   Utility function that converts a string to an integer    *
 *   while allowing the suffixes k, m, g to be kilo, mega     *
 *   and giga multipliers respectively.  Computer sizes e.g.  *
 *   k = 1024, m = 1024*1024, g = 1024*1024*1024              *
 *                                                            *
 * On failure, this will print the usage and exit with a fail *
 * code.                                                      *
 *                                                            *
 * Parameters:                                                *
 *   const char* str  - The input string                      *
 * Returns:                                                   *
 *   The integer result                                       *
 *************************************************************/

static int 
integerize(const char* str)
{
  char* endptr;

  // Decode the front part:

  int base = strtol(str, &endptr, 0); // Allow hex etc.
  
  // It's an error to not have at least one digit:

  if (endptr == str) {
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
  }

  // If the entire string was converted return it:

  if(strlen(endptr) == 0) {
    return base;
  }
  // If there's more than one extra char, that's an error too:

  if (strlen(endptr) > 1) {
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
  }

  // One extra character can be the multiplier

  char multiplier = *endptr;
  int value;

  switch (multiplier) {
  case 'k':
    value = base*1024;
    break;
  case 'm':
    value = base*1024*1024;
    break;
  case 'g':
    value = base*1024*1024*1024;
    break;

  default:
    cmdline_parser_print_help();
    exit(EXIT_FAILURE);
  }
  return value;
}

/**
 * Determine the size of an item in the possible presence of a difference
 * in endianness.  This is done by knowing only the bottom 16 bits of type data
 * are used.
 * @param pHeader - pointer to the header.
 * @return uint32_t
 * @retval the size of the item converted to local format if needed.
 */
static uint32_t
computeSize(struct header* pHeader)
{
  if ((pHeader->s_type & 0xffff) == 0) {
    uint8_t swappedSize[4];
    memcpy(swappedSize, &pHeader->s_size, 4);
    uint32_t result = 0;
    for (int i =0; i < 4; i++) {
      uint32_t byte = swappedSize[i];
      byte          = byte << (24 - 4*i);
      result       |= byte;
    }
    return result;
  }
  else {
    return pHeader->s_size;
  }
}

void dumpWords(void* src, size_t nwords)
{
  uint16_t* s = reinterpret_cast<uint16_t*>(src);
  std::cerr << std::hex;
  for (int i=0; i < nwords; i++) {
    if ((i % 8) == 0 ) std::cerr << std::endl;
    std::cerr << *s++ << " ";
  }
  std::cerr << std::dec;
}

/**
 * Put data into the ring.
 * Each data item is assumed to be preceded by a two longword header of the form:
 * struct header {
 *    uint32_t size;
 *    uint32_t type
 *  };
 * 
 * Futhermore, from the point of view of byte ordering, the type field only has
 * nonzero bits in the lower order 16 bits.
 *
 * We're going to put each data item in the ring atomically.
 * If there are left over data in the buffer, that will be shifted down
 * to the beginning of the buffer and
 * the remaining size will be returned:
 * 
 * @param ring    - reference to the target ring buffer.
 * @param pBuffer - Pointer to the data buffer.
 * @param nBytes  - Number of bytes in the data buffer.
 *
 * It is assumed that the total buffer size will be larger than
 * needed to hold the largest item.   That's controld by the 
 * --mindata switch in any event.
 */
static size_t 
putData(CRingBuffer& ring, void* pBuffer, size_t nBytes)
{
  size_t nOriginal = nBytes;

  struct header *pHeader;

  uint8_t* p = reinterpret_cast<uint8_t*>(pBuffer); // makes addr arithmetic easier.
  struct header *pLastItem;
  int           lastSize = 0;



  while(nBytes > sizeof(struct header)) {
    pHeader = reinterpret_cast<struct header*>(p);
    uint32_t size = computeSize(pHeader);


    if (size <= nBytes) {
      // we can put a complete item
      pLastItem = pHeader;

      ring.put(p, size);
      p += size;
      nBytes -= size;
      lastSize = size;
    } 
    else {
      // We don't have a complete packet.
      // Move the remainder of the buffer down to the start
      // must use memmove because this could be an overlapping
      // move
      break;
    }

  }								
  if ((nBytes > 0) && (nBytes != nOriginal)) {
    memmove(pBuffer, p, nBytes);
  }
  return nBytes;		// Residual data.
}


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
              cerr << "item larger than --minsize, reallocating bufer to " << firstItemSize + readOffset << endl;
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
