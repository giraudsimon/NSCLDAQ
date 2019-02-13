#ifndef EVENTLOGMAIN_H
#define EVENTLOGMAIN_H

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

// Headers:


#include <stdint.h>
#include <string>

#include <CBufferedOutput.h>

// Forward class definitions.

class CRingBuffer;
class CRingItem;
class CRingStateChangeItem;


/*!
   Class that represents the event log application.
   separating this out in a separate class may make
   possible unit testing of chunks of the application
   with cppunit
*/
class EventLogMain
{
  // Per object data:

  CRingBuffer*      m_pRing;
  std::string       m_eventDirectory;
  uint64_t          m_segmentSize;
  bool              m_exitOnEndRun;
  unsigned          m_nSourceCount;
  bool              m_fRunNumberOverride;
  uint32_t          m_nOverrideRunNumber;
  bool              m_fChecksum;
  void*             m_pChecksumContext;  
  uint32_t          m_nBeginsSeen;
  bool              m_fChangeRunOk;
  std::string       m_prefix;
  io::CBufferedOutput*  m_pOutputter;

  // Constructors and canonicals:

public:
  EventLogMain();
  ~EventLogMain();

private:
  EventLogMain(const EventLogMain& rhs);
  EventLogMain& operator=(const EventLogMain& rhs);
  int operator==(const EventLogMain& rhs) const;
  int operator!=(const EventLogMain& rhs) const;

  // Object operations:
public:
  int operator()(int argc, char**argv);

  // Utilities:
private:
  void parseArguments(int argc, char** argv);
  int  openEventSegment(uint32_t runNumber, unsigned int segment);
  void recordData();
  void recordRun(const CRingStateChangeItem& item, CRingItem* pFormatItem);
  void writeItem(int fd, CRingItem&    item);
  std::string defaultRingUrl() const;
  uint64_t    segmentSize(const char* pValue) const;
  bool  dirOk(std::string dirname) const;
  bool  dataTimeout();
  size_t itemSize(CRingItem& item) const;
  std::string shaFile(int runNumber) const;
  bool isBadItem(CRingItem& item, int runNumber);
};



#endif
