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

#include <CTransform10p0to11p0.h>
#include <V10/CRingItemFactory.h>
#include <V10/CRingScalerItem.h>
#include <V10/CRingTimestampedRunningScalerItem.h>
#include <V11/CRingScalerItem.h>
#include <V10/CRingStateChangeItem.h>
#include <V11/CRingStateChangeItem.h>
#include <V10/CPhysicsEventItem.h>
#include <V11/CPhysicsEventItem.h>
#include <V10/CRingPhysicsEventCountItem.h>
#include <V11/CRingPhysicsEventCountItem.h>
#include <V10/CRingTextItem.h>
#include <V11/CRingTextItem.h>
#include <V10/CRingFragmentItem.h>
#include <V11/CRingFragmentItem.h>
#include <V10/DataFormatV10.h>
#include <V11/DataFormatV11.h>

#include <stdexcept>
#include <iostream>
#include <memory>

using namespace std;

namespace DAQ {
  namespace Transform {


    //
    CTransform10p0to11p0::FinalType
    CTransform10p0to11p0::operator()(InitialType& item)
    {
      unique_ptr<InitialType> pItem(V10::CRingItemFactory::createRingItem(item));

      return dispatch(*pItem);
    }


    //
    CTransform10p0to11p0::FinalType
    CTransform10p0to11p0::dispatch(InitialType& item)
    {
      switch (item.type()) {
        case V10::INCREMENTAL_SCALERS :
          return transformScaler(item);
          break;
        case V10::BEGIN_RUN:
        case V10::END_RUN:
        case V10::PAUSE_RUN:
        case V10::RESUME_RUN:
          return transformStateChange(item);
          break;
        case V10::PHYSICS_EVENT:
          return transformPhysicsEvent(item);
          break;
        case V10::PHYSICS_EVENT_COUNT:
          return transformPhysicsEventCount(item);
          break;
        case V10::MONITORED_VARIABLES:
        case V10::PACKET_TYPES:
          return transformText(item);
          break;
        case V10::TIMESTAMPED_NONINCR_SCALERS:
          return transformNonIncrScaler(item);
          break;
        case V10::EVB_FRAGMENT:
          return transformFragment(item);
          break;
        default:
          std::string errmsg("CTransform10p0to11p0::dispatch()");
          errmsg += "Unsupported type (" + to_string(item.type()) + ") found";
          throw std::runtime_error(errmsg);
          break;
      }
      return CTransform10p0to11p0::FinalType(1);
    }


    //
    V11::CRingScalerItem
    CTransform10p0to11p0::transformScaler(InitialType& item)
    {
      V10::CRingScalerItem& v10item = dynamic_cast<V10::CRingScalerItem&>(item);
      V11::CRingScalerItem v11item(v10item.getScalerCount());

      v11item.setStartTime(v10item.getStartTime());

      v11item.setEndTime(v10item.getEndTime());

      v11item.setTimestamp(v10item.getTimestamp());

      v11item.setScalers(v10item.getScalers());

      return v11item;
    }

    //
    V11::CRingStateChangeItem
    CTransform10p0to11p0::transformStateChange(InitialType& item)
    {
      V10::CRingStateChangeItem& v10item = dynamic_cast<V10::CRingStateChangeItem&>(item);
      V11::CRingStateChangeItem v11item(v10item.type());

      v11item.setRunNumber(v10item.getRunNumber());

      v11item.setElapsedTime(v10item.getElapsedTime());

      v11item.setTimestamp(v10item.getTimestamp());

      v11item.setOffsetDivisor(1);

      v11item.setTitle(v10item.getTitle());

      return v11item;
    }

    //
    V11::CPhysicsEventItem
    CTransform10p0to11p0::transformPhysicsEvent(InitialType& item)
    {
      V10::CPhysicsEventItem& v10item = dynamic_cast<V10::CPhysicsEventItem&>(item);
      V11::CPhysicsEventItem v11item(v10item.getStorageSize());

      uint8_t* pBegin10  = reinterpret_cast<uint8_t*>(v10item.getBodyPointer());
      uint8_t* pEnd10    = reinterpret_cast<uint8_t*>(v10item.getBodyCursor());
      uint8_t* pCursor11 = reinterpret_cast<uint8_t*>(v11item.getBodyPointer());

      pCursor11 = std::copy(pBegin10, pEnd10, pCursor11);

      v11item.setBodyCursor(pCursor11);
      v11item.updateSize();

      return v11item;
    }

    //
    V11::CRingPhysicsEventCountItem
    CTransform10p0to11p0::transformPhysicsEventCount(InitialType& item)
    {
      V10::CRingPhysicsEventCountItem& v10item
          = dynamic_cast<V10::CRingPhysicsEventCountItem&>(item);

      V11::CRingPhysicsEventCountItem v11item(v10item.getEventCount(),
                                              v10item.getTimeOffset(),
                                              v10item.getTimestamp());

      return v11item;
    }

    //
    V11::CRingTextItem
    CTransform10p0to11p0::transformText(InitialType& item)
    {
      V10::CRingTextItem& v10item
          = dynamic_cast<V10::CRingTextItem&>(item);

      V11::CRingTextItem v11item(v10item.type(),
                                 v10item.getStrings(),
                                 v10item.getTimeOffset(),
                                 v10item.getTimestamp());
      v11item.setTimeDivisor(1);
      return v11item;
    }

    //
    V11::CRingScalerItem
    CTransform10p0to11p0::transformNonIncrScaler(InitialType& item)
    {
      V10::CRingTimestampedRunningScalerItem& v10item
          = dynamic_cast<V10::CRingTimestampedRunningScalerItem&>(item);

      V11::CRingScalerItem v11item(v10item.getOffsetStart(),
                                   v10item.getOffsetEnd(),
                                   v10item.getCalendarTime(),
                                   v10item.getScalers(),
                                   false,
                                   1);
      return v11item;
    }

    //
    V11::CRingFragmentItem
    CTransform10p0to11p0::transformFragment(InitialType& item)
    {
      V10::CRingFragmentItem& v10item
          = dynamic_cast<V10::CRingFragmentItem&>(item);

      V11::CRingFragmentItem v11item(v10item.timestamp(),
                                     v10item.source(),
                                     v10item.payloadSize(),
                                     v10item.payloadPointer(),
                                     v10item.barrierType());

      return v11item;
    }

  } // end Transform
} // end DAQ

