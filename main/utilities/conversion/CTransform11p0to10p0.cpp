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
#include "CTransform11p0to10p0.h"

#include <V11/CRingItemFactory.h>
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
#include <V10/CUnknownFragment.h>
#include <V11/CUnknownFragment.h>
#include <V10/CVoidItem.h>
#include <V10/DataFormatV10.h>
#include <V11/DataFormatV11.h>

#include <stdexcept>
#include <memory>
#include <string>

using namespace std;

namespace DAQ {
  namespace Transform {


    //
    CTransform11p0to10p0::FinalType
    CTransform11p0to10p0::operator()(InitialType& item) {
      V11::CRingItemFactory factory;
      unique_ptr<V11::CRingItem> pItem(factory.createRingItem(item));
      return dispatch(*pItem);
    }

    //
    CTransform11p0to10p0::FinalType
    CTransform11p0to10p0::dispatch(InitialType& item)
    {

      switch (item.type()) {
        case V11::PERIODIC_SCALERS:
          return transformScaler(item);
          break;
        case V11::BEGIN_RUN:
        case V11::END_RUN:
        case V11::PAUSE_RUN:
        case V11::RESUME_RUN:
          return transformStateChange(item);
          break;
        case V11::PHYSICS_EVENT:
          return transformPhysicsEvent(item);
          break;
        case V11::PHYSICS_EVENT_COUNT:
          return transformPhysicsEventCount(item);
          break;
        case V11::MONITORED_VARIABLES:
        case V11::PACKET_TYPES:
          return transformText(item);
          break;
        case V11::EVB_FRAGMENT:
          return transformFragment(item);
          break;
        case V11::EVB_UNKNOWN_PAYLOAD:
          return transformUnknownFragment(item);
          break;
        default:
          return V10::CVoidItem();
          break;
      }
      return CTransform11p0to10p0::FinalType(1);

    }

    //
    CTransform11p0to10p0::FinalType
    CTransform11p0to10p0::transformScaler(V11::CRingItem& item)
    {
      auto& sclrItem = dynamic_cast<V11::CRingScalerItem&>(item);
      if (sclrItem.isIncremental()) {
        return transformIncrScaler(item);
      } else {
        return transformNonIncrScaler(item);
      }

    }

    //
    V10::CRingScalerItem
    CTransform11p0to10p0::transformIncrScaler(InitialType &item)
    {
      auto& sclrItem = dynamic_cast<V11::CRingScalerItem&>(item);

      V10::CRingScalerItem v10item(sclrItem.getScalerCount());

      v10item.setStartTime(sclrItem.getStartTime());
      v10item.setEndTime(sclrItem.getEndTime());
      v10item.setTimestamp(sclrItem.getTimestamp());
      v10item.setScalers(sclrItem.getScalers());

      return v10item;

    }

    //
    V10::CRingTimestampedRunningScalerItem
    CTransform11p0to10p0::transformNonIncrScaler(InitialType &item)
    {
      auto& sclrItem = dynamic_cast<V11::CRingScalerItem&>(item);

      V10::CRingTimestampedRunningScalerItem v10item(
            sclrItem.getEventTimestamp(),
            sclrItem.getStartTime(),
            sclrItem.getEndTime(),
            sclrItem.getTimeDivisor(),
            sclrItem.getTimestamp(),
            sclrItem.getScalers()
            );

      return v10item;
    }

    //
    V10::CRingStateChangeItem
    CTransform11p0to10p0::transformStateChange(InitialType &item)
    {
      auto& v11Item = dynamic_cast<V11::CRingStateChangeItem&>(item);

      V10::CRingStateChangeItem v10item(v11Item.type(),
                                        v11Item.getRunNumber(),
                                        v11Item.getElapsedTime(),
                                        v11Item.getTimestamp(),
                                        v11Item.getTitle());
      return v10item;
    }

    //
    V10::CPhysicsEventItem
    CTransform11p0to10p0::transformPhysicsEvent(InitialType& item)
    {
      auto& v11Item = dynamic_cast<V11::CPhysicsEventItem&>(item);
      V10::CPhysicsEventItem v10item(V11::PHYSICS_EVENT,
                                     v11Item.getStorageSize());

      auto pBegin11 = reinterpret_cast<char*>(v11Item.getBodyPointer());
      auto pEnd11 = reinterpret_cast<char*>(v11Item.getBodyCursor());

      auto pBegin10 = reinterpret_cast<char*>(v10item.getBodyPointer());
      auto pEnd10 =  std::copy(pBegin11, pEnd11, pBegin10);
      v10item.setBodyCursor(pEnd10);
      v10item.updateSize();

      return v10item;
    }

    //
    V10::CRingPhysicsEventCountItem
    CTransform11p0to10p0::transformPhysicsEventCount(InitialType& item)
    {
      auto& v11Item = dynamic_cast<V11::CRingPhysicsEventCountItem&>(item);
      V10::CRingPhysicsEventCountItem v10item(v11Item.getEventCount(),
                                              v11Item.getTimeOffset(),
                                              v11Item.getTimestamp());

      return v10item;
    }

    //
    V10::CRingFragmentItem
    CTransform11p0to10p0::transformFragment(InitialType& item)
    {
      auto& v11Item = dynamic_cast<V11::CRingFragmentItem&>(item);
      V10::CRingFragmentItem v10item(v11Item.timestamp(),
                                     v11Item.source(),
                                     v11Item.payloadSize(),
                                     v11Item.payloadPointer(),
                                     v11Item.barrierType());

      return v10item;

    }

    //
    V10::CUnknownFragment
    CTransform11p0to10p0::transformUnknownFragment(InitialType& item)
    {
      auto& v11Item = dynamic_cast<V11::CUnknownFragment&>(item);
      V10::CUnknownFragment v10item(v11Item.timestamp(),
                                    v11Item.source(),
                                    v11Item.barrierType(),
                                    v11Item.payloadSize(),
                                    v11Item.payloadPointer());

      return v10item;

    }

    //
    V10::CRingTextItem
    CTransform11p0to10p0::transformText(InitialType& item)
    {
      auto& v11Item = dynamic_cast<V11::CRingTextItem&>(item);
      V10::CRingTextItem v10item(v11Item.type(),
                                 v11Item.getStrings(),
                                 v11Item.getTimeOffset(),
                                 v11Item.getTimestamp());

      return v10item;
    }

  } // end Transform
} // end DAQ

