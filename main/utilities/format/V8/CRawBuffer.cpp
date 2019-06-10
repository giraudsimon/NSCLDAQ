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

#include "V8/CRawBuffer.h"
#include <BufferPtr.h>
#include <Deserializer.h>

namespace DAQ
{
  namespace V8
  {


    //
    CRawBuffer::CRawBuffer(size_t size)
      : m_parsedHeader(), m_unparsedBuffer(), m_bytesNeededSwap(false)
    {
      m_unparsedBuffer.reserve(size);
    }

    
    //
    bheader CRawBuffer::getHeader() const
    {
      return m_parsedHeader;
    }


    //
    const Buffer::ByteBuffer& CRawBuffer::getBuffer() const
    { return m_unparsedBuffer; }


    //
    void CRawBuffer::setBuffer(const Buffer::ByteBuffer &buffer)
    {
      m_unparsedBuffer = buffer;
      m_unparsedBuffer.resize(gBufferSize, 0);

      // parse the header first assuming native byte orderig
      parseHeader(buffer, false);

      // now ask whether it should have been swapped
      if (m_parsedHeader.mustSwap()) {

        // oops. bytes should have been swapped, flag this and reparse header
        m_bytesNeededSwap = true;
        parseHeader(buffer, true);

      } else {
        m_bytesNeededSwap = false;
      }
    }


    //
    void CRawBuffer::toRawBuffer(CRawBuffer &buffer) const
    {
      buffer.setBuffer(getBuffer());
    }


    //
    void CRawBuffer::parseHeader(const Buffer::ByteBuffer &buffer, bool swap)
    {
      Buffer::Deserializer<Buffer::ByteBuffer> bufstream(buffer, swap);

      std::uint16_t tmp16;
      bufstream >> m_parsedHeader.nwds;
      bufstream >> tmp16;
      m_parsedHeader.type = BufferTypes(tmp16);
      bufstream >> m_parsedHeader.cks;
      bufstream >> m_parsedHeader.run;
      bufstream >> m_parsedHeader.seq;
      bufstream >> m_parsedHeader.nevt;
      bufstream >> m_parsedHeader.nlam;
      bufstream >> m_parsedHeader.cpu;
      bufstream >> m_parsedHeader.nbit;
      bufstream >> tmp16;
      m_parsedHeader.buffmt = BufferVersion(tmp16);
      bufstream >> m_parsedHeader.ssignature;
      bufstream >> m_parsedHeader.lsignature;
      bufstream >> m_parsedHeader.unused[0];
      bufstream >> m_parsedHeader.unused[1];
    }

  } // end of V8
} // end of DAQ
