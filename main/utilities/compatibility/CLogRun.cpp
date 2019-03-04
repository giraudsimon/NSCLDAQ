/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2008

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>
#include "CLogRun.h"
#include "buftypes.h"
#include "buffer.h"

#include <stdio.h>
#include <stdint.h>

#include <iostream>
#include <fstream>



size_t CLogRun::m_nMaxSegmentSize(1.8L*1024L*1024L*1024L); // When to start a new segment.

/**
 * Construct the application object CLogRun:
 * @param bufferSize - number of bytes to expect in each buffer.
 */
CLogRun::CLogRun(size_t bufferSize) :
  m_nBufferSize(bufferSize)
{}

/*
 --------------------------------------------------------------------------------------
*/

/**
 *   Log a run to disk.
 *  - Search for the first begin run buffer/buffersize (if that's not the first buffer complain.
 *  - Based on the begin run buffer open the output file
 *  - Log buffers until end of run if necessary creating additional run segments.
 */

void
CLogRun::operator()()
{
  bool seenBor = false;
  char fileBuffer[m_nBufferSize];
  int  segnum = 0;
  size_t segsize;
  while (!seenBor) {
    std::cin.read(fileBuffer, m_nBufferSize);
    if (std::cin.fail() || std::cin.eof()) {
      std::cerr << "Never saw a begin run buffer and the read from stdin failed\n";
      return;
    }
    int type = bufferType(fileBuffer);
    if (type == BEGRUNBF) break;
  }

  // We have our first useful buffer.
  // Create the first event segment and start writing data:
  //

  int run = getRun(fileBuffer);
  
  std::ofstream* pOutput = openRunFile(run, segnum);

  while (1) {

    // Write the current buffer:

    pOutput->write(fileBuffer, m_nBufferSize);
    if (pOutput->fail()) {
      std::cerr << "Event data buffer write failed\n";
      return;
    }
    // If the buffer we just wrote is an endrun we're done:

    if (bufferType(fileBuffer) == ENDRUNBF) {
      pOutput->close();
      delete pOutput;
      return;
    }

    // If needed, close off the current segment, open a new segment file
    // and reset the segment size counter:
    // 
    segsize += m_nBufferSize;
    if (segsize > m_nMaxSegmentSize) {
      pOutput->close();
      delete pOutput;
      segnum++;
      segsize = 0;
      pOutput = openRunFile(run, segnum);
    }
    // Read in the next data buffer:

    std::cin.read(fileBuffer, m_nBufferSize);
    if (std::cin.fail() || std::cin.eof()) {
      std::cerr << "Buffer read failed, closing event file and giving up\n";
      pOutput->close();
      delete pOutput;
      return;
      
    }
  }
  // Flow should never go here:

  std::cerr << "Somehow we got out of the main loop..should not happen\n";

}

/*
 ------------------------------------------------------------------------------------------
*/

/**
 * Open a run file.
 * The run file name is of the form:  run-%04d-%02d.evt
 *  where the first integer is the run number and the second the segment.
 *  This name is not compatible with old style daq because we need it to be
 *  compatible with the readout gui's file handling.
 * 
 * @param run - run number.
 * @param segment - Segment number.
 * @return std::ofstream*
 * @retval pointer to ofstream open on the new event file segment.
 */

std::ofstream*
CLogRun::openRunFile(int runNumber, int runSegment)
{
  char filename[100];
  const char* pFormat = "run-%04d-%02d.evt";

  sprintf(filename, pFormat, runNumber, runSegment);
  return new std::ofstream(filename);
}

/**
 * Extract the buffer type from an NSCL buffer:
 * @param pBuffer - Pointer to the buffer.
 * @return int
 * @retval integerized type field of buffer header.
 */
int
CLogRun::bufferType(const void* pBuffer)
{
  const bheader* pHeader = reinterpret_cast<const bheader*>(pBuffer);
  return pHeader->type;
}
/**
 * Extract the run number from an NSCL Buffer:
 * @param pBuffer - Pointer to the buffer.
 * @return int
 * @retval integerized run field of buffer header.
 */
int
CLogRun::getRun(const void* pBuffer)
{
  const bheader* pHeader = reinterpret_cast<const bheader*>(pBuffer);
  return pHeader->run;
}
