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

#include "V8/CPhysicsEventBuffer.h"
#include <V8/CGenericBodyParser.h>
#include <V8/CRawBuffer.h>
#include <ByteOrder.h>
#include <ByteBuffer.h>

namespace DAQ {
  namespace V8 {
    
    ///////////////////////////////////////////////////////////////////////////
    // CPhysicsEvent Implementation
    //

    CPhysicsEvent::CPhysicsEvent(const Buffer::ByteBuffer &data, bool needsSwap)
      : m_needsSwap(needsSwap),
        m_buffer(data) {}

    //
    CPhysicsEvent::CPhysicsEvent(Buffer::ByteBuffer&& data, bool needsSwap)
      : m_needsSwap(needsSwap),
        m_buffer( move(data) ) {}

    //
    CPhysicsEvent::CPhysicsEvent(const CPhysicsEvent& rhs)
      : m_needsSwap(rhs.m_needsSwap),
        m_buffer(rhs.m_buffer) {}

    CPhysicsEvent::~CPhysicsEvent() {}

    //
    CPhysicsEvent& CPhysicsEvent::operator=(const CPhysicsEvent& rhs)
    {
      if (this != &rhs) {
        m_needsSwap = rhs.m_needsSwap;
        m_buffer    = rhs.m_buffer;
      }
      return *this;
    }

    //
    std::size_t CPhysicsEvent::getNTotalShorts() const {
      std::size_t nBytes = m_buffer.size();

      // this knowingly truncates the odd byte.
      return nBytes/sizeof(std::uint16_t);
    }

    //
    CPhysicsEvent::iterator CPhysicsEvent::begin() const {
      return iterator(m_buffer.begin(), BO::CByteSwapper(m_needsSwap));
    }

    //
    CPhysicsEvent::iterator CPhysicsEvent::end() const {
      return iterator(m_buffer.end(), BO::CByteSwapper(m_needsSwap));
    }

    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // CPhysicsEventBuffer Implementation
    //


    // Static member initialization
    CPhysicsEventBuffer::BodyTypePolicy
    CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::BufferPreference;


    //
    CPhysicsEventBuffer::CPhysicsEventBuffer()
      : m_header(), m_body(), m_mustSwap(false)
    {
      m_header.type   = DATABF;
    }

    //
    CPhysicsEventBuffer::CPhysicsEventBuffer(const bheader &header,
                                             const Buffer::ByteBuffer &rawBody)
      : m_header(header),
        m_body(),
        m_mustSwap(m_header.mustSwap())
    {
      parseBodyData(rawBody.begin(), rawBody.end());
    }

    //
    CPhysicsEventBuffer::CPhysicsEventBuffer(const bheader &header,
                                             const std::vector<std::uint16_t>& body,
                                             bool mustSwap)
      : m_header(header),
        m_body(),
        m_mustSwap(mustSwap)
    {
      Buffer::ByteBuffer buffer;
      buffer << body;
      parseBodyData(buffer.begin(), buffer.end());
    }

    //
    CPhysicsEventBuffer::CPhysicsEventBuffer(const CRawBuffer &rawBuffer)
      : m_header(rawBuffer.getHeader()),
        m_body(),
        m_mustSwap(rawBuffer.bufferNeedsSwap())
    {
      if (m_header.type != DATABF) {
        std::string errmsg = "CPhysicsEventBuffer::CPhysicsEventBuffer(CRawBuffer const&) ";
        errmsg += "Buffer is not of type DATABF!";
        throw std::runtime_error(errmsg);
      }

      std::size_t hdrSize = 16*sizeof(std::uint16_t);
      auto buf = rawBuffer.getBuffer();
      parseBodyData(buf.begin()+hdrSize, buf.end());
    }

    //
    CPhysicsEventBuffer::CPhysicsEventBuffer(const CPhysicsEventBuffer& rhs)
      : m_header(rhs.m_header),
        m_body(),
        m_mustSwap(rhs.m_mustSwap)
    {
      // deep copy
      for (auto pEvt : rhs.m_body) {
        m_body.push_back(std::shared_ptr<CPhysicsEvent>(new CPhysicsEvent(*pEvt)));
      }
    }

    //
    CPhysicsEventBuffer::~CPhysicsEventBuffer() {}

    //
    CPhysicsEventBuffer& CPhysicsEventBuffer::operator=(const CPhysicsEventBuffer& rhs)
    {
      if (this != &rhs) {
        m_header = rhs.m_header;

        // deep copy
        std::vector<std::shared_ptr<CPhysicsEvent> > new_body;
        for (auto& pEvt : rhs.m_body) {
          new_body.push_back(std::shared_ptr<CPhysicsEvent>(new CPhysicsEvent(*pEvt)));
        }
        m_body = new_body;

        m_mustSwap = rhs.m_mustSwap;
      }
      return *this;
    }


    //
    void CPhysicsEventBuffer::parseBodyData(Buffer::ByteBuffer::const_iterator beg,
                                            Buffer::ByteBuffer::const_iterator end)
    {
      if ( m_bodyType == BufferPreference ) {

        if (m_header.buffmt == StandardVsn) {
          parseStandardBody(beg, end);
        } else {
          throw std::runtime_error("Only buffer version 5 is supported");
        }

      } else {
        parseGeneralBody(beg, end);
      }
    }


