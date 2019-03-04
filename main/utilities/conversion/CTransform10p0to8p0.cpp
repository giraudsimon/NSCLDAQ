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

#include "CTransform10p0to8p0.h"
#include <V10/DataFormatV10.h>
#include <V10/CRingItem.h>
#include <V8/CRawBuffer.h>
#include <V10/CRingScalerItem.h>
#include <V10/CRingTimestampedRunningScalerItem.h>
#include <V8/CScalerBuffer.h>
#include <V10/CRingStateChangeItem.h>
#include <V8/CControlBuffer.h>
#include <V10/CPhysicsEventItem.h>
#include <V10/CRingPhysicsEventCountItem.h>
#include <V10/CRingTextItem.h>
#include <V10/CRingItemFactory.h>
#include <V8/CTextBuffer.h>
#include <V8/CVoidBuffer.h>
#include <V8/format_cast.h>

#include <memory>

using namespace std;

namespace DAQ {
  namespace Transform {
    
    // Initialize and start a new, empty physics event buffer
    CTransform10p0to8p0::CTransform10p0to8p0()
      : m_run(0),
        m_nTriggersProcessed(0),
        m_samplingFactor(1.0),
        m_physicsBuffer(),
        m_textBuffers()
    {
       startNewPhysicsBuffer();
    }

    // Main entry point
    CTransform10p0to8p0::FinalType CTransform10p0to8p0::operator ()(const InitialType& item)
    {
      V10::CRingItemFactory factory;
      std::unique_ptr<V10::CRingItem> pItem(factory.createRingItem(item));

      switch (pItem->type()) {
        case V10::INCREMENTAL_SCALERS:
          return V8::format_cast<V8::CRawBuffer>(transformIncrScaler(*pItem));
          break;
        case V10::TIMESTAMPED_NONINCR_SCALERS:
          return V8::format_cast<V8::CRawBuffer>(transformNonIncrScaler(*pItem));
          break;
        case V10::BEGIN_RUN:
        case V10::END_RUN:
        case V10::PAUSE_RUN:
        case V10::RESUME_RUN:
          return V8::format_cast<V8::CRawBuffer>(transformStateChange(*pItem));
          break;
        case V10::PHYSICS_EVENT:
          return transformPhysicsEvent(*pItem);
          break;
        case V10::MONITORED_VARIABLES:
        case V10::PACKET_TYPES:
          return transformText(*pItem);
          break;
        case V10::EVB_FRAGMENT:
        case V10::EVB_UNKNOWN_PAYLOAD: // these do not transform.
          break;
        case V10::PHYSICS_EVENT_COUNT:
          updateSamplingFactor(*pItem);
          break;
      default:
          std::string errmsg("CTransform10p0to8p0::dispatch()");
          errmsg += "Unsupported type (" + to_string(item.type()) + ") found";
          throw std::runtime_error(errmsg);
          break;
      }

      // Fill the header with a VOID header
      Buffer::ByteBuffer buffer;
      buffer << V8::bheader();

      DAQ::V8::CRawBuffer voidBuffer;
      voidBuffer.setBuffer(buffer);
      return voidBuffer;
    }

    //
    V8::CScalerBuffer CTransform10p0to8p0::transformIncrScaler(const InitialType &item)
    {
      auto& v10item = dynamic_cast<const V10::CRingScalerItem&>(item);

      V8::bheader header;
      header.type = V8::SCALERBF;
      header.nevt = v10item.getScalerCount();
      header.run  = m_run;
      header.seq  = computeSequence();
      V8::CScalerBuffer sclrBuf(header, v10item.getStartTime(), v10item.getEndTime(),
                                v10item.getScalers());
      return sclrBuf;
    }

    //
    V8::CScalerBuffer CTransform10p0to8p0::transformNonIncrScaler(const InitialType &item)
    {

      auto& v10item = dynamic_cast<const V10::CRingTimestampedRunningScalerItem&>(item);

      V8::bheader header;
      header.type = V8::SCALERBF;
      header.nevt = v10item.getScalerCount();
      header.run  = m_run;
      header.seq  = computeSequence();
      V8::CScalerBuffer sclrBuf(header, v10item.getOffsetStart(), v10item.getOffsetEnd(),
                                v10item.getScalers());
      return sclrBuf;

    }

    //
    V8::CControlBuffer CTransform10p0to8p0::transformStateChange(const InitialType &item)
    {
      auto& v10item = dynamic_cast<const V10::CRingStateChangeItem&>(item);

      if (v10item.type() == V10::BEGIN_RUN) {
        resetStatistics();
      }

      m_run = v10item.getRunNumber();

      V8::bheader header;
      header.type = mapControlType(v10item.type());
      header.run = m_run;
      header.seq = computeSequence();

      std::string title = v10item.getTitle();
      title.resize(80, ' ');
      title.at(79) = '\0';

      V8::CControlBuffer ctlBuf(header,
                                title,
                                v10item.getElapsedTime(),
                                V8::to_bftime(v10item.getTimestamp()));

      return ctlBuf;
    }

