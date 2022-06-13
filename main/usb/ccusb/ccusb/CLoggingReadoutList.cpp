
#include <CLoggingReadoutList.h>

#include <sstream>

CLoggingReadoutList::CLoggingReadoutList(const CLoggingReadoutList& rhs) 
  : CCCUSBReadoutList(rhs), m_log()
{ 
}

CLoggingReadoutList&
  CLoggingReadoutList::operator=(const CLoggingReadoutList& rhs) 
{ 
  if (this != &rhs) {
    m_log = rhs.m_log;
    CCCUSBReadoutList::operator=(rhs);
  }

  return *this;
}

void CLoggingReadoutList::append(const CLoggingReadoutList& rhs)
{
  CCCUSBReadoutList::append(rhs);
  auto newEntries = rhs.getLog();
  m_log.insert(m_log.end(), newEntries.begin(), newEntries.end());
}

void
CLoggingReadoutList::addWrite16(int n, int a, int f, uint16_t data)
{
  CCCUSBReadoutList::addWrite16(n,a,f,data);

  std::stringstream msg;
  msg << "write16 " << n << " " << a << " " << f << " " << data;
  m_log.push_back(msg.str());
}


void
CLoggingReadoutList::addWrite24(int n, int a, int f, uint32_t data)
{
  CCCUSBReadoutList::addWrite24(n,a,f,data);

  std::stringstream msg;
  msg << "write24 " << n << " " << a << " " << f << " " << data;
  m_log.push_back(msg.str());
}



void
CLoggingReadoutList::addRead16(int n, int a, int f, bool lamwait)
{
  CCCUSBReadoutList::addRead16(n,a,f, lamwait);

  std::stringstream msg;
  msg << "read16 " << n << " " << a << " " << f;
  if (lamwait) msg << " true";
  else msg << " false";
  
  m_log.push_back(msg.str());
}


void
CLoggingReadoutList::addRead24(int n, int a, int f, bool lamwait)
{
  CCCUSBReadoutList::addRead24(n,a,f, lamwait);

  std::stringstream msg;
  msg << "read24 " << n << " " << a << " " << f;
  if (lamwait) msg << " true";
  else msg << " false";
  m_log.push_back(msg.str());
}

void
CLoggingReadoutList::addControl(int n, int a, int f)
{
  CCCUSBReadoutList::addControl(n,a,f);

  std::stringstream msg;
  msg << "control " << n << " " << a << " " << f;
  m_log.push_back(msg.str());
}


void
CLoggingReadoutList::addQStop(int n, int a, int f, uint16_t max, bool lamwait)
{
  CCCUSBReadoutList::addQStop(n,a,f,max,lamwait);

  std::stringstream msg;
  msg << "qStop " << n << " " << a << " " << f << " " << max;
  if (lamwait) msg << " true";
  else msg << " false";
  m_log.push_back(msg.str());
}


void
CLoggingReadoutList::addQStop24(int n, int a, int f, uint16_t max, bool lamwait)
{
  CCCUSBReadoutList::addQStop24(n,a,f,max,lamwait);

  std::stringstream msg;
  msg << "qStop24 " << n << " " << a << " " << f << " " << max;
  if (lamwait) msg << " true";
  else msg << " false";
  m_log.push_back(msg.str());
}

void
CLoggingReadoutList::addQScan(int n, int a, int f, uint16_t max, bool lamwait)
{
  CCCUSBReadoutList::addQScan(n,a,f,max,lamwait);

  std::stringstream msg;
  msg << "qScan " << n << " " << a << " " << f << " " << max;
  if (lamwait) msg << " true";
  else msg << " false";
  m_log.push_back(msg.str());
}


void
CLoggingReadoutList::addRepeat(int n, int a, int f, uint16_t max, bool lamwait)
{
  CCCUSBReadoutList::addRepeat(n,a,f,max,lamwait);

  std::stringstream msg;
  msg << "repeat " << n << " " << a << " " << f << " " << max;
  if (lamwait) msg << " true";
  else msg << " false";
  m_log.push_back(msg.str());
}

void
CLoggingReadoutList::addAddressPatternRead16(int n, int a, int f, bool lamwait)
{
  CCCUSBReadoutList::addAddressPatternRead16(n,a,f,lamwait);

  std::stringstream msg;
  msg << "addressPatternRead16 " << n << " " << a << " " << f;
  if (lamwait)  msg << " true";
  else  msg << " false";

  m_log.push_back(msg.str());
}

void
CLoggingReadoutList::addMarker(uint16_t marker)
{
  CCCUSBReadoutList::addMarker(marker);

  std::stringstream msg;
  msg.flags(std::ios::hex);
  msg << "marker ";
  msg.width(4);
  msg << marker;
  m_log.push_back(msg.str());
}
