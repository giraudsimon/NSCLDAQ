/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

#include <BufferIOV8.h>
#include <V8/CRawBuffer.h>
#include <ByteBuffer.h>
#include <CDataSource.h>
#include <CDataSink.h>

#include <iostream>
#include <stdexcept>

std::istream& operator>>(std::istream& stream, DAQ::V8::CRawBuffer& buffer)
{
  DAQ::Buffer::ByteBuffer bytes(DAQ::V8::gBufferSize);

  auto pData = reinterpret_cast<char*>(bytes.data());
  stream.read(pData, bytes.size());

  // i've yet to see a gaurantee that the above won't fail,
  // check that we got everything we asked for.
  if (stream.gcount() != DAQ::V8::gBufferSize && stream.good()) {
    std::string errmsg ("operator>>(std::istream&, DAQ::V8::CRawBuffer&) ");
    errmsg += "failed to read entire buffer.";
    throw std::runtime_error(errmsg);
  }

  buffer.setBuffer(bytes);

  return stream;
}

CDataSource& operator>>(CDataSource& stream, DAQ::V8::CRawBuffer& buffer)
{
  DAQ::Buffer::ByteBuffer bytes(DAQ::V8::gBufferSize);

  auto pData = reinterpret_cast<char*>(bytes.data());
  stream.read(pData, bytes.size());

  // CDataSource gaurantees that we get the entirety of our requested data.

  buffer.setBuffer(bytes);

  return stream;
}


std::ostream& operator<<(std::ostream& stream, const DAQ::V8::CRawBuffer& buffer)
{
  if (buffer.getBuffer().size() != DAQ::V8::gBufferSize) {
    std::string errmsg ("operator<<(std::ostream&, const DAQ::V8::CRawBuffer&) ");
    errmsg += "buffer size is different than DAQ::V8::gBufferSize!";
    throw std::runtime_error(errmsg);
  }

  auto pData = reinterpret_cast<const char*>(buffer.getBuffer().data());
  stream.write(pData, buffer.getBuffer().size());

  return stream;
}

CDataSink& operator<<(CDataSink& stream, const DAQ::V8::CRawBuffer& buffer)
{
  if (buffer.getBuffer().size() != DAQ::V8::gBufferSize) {
    std::string errmsg ("operator<<(CDataSource&, const DAQ::V8::CRawBuffer&) ");
    errmsg += "buffer size is different than DAQ::V8::gBufferSize";
    throw std::runtime_error(errmsg);
  }

  stream.put(buffer.getBuffer().data(), buffer.getBuffer().size());

  return stream;
}
