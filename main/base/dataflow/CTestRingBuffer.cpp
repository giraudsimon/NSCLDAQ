
#include "CTestRingBuffer.h"
#include <algorithm>
#include <stdexcept>

using namespace std;

CTestRingBuffer::CTestRingBuffer(string name)
  : CRingBuffer(name)
{
}

size_t CTestRingBuffer::put(const void* pBuffer, size_t nBytes, unsigned long timeout)
{
  auto pBegin = reinterpret_cast<const char*>(pBuffer);
  auto pEnd   = pBegin + nBytes;

  copy(pBegin, pEnd, back_inserter(m_buffer));
  return nBytes;
}

size_t CTestRingBuffer::get(void* pBuffer, size_t maxBytes, size_t minBytes,
                            unsigned long timeout)
{
  size_t nBufferedBytes = m_buffer.size();
  if (minBytes > nBufferedBytes) {
    throw std::runtime_error("CTestRingBuffer::get() Insufficient data to satisfy request");
  }

  auto nBytes = min(nBufferedBytes, maxBytes);
  read(reinterpret_cast<char*>(pBuffer), nBytes);
  return nBytes;
}

size_t CTestRingBuffer::peek(void* pBuffer, size_t maxBytes)
{
  size_t actualBytes = min(maxBytes, availableData());
  
  auto pOutput = reinterpret_cast<char*>(pBuffer);
  auto beg = m_buffer.begin();
  copy(beg, beg+actualBytes, pOutput);

  return actualBytes;
}

void CTestRingBuffer::skip(size_t nBytes)
{
  vector<char> tempBuffer(nBytes);
  read(tempBuffer.data(), nBytes);
}

size_t CTestRingBuffer::availableData()
{
  return m_buffer.size();
}

//
void CTestRingBuffer::read(char* pBuffer, size_t nBytes)
{
  if (m_buffer.size() < nBytes) {
    throw std::runtime_error("CTestRingBuffer::read() does not have requested bytes stored");
  }

  auto itBegin = m_buffer.begin();
  auto itEnd   = itBegin + nBytes;

  auto itOut = reinterpret_cast<uint8_t*>(pBuffer);

  // copy the bytes requested
  copy(itBegin, itEnd, itOut);

  // erase the bytes sent out
  m_buffer.erase(itBegin, itEnd);
}
