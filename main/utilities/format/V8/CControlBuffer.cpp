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

#include "V8/CControlBuffer.h"
#include <V8/CRawBuffer.h>
#include <Deserializer.h>
#include <stdexcept>
#include <typeinfo>

namespace DAQ {
  namespace V8 {
    
    //
    CControlBuffer::CControlBuffer() : m_header(), m_title(), m_offset(), m_time()
    {
      setTitle(m_title);
    }

    //
    CControlBuffer::CControlBuffer(const bheader &header, const std::string &title,
                                   std::uint32_t offset, const bftime &time)
      : m_header(header), m_title(), m_offset(offset), m_time(time)
    {
      setTitle(title);
    }

    //
    CControlBuffer::CControlBuffer(const CRawBuffer &rawBuf)
      : m_header(rawBuf.getHeader()), m_title(), m_offset(), m_time()
    {
      Buffer::Deserializer<Buffer::ByteBuffer> databuf(rawBuf.getBuffer(),
                                                       rawBuf.bufferNeedsSwap());

      // skip header
      std::uint16_t discard;
      for (std::size_t i=0; i<16; ++i) databuf >> discard;

      // extract the title
      uint8_t title[80];
      databuf.extract(title, title+80);
      m_title.assign(title, title+80);

      databuf >> m_offset;
      databuf >> m_time.month;
      databuf >> m_time.day;
      databuf >> m_time.year;
      databuf >> m_time.hours;
      databuf >> m_time.min;
      databuf >> m_time.sec;
      databuf >> m_time.tenths;
    }

    //
    void CControlBuffer::toRawBuffer(CRawBuffer &buffer) const
    {
      if (computeNWords() > gBufferSize) {
        std::string errmsg("DAQ::V8::CControlBuffer::toRawBuffer(CRawBuffer&) ");
        errmsg += "Total control buffer size (" + std::to_string(computeNWords()) + ") ";
        errmsg += "cannot fit in buffer (gBufferSize=" + std::to_string(gBufferSize) + ")";
        throw std::runtime_error(errmsg);
      }

      bheader header = m_header;
      header.nwds = computeNWords();
      header.nevt = 0; // we never have an event

      Buffer::ByteBuffer tmpBuf;
      tmpBuf << header;
      tmpBuf.insert(tmpBuf.end(), m_title.begin(), m_title.end());
      tmpBuf << m_offset;
      tmpBuf << m_time;

      buffer.setBuffer(tmpBuf);

    }
    
    //
    void CControlBuffer::setTitle(const std::string &title)
    {
      if (title.size() > 80) {
        throw std::runtime_error("CControlBuffer::CControlBuffer() title cannot exceed 80 characters");
      } else {
        m_title = title;
        m_title.resize(80, ' ');
      }

    }

    //
    std::string CControlBuffer::getTitle() const
    {
      return m_title;
    }

  } // namespace V8
} // namespace DAQ
