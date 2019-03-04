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

#ifndef CTRANSFORM11P0TO10P0_H
#define CTRANSFORM11P0TO10P0_H



namespace DAQ {

  // Fwd declarations
  namespace V10
  {
      class CRingItem;
      class CRingScalerItem;
      class CRingTimestampedRunningScalerItem;
      class CRingStateChangeItem;
      class CPhysicsEventItem;
      class CRingPhysicsEventCountItem;
      class CRingTextItem;
      class CRingFragmentItem;
      class CUnknownFragment;
  }

  namespace V11 {
      class CRingItem;
  }
  /////////////////////////////////////////////////////////////////////////////

  namespace Transform {

    /*!
     * \brief Transforms version 11 data to version 10 data.
     *
     * This is a fairly straightforward conversion because there is a one-to-one
     * or 1-to-0 conversion logic. Most of the version 11 items have a corresponding
     * version 10 type to convert to, and the ones that do not simply do not convert
     * to anything.
     */
    class CTransform11p0to10p0
    {
    public:
      typedef typename V11::CRingItem InitialType;
      typedef typename V10::CRingItem FinalType;

    public:
      /*!
       * \brief General purpose converter
       *
       * This is the main entry point for converting any type of V11 item to
       * a V10 item. The function dispatches to the correct transformer based on
       * the type of the type of the initial V11 item. It then returns a generic
       * ring item.
       * \param item - any V11 type
       * \return the transformed v10 ring item
       */
      FinalType operator()(InitialType& item);

      /*!
       * \brief Dispatches the ring item to the appropriate converter
       *
       * This dispatches an item to the specific transform function. In order for
       * the transform function to succeed without throwing a std::bad_cast, the
       * ring item passed in must have a static type that corresponds to one of the
       * derived classes from CRingItem. In other words, it is a must that the
       * an upcast using dynamic_cast<> will succeed.
       *
       * \param item - any v11 type
       * \return the converted ring item
       */
      FinalType dispatch(InitialType& item);

      /*!
       * \brief Transform V11::CRingScalerItem --> V10::CRingScalerItem
       *
       * This dispatches to either transformIncrScaler() or transformNonIncrScaler()
       *
       * \param item - a v11 item of type V11::PERIODIC_SCALERS
       * \return transformed V10 scaler type item
       */
      FinalType transformScaler(InitialType& item);

      /*!
       * \brief Transforms incremental scalers to V10::INCREMENTAL_SCALERS
       *
       * The rules for this transform are:
       *  -	Size in ring item header will be recomputed.
          -	Type will map as PERIODIC_SCALERS INCREMENTAL_SCALER
          -	The body header will be discarded if present.
          -	The interval start offset will transform unchanged.
          -	The interval end offset will transform unchanged.
          -	The timestamp (unix) will transform unchanged.
          -	The interval divisor will be discarded.
          -	The scaler count will transform unchanged.
          -	The is incremental value will be discarded.
          -	The scaler values will transform unchanged.
       *
       * \param item - the scaler item to be transformed.
       * \return the transformed scaler item (type=V10::INCREMENTAL_SCALERS)
       *
       * \throws std::bad_cast if parameter cannot upcast to V11::CRingScalerItem
       */
      V10::CRingScalerItem transformIncrScaler(InitialType& item);

      /*!
       * \brief Transform non incremental scalers -> V10::TIMESTAMPED_NONINCR_SCALERS
       *
       *  The rules for this transform are:
       *  -	Size in ring item header will be recomputed.
          -	Type will map as PERIODIC_SCALERS  TIMESTAMPED_NONINCR_SCALER
          -	If a body header is present, only the timestamp will be salvaged and
            set to the event timestamp. All other body header information
            will be discarded.
          -	The interval start offset will transform unchanged.
          -	The interval end offset will transform unchanged.
          -	The timestamp (unix) will be set to the clock timestamp
          -	The interval divisor will transform unchanged.
          -	The scaler count will transform unchanged.
          -	The is incremental value will be discarded.
          -	The scaler values will transform unchanged.
       *
       * \param item - a V11 item of type V11::PERIODIC_SCALERS
       * \return transformed scaler item (type=V10::TIMESTAMPED_NONINCR_SCALERS)
       *
       * \throws std::bad_cast if parameter cannot upcast to V11::CRingScalerItem
       */
      V10::CRingTimestampedRunningScalerItem
              transformNonIncrScaler(InitialType& item);

