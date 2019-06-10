/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
       Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#include <CTestSourceSink.h>

#include <iterator>
#include <algorithm>
#include <stdexcept>

using namespace std;


//
CTestSourceSink::CTestSourceSink()
  : m_buffer()
{}


//
CTestSourceSink::CTestSourceSink(size_t buffer_size)
  : m_buffer()
{
  m_buffer.reserve(buffer_size);
}


//
CTestSourceSink::~CTestSourceSink() {}


//
void CTestSourceSink::putItem(const CRingItem& item)
{}


//
void
CTestSourceSink::put(const void* pData, size_t nBytes)
{
  auto pBegin = reinterpret_cast<const char*>(pData);
  auto pEnd   = pBegin + nBytes;

  copy(pBegin, pEnd, back_inserter(m_buffer));
}


//
void CTestSourceSink::read(char* pBuffer, size_t nBytes)
{
  if (m_buffer.size() < nBytes) {
    throw std::runtime_error("TestSourceSink::get() does not have requested bytes stored");
  }

  auto itBegin = m_buffer.begin();
  auto itEnd   = itBegin + nBytes;

  auto itOut = reinterpret_cast<uint8_t*>(pBuffer);

  // copy the bytes requested
  copy(itBegin, itEnd, itOut);

  // erase the bytes sent out
  m_buffer.erase(itBegin, itEnd);
}