    //
    void CPhysicsEventBuffer::parseStandardBody(Buffer::ByteBuffer::const_iterator beg,
                                                Buffer::ByteBuffer::const_iterator end)
    {
//      CStandardBodyParser parser;
      CGenericBodyParser parser( mapBodyType(Inclusive16BitWords) );
      Buffer::BufferPtr<uint16_t> begPtr(beg, m_mustSwap);
      Buffer::BufferPtr<uint16_t> endPtr(end, m_mustSwap);

      m_body = parser(m_header.nevt, begPtr, endPtr);
    }


    //
    void CPhysicsEventBuffer::parseGeneralBody(Buffer::ByteBuffer::const_iterator beg,
                                               Buffer::ByteBuffer::const_iterator end)
    {
      CGenericBodyParser parser( mapBodyType(m_bodyType) );

      Buffer::BufferPtr<uint16_t> begPtr(beg, m_mustSwap);
      Buffer::BufferPtr<uint16_t> endPtr(end, m_mustSwap);

      m_body = parser(m_header.nevt, begPtr, endPtr);
    }


    //
    bheader CPhysicsEventBuffer::getHeader() const {
      return m_header;
    }
    

    //
    CPhysicsEventBuffer::iterator CPhysicsEventBuffer::begin()
    {
      return m_body.begin();
    }


    //
    CPhysicsEventBuffer::const_iterator CPhysicsEventBuffer::begin() const
    {
      return m_body.begin();
    }


    //
    CPhysicsEventBuffer::iterator CPhysicsEventBuffer::end() 
    {
      return m_body.end();
    }

    //
    CPhysicsEventBuffer::const_iterator CPhysicsEventBuffer::end() const
    {
      return m_body.end();
    }


    // Because I don't know how to properly swap the body of a physics event, I
    // have to send the entire buffer back unswapped if it is not in native-byte order.
    void CPhysicsEventBuffer::toRawBuffer(CRawBuffer &buffer) const
    {
      bheader header = m_header;

      std::size_t nWords = computeNWords();
      if (nWords*sizeof(std::uint16_t) > gBufferSize) {
        std::string errmsg("DAQ::V8::CPhysicsEventBuffer::toRawBuffer(CRawBuffer&) ");
        errmsg += "Total event buffer size (" + std::to_string(nWords*sizeof(std::uint16_t)) + ") ";
        errmsg += "cannot fit in buffer (gBufferSize=" + std::to_string(gBufferSize) + ")";
        throw std::runtime_error(errmsg);
      }

      updateHeader(header);

      if (m_mustSwap) {
        swapBytesOfHeaderInPlace(header);
      }

      Buffer::ByteBuffer newbuf;
      newbuf << header;

      for (auto& pEvent : m_body) {
        newbuf << pEvent->getBuffer();
      }

      buffer.setBuffer(newbuf);

    }

    //
    bool CPhysicsEventBuffer::appendEvent(std::shared_ptr<CPhysicsEvent> pEvent)
    {
      bool successfullyAppended = true;

      std::size_t resultingNWords = computeNWords() + pEvent->getNTotalShorts();
      std::size_t resultingNBytes = resultingNWords*sizeof(std::uint16_t);

      if (resultingNBytes > gBufferSize) {
        successfullyAppended = false;
      } else {
        successfullyAppended = true;
        m_body.push_back(pEvent);
      }

      return successfullyAppended;
    }


    //
    std::size_t CPhysicsEventBuffer::getNBytesFree() const
    {
      std::size_t nBytesOccuppied = computeNWords()*sizeof(std::uint16_t);
      return (gBufferSize-nBytesOccuppied);
    }


    //
    void CPhysicsEventBuffer::swapBytesOfHeaderInPlace(bheader &header) const
    {
      BO::swapBytes(header.nwds);
      BO::swapBytes(header.type);
      BO::swapBytes(header.cks);
      BO::swapBytes(header.run);
      BO::swapBytes(header.seq);
      BO::swapBytes(header.nevt);
      BO::swapBytes(header.nlam);
      BO::swapBytes(header.cpu);
      BO::swapBytes(header.nbit);
      BO::swapBytes(header.buffmt);
      BO::swapBytes(header.ssignature);
      BO::swapBytes(header.lsignature);
      BO::swapBytes(header.unused[0]);
      BO::swapBytes(header.unused[1]);
    }


    //
    std::size_t CPhysicsEventBuffer::computeNWords() const
    {
      std::size_t nBytes = 16*sizeof(std::uint16_t); // size of header
      for (auto& pEvent : m_body) {
        nBytes += pEvent->getBuffer().size();
      }

      std::size_t nWords;
      if ((nBytes%2)==0) {
        nWords = nBytes/sizeof(std::uint16_t);
      } else {
        nWords = nBytes/sizeof(std::uint16_t) + 1;
      }

      return nWords;
    }

    //
    void CPhysicsEventBuffer::updateHeader(bheader& header) const
    {
      header.type = static_cast<std::uint16_t>(DATABF);
      header.nwds = static_cast<std::uint16_t>(computeNWords());
      header.nevt = static_cast<std::uint16_t>(m_body.size());
    }

    //
    PhysicsEventSizePolicy
    CPhysicsEventBuffer::mapBodyType(BodyTypePolicy type) const
    {
      PhysicsEventSizePolicy genericType = V8::Inclusive16BitWords;
      switch (type) {
        case (Inclusive16BitWords) :
          genericType = V8::Inclusive16BitWords;
          break;
        case (Exclusive16BitWords) :
          genericType = V8::Exclusive16BitWords;
          break;
        case (Inclusive32BitBytes) :
          genericType = V8::Inclusive32BitBytes;
          break;
        case (Inclusive32BitWords) :
          genericType = V8::Inclusive32BitWords;
          break;
      }
      return genericType;
    }

  } // namespace V8
} // namespace DAQ
