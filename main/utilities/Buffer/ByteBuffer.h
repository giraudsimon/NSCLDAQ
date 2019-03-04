
#ifndef CBUFFER_H
#define CBUFFER_H

#include <vector>
#include <array>
#include <cstring>
#include <string>
#include <iterator>

#include <iostream>

namespace DAQ
{
  namespace Buffer
  {

    using ByteBuffer = std::vector<std::uint8_t>;

  }  // end of Buffer
} // end of DAQ




template<class T>
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const T& value)
{
  // the char* can always refer to another type
  const char* beg = reinterpret_cast<const char*>(&value);
  const char* end = beg + sizeof(value);

  // make sure we only allocate once the memory we need
  buffer.reserve(buffer.size() + (end - beg));

  // copy the data using a back_inserter (calls push_back when assigning),
  // so that the buffer is properly sized when the copy is completed.
  std::copy(beg, end, std::back_inserter(buffer));

  return buffer;
}

template<> inline
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const std::string& rhs)
{
  buffer << std::uint16_t(rhs.size());
  buffer.insert(buffer.end(), rhs.begin(), rhs.end());

  return buffer;
}

template<> inline
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const DAQ::Buffer::ByteBuffer& rhs)
{
  buffer.insert(buffer.end(), rhs.begin(), rhs.end());

  return buffer;
}

/*!
 *  \brief Data insertion using a vector
 */
template<class T>
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const std::vector<T>& data) {

  std::size_t nBytes = data.size() * sizeof(T);

  // the char* can always refer to another type
  const char* beg = reinterpret_cast<const char*>(data.data());
  const char* end = beg + nBytes;

  // make sure we only allocate once the memory we need
  buffer.reserve(buffer.size() + nBytes);

  // copy the data using a back_inserter (calls push_back when assigning),
  // so that the buffer is properly sized when the copy is completed.
  std::copy(beg, end, std::back_inserter(buffer));

  return buffer;
}


inline DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const char* rhs)
{
  std::size_t length = strlen(rhs);

  buffer.insert(buffer.end(), rhs, rhs+length);

  // even lengths have a \0\0 trailer while odd lengths have \0
  if ((length % 2)==0) buffer << uint16_t(0);
  else                 buffer << uint8_t(0);

  return buffer;
}

template<class T, std::size_t N>
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const T (&arr)[N]) {
  for (std::size_t i=0; i<N; ++i) {
    buffer << arr[i];
  }

  return buffer;
}

template<class T, std::size_t N>
DAQ::Buffer::ByteBuffer& operator<<(DAQ::Buffer::ByteBuffer& buffer,
                                    const std::array<T,N>& array) {
  for (auto & element : array) {
    buffer << element;
  }

  return buffer;
}

#endif
