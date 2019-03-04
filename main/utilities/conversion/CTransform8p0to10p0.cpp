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

#include "CTransform8p0to10p0.h"

#include <V8/DataFormatV8.h>
#include <V8/CRawBuffer.h>
#include <V8/CScalerBuffer.h>
#include <V8/CPhysicsEventBuffer.h>
#include <V8/CControlBuffer.h>
#include <V8/CTextBuffer.h>
#include <V8/format_cast.h>

#include <V10/CRingItem.h>
#include <V10/CRingScalerItem.h>
#include <V10/CRingStateChangeItem.h>
#include <V10/CRingTextItem.h>
#include <V10/CPhysicsEventItem.h>

#include <make_unique.h>

#include <string>
#include <stdexcept>
#include <chrono>

using namespace std;

namespace DAQ {
  namespace Transform {
    
    //
    CTransform8p0to10p0::FinalType CTransform8p0to10p0::operator ()(const InitialType& item)
    {

      switch (item.getHeader().type) {
        case V8::SCALERBF:
        case V8::SNAPSCBF:
          return transformScaler(item);
          break;
        case V8::BEGRUNBF:
        case V8::ENDRUNBF:
        case V8::PAUSEBF:
        case V8::RESUMEBF:
          return transformControl(item);
          break;
        case V8::DATABF:
          return transformPhysicsEvent(item);
          break;
        case V8::STATEVARBF:
        case V8::RUNVARBF:
        case V8::PKTDOCBF:
        case V8::PARAMDESCRIP:
          return transformText(item);
          break;
      default:
          std::string errmsg("CTransform8p0to10p0::dispatch()");
          errmsg += "Unsupported type (" + to_string(item.getHeader().type) + ") found";
          throw std::runtime_error(errmsg);
          break;
      }

      return V10::CRingItem(1);
    }
    
    //
    V10::CRingScalerItem CTransform8p0to10p0::transformScaler(const InitialType &item)
    {
      using namespace std::chrono;

      auto sclrBuf = V8::format_cast<V8::CScalerBuffer>(item);

      V10::CRingScalerItem newItem(sclrBuf.getOffsetBegin(),
                                         sclrBuf.getOffsetEnd(),
                                         system_clock::to_time_t(system_clock::now()),
                                         sclrBuf.getScalers());

      return newItem;
    }

    //
    V10::CRingStateChangeItem CTransform8p0to10p0::transformControl(const InitialType &item)
    {
      auto ctlBuf = V8::format_cast<V8::CControlBuffer>(item);

      time_t tstamp      = convertToTime_t(ctlBuf.getTimeStruct());
      std::uint16_t run  = ctlBuf.getHeader().run;
      std::uint16_t type = ctlBuf.getHeader().type;

      V10::CRingStateChangeItem v10item(mapControlType(type),
                                              run,
                                              ctlBuf.getOffset(),
                                              tstamp,
                                              ctlBuf.getTitle());

      return v10item;
    }

    //
    V10::CPhysicsEventItem CTransform8p0to10p0::transformPhysicsEvent(const InitialType &item)
    {
      m_physicsEvents.clear();

      auto evtBuf = V8::format_cast<V8::CPhysicsEventBuffer>(item);

      auto it = evtBuf.begin();
      auto end = evtBuf.end();
      while (it != end) {
        auto& pEvent = *it;

        transformOnePhysicsEvent(pEvent);
        ++it;
      }

      V10::CPhysicsEventItem firstEvent( m_physicsEvents.front() );
      m_physicsEvents.erase(m_physicsEvents.begin());

      return firstEvent;
    }

    //
    void CTransform8p0to10p0::transformOnePhysicsEvent(const std::shared_ptr<DAQ::V8::CPhysicsEvent>& pEvent)
    {
      // make sure that we construct a physics event big enough to handle any V8 buffer
      m_physicsEvents.emplace(m_physicsEvents.end(), V10::PHYSICS_EVENT,
                              pEvent->getNTotalShorts()*sizeof(std::uint16_t));

      auto& v10Item = m_physicsEvents.back();
      char* pBody = reinterpret_cast<char*>(v10Item.getBodyPointer());
      auto v8Buffer = pEvent->getBuffer();
      pBody = std::copy(v8Buffer.begin(), v8Buffer.end(), pBody);
      v10Item.setBodyCursor(pBody);
      v10Item.updateSize();
    }

    //
    V10::CRingTextItem CTransform8p0to10p0::transformText(const InitialType &item)
    {
      auto textBuf = V8::format_cast<V8::CTextBuffer>(item);

      uint32_t v10type;
      std::uint16_t v8type = textBuf.getHeader().type;
      if (v8type == V8::STATEVARBF || v8type == V8::RUNVARBF) {
        v10type = V10::MONITORED_VARIABLES;
      } else if (v8type == V8::PKTDOCBF) {
        v10type = V10::PACKET_TYPES;
      } else {
        std::string errmsg("CTransform8p0to10p0::transformText() ");
        errmsg += "No known conversion of version 8 text type " + to_string(v8type) + " ";
        errmsg += "to a version 10 text type";
        throw std::runtime_error(errmsg);
      }

      V10::CRingTextItem textItem(v10type, textBuf.getStrings());
      return textItem;
    }

    //
    std::time_t CTransform8p0to10p0::convertToTime_t(const V8::bftime &tstruct) const
    {
      std::tm calTime;
      calTime.tm_mon  = tstruct.month;
      calTime.tm_mday = tstruct.day;
      calTime.tm_year = tstruct.year - 1900; // tm_year is number of years since 1900
      calTime.tm_hour = tstruct.hours;
      calTime.tm_min  = tstruct.min;
      calTime.tm_sec  = tstruct.sec;
      calTime.tm_isdst= -1;

      return std::mktime(&calTime);
    }

    //
    std::uint16_t CTransform8p0to10p0::mapControlType(std::uint16_t type) const
    {
      std::uint16_t v10type;

      switch(type) {
        case V8::BEGRUNBF:
          v10type = V10::BEGIN_RUN;
          break;
        case V8::ENDRUNBF:
          v10type = V10::END_RUN;
          break;
        case V8::PAUSEBF:
          v10type = V10::PAUSE_RUN;
          break;
        case V8::RESUMEBF:
          v10type = V10::RESUME_RUN;
          break;
        default:
          throw std::runtime_error("CTransform8p0to10p0::mapControlType(std::uint16_t) unknown type provided");
          break;
      }

      return v10type;

    }

  } // namespace Transform
} // namespace DAQ