    //
    V8::CRawBuffer CTransform10p0to8p0::transformPhysicsEvent(const InitialType &item)
    {
      const V10::CPhysicsEventItem& v10item
                          = dynamic_cast<const V10::CPhysicsEventItem&>(item);


      Buffer::ByteBuffer body = v10item.getBodyData();
      std::shared_ptr<V8::CPhysicsEvent> pEvent(new V8::CPhysicsEvent(body, v10item.mustSwap()));

      V8::CRawBuffer returnBuffer;
      if ( m_physicsBuffer.appendEvent(pEvent) ) {
        ++m_nTriggersProcessed;

        if (m_physicsBuffer.getNBytesFree() == 0) {
          returnBuffer = V8::format_cast<V8::CRawBuffer>(m_physicsBuffer);

          startNewPhysicsBuffer();
        } else {
          returnBuffer = V8::format_cast<V8::CRawBuffer>(V8::CVoidBuffer());
        }

      } else {
        returnBuffer = V8::format_cast<V8::CRawBuffer>(m_physicsBuffer);

        startNewPhysicsBuffer();
        m_physicsBuffer.appendEvent(pEvent);
        ++m_nTriggersProcessed;
      }

      return returnBuffer;
    }

    //
    V8::CRawBuffer CTransform10p0to8p0::transformText(const InitialType &item)
    {
      const V10::CRingTextItem& v10item = dynamic_cast<const V10::CRingTextItem&>(item);

      auto strings = v10item.getStrings();

      appendNewTextBuffer(v10item.type());

      std::size_t nStr = strings.size();
      for (std::size_t i=0; i<nStr; ++i) {
        std::string& str = strings.at(i);

        auto& textBuf = m_textBuffers.back();

        if ( textBuf.appendString(str) ) {
          // text buffer was successfully appended

          if (textBuf.getNBytesFree() == 0) {
            appendNewTextBuffer(v10item.type());
          }

        } else {
          // text buffer was not appended.

          appendNewTextBuffer(item.type());
          m_textBuffers.back().appendString(str);
        }
      }

      // send out the first V8::CTextBuffer and remove it from
      // the stored list of buffers so it does not get emitted twice
      const V8::CTextBuffer textBuf = m_textBuffers.front();
      m_textBuffers.erase(m_textBuffers.begin());

      V8::CRawBuffer rawBuf = V8::format_cast<V8::CRawBuffer>(textBuf);
      return rawBuf;
    }

    //
    const V8::CPhysicsEventBuffer& CTransform10p0to8p0::getCurrentPhysicsBuffer() const
    {
      return m_physicsBuffer;
    }

    //
    const std::vector<V8::CTextBuffer>& CTransform10p0to8p0::getStagedTextBuffers() const
    {
      return m_textBuffers;
    }

    //
    void CTransform10p0to8p0::updateSamplingFactor(const InitialType &item)
    {
      auto& v10item = dynamic_cast<const V10::CRingPhysicsEventCountItem&>(item);
      double observedTriggers = static_cast<double>(m_nTriggersProcessed);
      double realTriggers     = static_cast<double>(v10item.getEventCount());
      if (realTriggers != 0) {
        m_samplingFactor = observedTriggers/realTriggers;
      } else {
        m_samplingFactor = 1.0; // realTriggers=0 means that we have no info to compute something else
      }
    }

    //
    std::uint16_t CTransform10p0to8p0::mapControlType(std::uint16_t type) const
    {
      std::uint16_t v8type;

      switch(type) {
        case V10::BEGIN_RUN:
          v8type = V8::BEGRUNBF;
          break;
        case V10::END_RUN:
          v8type = V8::ENDRUNBF;
          break;
        case V10::PAUSE_RUN:
          v8type = V8::PAUSEBF;
          break;
        case V10::RESUME_RUN:
          v8type = V8::RESUMEBF;
          break;
        default:
          throw std::runtime_error("CTransform10p0to8p0::mapControlType(std::uint16_t) unknown type provided");
          break;
      }

      return v8type;
    }

    //
    std::uint16_t CTransform10p0to8p0::mapTextType(std::uint16_t type) const
    {
      std::uint16_t v8type;

      switch(type) {
        case V10::MONITORED_VARIABLES:
          v8type = V8::RUNVARBF;
          break;
        case V10::PACKET_TYPES:
          v8type = V8::PKTDOCBF;
          break;
        default:
          throw std::runtime_error("CTransform10p0to8p0::mapTextType(std::uint16_t) unknown type provided");
          break;
      }

      return v8type;
    }
    
    //
    void CTransform10p0to8p0::startNewPhysicsBuffer()
    {

      V8::bheader header;
      header.type = V8::DATABF;
      header.nevt = 0;
      header.run = m_run;
      header.seq = computeSequence();

      m_physicsBuffer = V8::CPhysicsEventBuffer(header,  Buffer::ByteBuffer({}));
    }

    //
    std::uint32_t CTransform10p0to8p0::computeSequence() const
    {
      return m_nTriggersProcessed/m_samplingFactor;
    }

    //
    void CTransform10p0to8p0::appendNewTextBuffer(std::uint16_t type) {
      V8::bheader header;
      header.type = mapTextType(type);
      header.run  = m_run;
      header.seq  = computeSequence();

      V8::CTextBuffer buffer(header, {});
      m_textBuffers.push_back(buffer);
    }

    //
    void CTransform10p0to8p0::resetStatistics()
    {
      m_samplingFactor = 1.0;
      m_nTriggersProcessed = 0;
    }
  } // namespace Transform
} // namespace DAQ
