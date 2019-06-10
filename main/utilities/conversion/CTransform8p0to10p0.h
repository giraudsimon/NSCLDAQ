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

#ifndef DAQ_TRANSFORM_CTRANSFORM8P0TO10P0_H
#define DAQ_TRANSFORM_CTRANSFORM8P0TO10P0_H

#include <ByteBuffer.h>
#include <V10/CPhysicsEventItem.h>

#include <cstdint>
#include <vector>
#include <memory>
#include <ctime>


namespace DAQ {

  // forward declarations
  namespace V8 {
    struct bftime;
    class CRawBuffer;
    class ScalerBuffer;
    class CPhysicsEvent;

    extern std::size_t gBufferSize;
  }

  namespace V10 {
    class CRingItem;
    class CRingScalerItem;
    class CRingTextItem;
    class CRingStateChangeItem;
  }

  ////// end fwd declarations

  namespace Transform {
    
    /*!
     * \brief Transforms version 8 buffers into version 10 ring items.
     *
     * These transforms are non-trivial becuase the the transform may be
     * a 1-to-N mapping instead of 1-to-1. For this reason, the transform
     * maintains a list of physics events that may need to be read out. The list
     * may be empty, but after a V8::DATABF is processed, it will likely contain
     * many elements.
     *
     * THe rules for the mappings are made explicit in the documentation of
     * each transform type.
     *
     */
    class CTransform8p0to10p0
    {
    private:
      std::vector<V10::CPhysicsEventItem> m_physicsEvents;

    public:
      typedef typename DAQ::V8::CRawBuffer InitialType;
      typedef typename V10::CRingItem FinalType;

    public:
      CTransform8p0to10p0() {}

      /*!
       * \brief Dispatch and transform any valid V8 item
       *
       * \param type a V8 buffer
       * \return the transformed V10 buffer
       */
      FinalType operator()(const InitialType& type);

      /*!
       * \brief Transform a scaler buffer into a scaler ring item
       *
       * This is a one-to-one transformation. THe rules are:
       *
       *  - Size of the ring item will be computed
          - Type will map as SCALERBFINCREMENTAL_SCALERS (Note that this is
            not guaranteed to be accurate for the type. It could have been
            better suited for TIMESTAMPED_NONINCR_SCALERS, but there is no
            information to make this decision.)
          - Interval offset end value used for interval end offset.
          - Interval offset begin value used for interval start offset.
          - Timestamp is filled in using the value returned by the std::time(std::time_t*)
            function in C++ at the time of conversion. (Note that this has no meaning
            to the data but serves to produce a functional value).
          - The number of events in buffer header is used to set scaler count.
          - Scaler data transforms unchanged (both formats store value in 32-bit integers).

       *
       * \param item  a scaler buffer
       * \return  the transformed item
       */
      V10::CRingScalerItem transformScaler(const InitialType& item);

      /*!
       * \brief Transform a control buffer to a state change item
       *
       * This is a one-to-one transformation. The rules are:
       *Size of the ring item will be computed based on size of body.
          - The type of 8k buffer will be translated to a ring item type as
            follows:
            o BEGRUNBF -> BEGIN_RUN
            o ENDRUNBF -> END_RUN
            o PAUSEBF -> PAUSE_RUN
            o RESUMEBF -> RESUME_RUN
          - Run number from buffer header will be used to set the run number
            in the new ring item.
          - Time since start of run will transform to the time offset.
          - Time values for year, month, day, hour, min, second, and tenth of
            second are translated into unix time and used as the timestamp. The tenths
            of second are discarded during this conversion.
          - Title will transform unchanged and will be null-terminated.

       * \param item a v8 control buffer
       * \return the transformed item
       */
      V10::CRingStateChangeItem transformControl(const InitialType& item);

      /*!
       * \brief Transform physics event buffers into physics events
       *
       * In general, this is a 1-to-N transform. This returns the first of the
       * the first of the physics event items produced, but then the remainder of the
       * physics events are stored for subsequent access. These physics events can
       * be obtained using the getRemainingEvents() method.
       *
       * The rules for this transform are:
          - Type will map as DATABF  PHYSICS_EVENT
          - Data buffers (DATABF) actually contain multiple physics event bodies
            within them. The original DATABF buffer will therefore be split into
            multiple ring items of type PHYSICS_EVENT. The size in each ring item
            produced will be computed according to the inclusive body header in the
            original physics event body. The total number of ring items produced
            will be the same as number of events in the 8k buffer header.
       *
       * \param item a data buffer
       * \return the first transformed physics event ring item
       */
      V10::CPhysicsEventItem transformPhysicsEvent(const InitialType &item);


      /*!
       * \brief Transform a text buffer into a text ring item
       *
       * This is a 1-to-1 transformation. The rules are:
       *  - Size of the ring item will be computed based on the number of strings
            and their collective length.
          - The data types map as:
              o STATEVARBF -> MONITORED_VARIABLES
              o RUNVARBF -> MONITORED_VARIABLES
              o PKTDOCBF -> PACKET_TYPES
          - Time offset will be set to 0.
          - Timestamp (Unix) will be set to the value returned by the C++
            std::time(std::time_t*) at time of conversion.
          - The number of events becomes the string count value
          - String content will remain the same, though an uninitialized padding
            byte following a null terminator may be added between strings. The
            padding byte serves the purpose of aligning the start of every string
            to an even address.
       *
       * \param item - a v8 text buffer
       * \return  the transformed item
       */
      V10::CRingTextItem transformText(const InitialType& item);

      /*!
       * \brief Access the physics events that were not returned from the last transformPhysicsEvent
       * \return reference to events
       */
      std::vector<V10::CPhysicsEventItem>& getRemainingEvents()
      {  return m_physicsEvents; }

      /*!
       * \brief Utility method to transform V8::CPhysicsEvent in V10::CPhysicsEventItem
       *
       * There is a 1-to-1 relationship between the V8::CPhysicsEvent and the
       * V10::CPhysicsEventItem. This implements the transformation logic for
       * transformPhysicsEvent(). See its documentation for the rules.
       *
       * \param pEvent - a V8::CPhysicsEvent
       */
      void transformOnePhysicsEvent(const std::shared_ptr<V8::CPhysicsEvent>& pEvent);

      /*!
       * \brief Converts a V8::bftime struct to a time_t
       *
       * \param tstruct the structure to convert
       * \return  the converted valued
       */
      std::time_t convertToTime_t(const V8::bftime& tstruct) const;

      /*!
       * \brief Properly maps V8 control types to v10 types
       * \param type the type to convert
       * \return the converted type
       */
      std::uint16_t mapControlType(std::uint16_t type) const;

    };
    
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_CTRANSFORM8P0TO10P0_H