      /*!
       * \brief Transform V11::CRingStateChangeItem --> V10::CRingStateChangeItem
       *
       * The rules for this transform are:
       *  - Size in ring item header will be recomputed.
          - Types will transform as:
            o BEGIN_RUN   -> BEGIN_RUN
            o END_RUN     -> END_RUN
            o PAUSE_RUN   -> PAUSE_RUN
            o RESUME_RUN  -> RESUME_RUN
          - The body header will be discarded if it exists.
          - Run number will remain the same.
          - Time offset will remain the same.
          - Timestamp (unix) will remain the same.
          - Offset divisor will be discarded.
          - Title will remain the same.
       *
       * \param item a v11 state change item
       * \return the transformed item
       *
       * \throws std::bad_cast if cannot upcast parameter to V11::CRingStateChangeItem
       */
      V10::CRingStateChangeItem transformStateChange(InitialType& item);

      /*!
       * \brief transformPhysicsEvent
       *
       * The rules for this transform are:
       *  - Size in ring item header will be recomputed.
          - Type will map as PHYSICS_EVENT -> PHYSICS_EVENT
          - Body header will be discarded if present.
          - All data in the body will transform unchanged.

       * \param item a v11 physics event item
       * \return  the transformed item
       */
      V10::CPhysicsEventItem transformPhysicsEvent(InitialType& item);

      /*!
       * \brief Transform a physics event count item
       *
       * The rules for this transform are:
       *  - Size in ring item header will be recomputed.
          - Type will map as PHYSICS_EVENT_COUNT -> PHYSICS_EVENT_COUNT
          - Body header will be discarded if present.
          - Time offset will transform unchanged.
          - Offset divisor will be discarded.
          - Timestamp (Unix) will transform unchanged.
          - Event count will transform unchanged.

       * \param item - a v11 physics event count item
       * \return transformed item
       */
      V10::CRingPhysicsEventCountItem transformPhysicsEventCount(InitialType& item);

      /*!
       * \brief Transform a fragment
       *
       * The rules for this are:
       *  -	Size in ring item header will be recomputed
          -	Type will map as EVB_FRAGMENT -> EVB_FRAGMENT
          -	Body header timestamp will become timestamp in body
          -	Body header source id  will becomes source id in body
          -	Body header barrier type will become barrier type in body
          -	Payload size will be computed based on size of ring item.
          -	Payload will be transformed unchanged.

       * \param item a v11 fragment item
       * \return the transformed item
       */
      V10::CRingFragmentItem transformFragment(InitialType& item);

      /*!
       * \brief Transform unknown payload fragment
       *
       * The rules are the same as for transformFragment
       * \param item - a v11 fragment item
       * \return the transformed fragment
       */
      V10::CUnknownFragment transformUnknownFragment(InitialType& item);

      /*!
       * \brief Transform text item
       *
       * The rules for this are:
       *  -	Size in ring item header will be recomputed.
          -	Type will map as:
            o	PACKET_TYPES -> PACKET_TYPES
            o	MONITORED_VARIABLES -> MONITORED_VARIABLES
          -	Any body header will be discarded if present.
          -	Time offset will transform unchanged.
          -	Timestamp (Unix) will transform unchanged.
          -	String count will transform unchanged.
          -	Strings will transform unchanged.

       * \param item - v11 text item
       * \return transformed item
       */
      V10::CRingTextItem transformText(InitialType& item);

    };

  } // end Transform
} // end DAQ

#endif // CTRANSFORM11P0TO10P0_H
