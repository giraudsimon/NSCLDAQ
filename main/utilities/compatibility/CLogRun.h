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
#ifndef CLOGRUN_H
#define CLOGRUN_H

#include <cstddef>
#include <fstream>



/**
 * This is a class that logs a single run to disk and then exits
 * it is our application class.
 */

class CLogRun {
private:
  size_t m_nBufferSize;
  static size_t m_nMaxSegmentSize;
public:
  CLogRun(size_t bufferSize);

  void operator()();
private:
  std::ofstream*  openRunFile(int runNumber, int runSegment);
  int bufferType(const void* pBuffer);
  int getRun(const void* pBuffer);

};


#endif
