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

#include "V8/CScalerBuffer.h"
#include "V8/CRawBuffer.h"
#include <ByteBuffer.h>
#include <BufferPtr.h>
#include <Deserializer.h>

using namespace std;

namespace DAQ
{
  namespace V8
  {
    CScalerBuffer::CScalerBuffer()
      : CScalerBuffer( bheader(), 0, 0, vector<uint32_t>())
    {
    }

    CScalerBuffer::CScalerBuffer(const CRawBuffer &rawBuffer)
      : m_header(rawBuffer.getHeader()),
        m_offsetBegin(),
        m_offsetEnd(),
        m_scalers()
    {

      if (m_header.type != SCALERBF && m_header.type != SNAPSCBF) {
        std::string errmsg = "CScalerBuffer::CScalerBuffer(CRawBuffer const&) ";
        errmsg += "Buffer is not of type SCALERBF or SNAPSCBF!";
        throw std::runtime_error(errmsg);
      }

      Buffer::Deserializer<Buffer::ByteBuffer> buffer(rawBuffer.getBuffer(),
                                                      rawBuffer.bufferNeedsSwap());

      // skip the header b/c we obtained it from the RawBuffer already
      uint16_t discard;
      for (std::size_t i=0; i<16; ++i) buffer >> discard;

      buffer >> m_offsetEnd;
      buffer >> discard;
      buffer >> discard;
      buffer >> discard;
      buffer >> m_offsetBegin;
      buffer >> discard;
      buffer >> discard;
      buffer >> discard;

      std::uint32_t value;
      m_scalers.reserve(m_header.nevt);
      for (std::size_t index=0; index<m_header.nevt; ++index) {
        buffer >> value;
        m_scalers.push_back(value);
      }

    }

    CScalerBuffer::CScalerBuffer(const bheader &header, std::uint32_t offsetBegin,
                  std::uint32_t offsetEnd,
                  const std::vector<uint32_t> &scalers)
      : m_header(header),
        m_offsetBegin(offsetBegin),
        m_offsetEnd(offsetEnd), m_scalers(scalers)
    {}

    bheader CScalerBuffer::getHeader() const {
      return m_header;
    }

    void CScalerBuffer::toRawBuffer(CRawBuffer &buffer) const
    {
      vector<uint8_t> empty(6);

      std::size_t nWords = computeNWords();

      if (nWords*2 > gBufferSize) {
        std::string errmsg("DAQ::V8::CScalerBuffer::toRawBuffer(CRawBuffer&) ");
        errmsg += "Scaler buffer size (" + std::to_string(nWords) + ") ";
        errmsg += "cannot fit in buffer (gBufferSize=" + std::to_string(gBufferSize) + ")";
        throw std::runtime_error(errmsg);
      }

      bheader header = m_header;
      header.nwds = nWords;
      header.nevt = m_scalers.size();

      Buffer::ByteBuffer newbuf;
      newbuf << m_header;
      newbuf << m_offsetEnd;
      newbuf << empty;
      newbuf << m_offsetBegin;
      newbuf << empty;
      newbuf << m_scalers;


      buffer.setBuffer(newbuf);
    }

    std::size_t CScalerBuffer::computeNWords() const {
      return (16 + 6 + 2*m_scalers.size());
    }
    std::uint32_t CScalerBuffer::getOffsetBegin() const
    {
      return m_offsetBegin;
    }

    std::uint32_t CScalerBuffer::getOffsetEnd() const
    {
      return m_offsetEnd;
    }

    std::vector<std::uint32_t> CScalerBuffer::getScalers() const
    {
      return m_scalers;
    }
  } // end of V8
} // end of DAQ
