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

#include "V8/CTextBuffer.h"
#include <V8/CRawBuffer.h>
#include <BufferPtr.h>
#include <algorithm>

namespace DAQ {
  namespace V8 {


    //
    CTextBuffer::CTextBuffer(BufferTypes type)
      : m_header(), m_strings()
    {
      m_header.type = type;
    }


    //
    CTextBuffer::CTextBuffer(const bheader &header,
                             const std::vector<std::string> &strings)
      : m_header(header), m_strings(strings)
    {}


    //
    CTextBuffer::CTextBuffer(const CRawBuffer &rawBuf)
      : m_header(rawBuf.getHeader()), m_strings()
    {
      validateTypeToConstructFrom(); // throws if bad

      std::uint16_t totalShorts = extractTotalShorts(rawBuf.getBuffer(),
                                                     rawBuf.bufferNeedsSwap());

      // locate beginning of body (i.e. skip the 16-word header)
      auto beg     = rawBuf.getBuffer().begin() + 16*sizeof(std::uint16_t);
      auto deadEnd = beg + totalShorts*sizeof(std::uint16_t);

      validateDeadEndMeaningful(deadEnd, rawBuf.getBuffer().end());

      // Skip over the 16-bit total byte size
      beg += sizeof(std::uint16_t);

      parseStringsFromBuffer(beg, deadEnd);

    }


    //
    CTextBuffer::CTextBuffer(const CTextBuffer &rhs)
      : m_header(rhs.m_header), m_strings(rhs.m_strings)
    {}


    //
    void CTextBuffer::toRawBuffer(CRawBuffer &buffer) const
    {
      // make sure that the number of words are correctly computed

      std::size_t nWords = 16+totalShorts();

      if (nWords > gBufferSize) {
        std::string errmsg("DAQ::V8::CTextBuffer::toRawBuffer(CRawBuffer&) ");
        errmsg += "Text buffer size (" + std::to_string(nWords) + ") ";
        errmsg += "cannot fit in buffer (gBufferSize=" + std::to_string(gBufferSize) + ")";
        throw std::runtime_error(errmsg);
      }

      bheader header = m_header;
      header.nwds = nWords;
      header.nevt = m_strings.size();

      Buffer::ByteBuffer buf;
      buf << header;
      buf << totalShorts();
      for (auto& str : m_strings) {
        buf << str.c_str();
      }

      buffer.setBuffer(buf);
    }


    //
    std::uint16_t CTextBuffer::totalShorts() const
    {
      std::uint32_t nBytes = totalBytes();

      return ((nBytes%2)==0) ? nBytes/2 : (nBytes+1)/2;
    }


    //
    std::uint32_t CTextBuffer::totalBytes() const
    {
      std::uint32_t totalBytes = sizeof(std::uint16_t);

      for (auto& element : m_strings) {
        if ((element.size() % 2) == 0) {
          totalBytes += element.size() + 2;
        } else {
          totalBytes += element.size() + 1;
        }
      }

      return totalBytes;
    }


    //
    bool CTextBuffer::appendString(const string &str)
    {
      std::uint32_t nBytes = totalBytes() + 16*sizeof(std::uint16_t); // body + header

      std::size_t strLength = str.size();
      if ( (strLength%2) == 0 ) strLength += 2;
      else                      strLength += 1;

      bool successfullyAppended = true;
      if (nBytes + strLength > gBufferSize ) {
        successfullyAppended = false;
      } else {
        successfullyAppended = true;
        m_strings.push_back(str);
      }

      return successfullyAppended;
    }


    //
    std::size_t CTextBuffer::getNBytesFree() const
    {
      std::uint32_t nBytesOccupied = totalBytes() + 16*sizeof(std::uint16_t); // body + header
      return gBufferSize - nBytesOccupied;
    }


    //
    std::uint16_t CTextBuffer::extractTotalShorts(const Buffer::ByteBuffer &buffer, bool needsSwap) const
    {
      Buffer::BufferPtr<std::uint16_t> pSize(buffer.begin(), needsSwap);
      pSize += 16; // skip over 16 shorts
      return *pSize;
    }


    //
    void CTextBuffer::validateTypeToConstructFrom()
    {
      if (m_header.type != STATEVARBF && m_header.type != RUNVARBF
          && m_header.type != PKTDOCBF && m_header.type != PARAMDESCRIP) {

        std::string errmsg("CTextBuffer::CTextBuffer(CRawBuffer const&) ");
        errmsg += "Cannot construct from buffer type ";
        errmsg += std::to_string(m_header.type);

        throw std::runtime_error(errmsg);

      }
    }


    //
    void CTextBuffer::validateDeadEndMeaningful(Buffer::ByteBuffer::const_iterator deadEnd,
                                                Buffer::ByteBuffer::const_iterator bufferEnd) {
      if (deadEnd > bufferEnd) {
        std::string errmsg("CTextBuffer::CTextBuffer(CRawBuffer const&) ");
        errmsg += "Inclusive size indicates more data exists than is valid.";

        throw std::runtime_error(errmsg);
      }
    }


    //
    void CTextBuffer::parseStringsFromBuffer(Buffer::ByteBuffer::const_iterator beg,
                                             Buffer::ByteBuffer::const_iterator end) {

      auto origBeg = beg;
      auto begEnd = beg;
      // the +1 check is because the char padding skip can jump by 1 or 2
      while ((beg != end) && (beg != end+1)) {
        begEnd = std::find(beg, end, '\0');
        m_strings.push_back(std::string(beg, begEnd));

        beg = skipNullCharPadding(beg, begEnd);
      }

    }


    //
    Buffer::ByteBuffer::const_iterator
    CTextBuffer::skipNullCharPadding(Buffer::ByteBuffer::const_iterator beg,
                                     Buffer::ByteBuffer::const_iterator begEnd)
    {
      Buffer::ByteBuffer::const_iterator startNextStr;
      if ((std::distance(beg, begEnd)%2)==0) {
        startNextStr = begEnd+2;
      } else {
        startNextStr = begEnd+1;
      }

      return startNextStr;
    }

    } // namespace V8
} // namespace DAQ
