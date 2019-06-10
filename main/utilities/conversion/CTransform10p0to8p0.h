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

#ifndef DAQ_TRANSFORM_CTRANSFORM10P0TO8P0_H
#define DAQ_TRANSFORM_CTRANSFORM10P0TO8P0_H

#include <V8/CPhysicsEventBuffer.h>
#include <V8/CTextBuffer.h>
#include <cstdint>


namespace DAQ {

  // Forward declarations
  namespace V8 {
    class CRawBuffer;
    class CControlBuffer;
    class CScalerBuffer;

    extern std::size_t gBufferSize;
  }

  namespace V10 {
    class CRingItem;
  }

  // end fwd declarations


  namespace Transform {

    /*!
     * \brief Transforms data from 10 -> 8
     *
     * All of the logic for converting version 10 CRingItems into
     * version 8 buffers is contained in this class. The operator()
     * method can receive any type of V10::CRingItem and dispatch it
     * to the appropriate transformer. However you can also go directly
     * to the transforms themselves if you know you have the correct type.
     *
     */
    class CTransform10p0to8p0
    {
      using InitialType = V10::CRingItem;
      using FinalType   = V8::CRawBuffer;

    private:
      std::uint32_t m_nTriggersProcessed;
      double        m_samplingFactor;
      std::uint16_t m_run;
      V8::CPhysicsEventBuffer              m_physicsBuffer;
      std::vector<V8::CTextBuffer>         m_textBuffers;

    public:
      CTransform10p0to8p0();

      /*!
       * \brief Transform any type
       *
       * This may return a V8::CRawBuffer with a real piece of data in it or
       * not (i.e. VOID).
       *
       * \return a V8::CRawBuffer
       */
      FinalType operator()(const InitialType& item);

      /*!
       * \brief Transform V10::CRingStateChangeItem to V8::CControlBuffer
       *
       * The rules for this transformation are:
       * The content size will be computed based on the size of the ring item.
          - The types will map accordingly:
            o BEGIN_RUN  -> BEGRUNBF
            o END_RUN    -> ENDRUNBF
            o PAUSE_RUN  -> PAUSEBF
            o RESUME_RUN -> RESUMEBF
          - The checksum will be set to 0.
          - The run number will be used from the ring item and stored to
            produce 8.0 buffers until the next state change item is converted.
          - The sequence number will be computed according to the
            PHYSICS_EVENT_COUNT and the number of previously converted
            PHYSICS_EVENT ring items.
          - The number of events in buffer will be set to 0.
          - The processor number will be set to 0.
          - The number of bit registers will be set to 0.
          - The data format will be set to 5.
          - The short byte order signature and long byte order signature will
            be selected to match the byte ordering of the original ring item.
          - The elements of the state transition item will map to a
            control body in the in the 8k buffer.
              o The first 79 characters of the title will transformed unchanged while the 80th will be
                replaced by a null terminator.
              o Time offset will be used to set time since run start.
              o Timestamp (unix) will be unpacked appropriately into month, day, year, hour, minute,
                and second. The tenths place will be set to 0.
              o Offset divisor will be discarded.

       *
       * \param item - a V10::CRingStateChangeItem
       * \return properly converted V8::CControlBuffer
       */
      V8::CControlBuffer transformStateChange(const InitialType& item);

      /*!
       * \brief Transform normal V10::CRingScaler items
       *
       * Rules for this transformation are:
       *  -	The content size will be computed based on the size of the ring item.
          -	The type will become SCALERBF
          -	The checksum will be set to 0.
          -	The run number will be set to 0 by default. If it can be determined
            by data converted beforehand, the previously extracted run number
            will be used.
          -	The sequence number will be computed according to the
            PHYSICS_EVENT_COUNT and the number of previously converted
            PHYSICS_EVENT ring items.
          -	The number of events in buffer will be set to N, where N is the
            total number of scaler values contained in the buffer (i.e. scaler
            count).
          -	The processor number will be set to 0.
          -	The number of bit registers will be set to 0.
          -	The data format will be set to 5.
          -	The short byte order signature and long byte order signature will be
            set to 0x0102 and 0x01020304 respectively.
          -	The interval start offset will transform to interval offset begin unchanged.
          -	The interval end offset will transform to interval offset end unchanged.
          -	The unix timestamp will be discarded.
          -	The scaler values will convert unchanged.
          -	An error will occur if the size of the ring item body is larger
            than the allotted storage of the 8.0 buffer. The conversion will
            cease and the user will have to start it over using a larger 8.0
            buffer size.

       * \param item - scaler item to transform
       * \return properly converted scaler buffer
       */
      V8::CScalerBuffer transformIncrScaler(const InitialType& item);

      /*!
       * \brief Transform V10::CRingTimestampedRunningScaler -> V8::CScalerBuffer
       *
       * All of the rules for transformIncrScaler apply to this method.
       * Additionally, the following rules also apply:
       *  -	The event timestamp is discarded.
          -	The interval divisor is discarded.

       * \param item - scaler item to convert
       * \return properly converted scaler buffer
       */
      V8::CScalerBuffer transformNonIncrScaler(const InitialType& item);

      /*!
       * \brief Transform V10::CPhysicsEventItem -> V8::CPhysicsEventBuffer
       *
       * Because the version 8 data format uses a buffering scheme for
       * physics event items, there is not necessarily a V8::CPhysicsEventBuffer
       * returned for each input V10::CPhysicsEventItem. The only time a
       * full V8::CPhysicsEventBuffer is returned is if the original buffer
       * causes the V8::CPhysicsEventBuffer to overflow or reach its capacity.
       * If it has not reached capacity, the V8::CRawBuffer returned has a
       * V8::VOID type to indicate that it is empty.
       *
       * The rules for this conversion are:
       *  -	The content size will be computed based on the size of the ring item.
          -	The type will become DATABF
          -	The checksum will be set to 0.
          -	The run number will be set to 0 by default. If it can be determined
            by data converted beforehand, the previously extracted run number
            will be used.
          -	The sequence number will be computed according to the
            PHYSICS_EVENT_COUNT and the number of previously converted
            PHYSICS_EVENT ring items.
          -	The number of events in buffer will be set to N, where N is the
            total number of contiguous PHYSICS_EVENTs contained in the 8k buffer.
          -	The processor number will be set to 0.
          -	The number of bit registers will be set to 0.
          -	The number LAMs will be set to 0.
          -	The data format will be set to 5.
          -	The short byte order signature and long byte order signature will
            be set according to the byte order of the original item.
          -	The data body will be copied in to the next available space of the
            current 8k buffer and the size prepended to it will be computed
            from the size of the original ring item.
          -	An error will occur if the size of the ring item body is larger than
            the allotted storage of the 8.0 buffer. The conversion will cease
            and the use will have to start it over using a larger 8.0 buffer size.

       *
       * \param item - the physics event to transform
       * \return buffer of type VOID or DATABF
       */
      V8::CRawBuffer transformPhysicsEvent(const InitialType& item);

      /*!
       * \brief Transform V10::CRingTextItem -> V8::CTextBuffer
       *
       * This is a unique transform because it is possible that the input
       * V10::CRingTextItem is actually too big to fit into a single
       * buffer. In that case, the contents of the CRingTextItem must be split
       * into multiple V8::CTextBuffers. In that scenario, the first V8::CTextBuffer
       * is returned and the remaining text buffers are left stored in
       * the transform. These are accessible via the getStagedTextBuffers() method.
       *
       * The rules for the transform are:
       *  -	The content size will be computed based on the size of the ring item.
          -	The types will map as follows:
            o	PACKET_TYPES -> PKTDOCBF
            o	MONITORED_VARIABLES -> RUNVARBF
          -	The checksum will be set to 0.
          -	The run number will be set to 0 by default. If it can be determined
            by data converted beforehand, the previously extracted run number
            will be used.
          -	The sequence number will be computed according to the
            PHYSICS_EVENT_COUNT and the number of previously converted
            PHYSICS_EVENT ring items.
          -	The number of events in buffer will be set to N, where N is the
            total number of contiguous null-terminated strings that will fit
            into a single 8k buffer.
          -	The processor number will be set to 0.
          -	The number of bit registers will be set to 0.
          -	The data format will be set to 5.
          -	The short byte order signature and long byte order signature will
            be set to 0x0102 and 0x01020304 respectively.
          -	The null-terminated strings will be copied into a new 8k buffer
            unaltered. They will be null-terminated and a padding byte may be
            added in between them in order to aligned them on even memory
            boundaries. If the total number of strings cannot fit, multiple 8k
            buffers may be sent.
          -	An error will occur if a null-terminated string will not fit into
            the 8.0 format buffer. The conversion process will cease and the
            user will need to initiate the conversion using a larger buffer
            size.

       * \param item - V10::CRingTextItem
       * \return the first V8::CTextBuffer produced
       */
      V8::CRawBuffer transformText(const InitialType& item);

      /*!
       * \brief Read-only access to the V8::CPhysicsEventBuffer being filled
       * \return reference to buffer
       */
      const V8::CPhysicsEventBuffer& getCurrentPhysicsBuffer() const;

      /*!
       * \brief Read-only access to the V8::CTextBuffers remaining to read
       * \return reference to list of buffers
       */
      const std::vector<V8::CTextBuffer>& getStagedTextBuffers() const;


      void clearStagedTextBuffers () { m_textBuffers.clear(); }

      /// Utility methods to get and set various traits
      void setCurrentRunNumber(std::uint16_t runNo) { m_run = runNo; }
      std::uint16_t getCurrentRunNumber() const { return m_run; }

      void setNTriggersProcessed(std::size_t nTriggers) { m_nTriggersProcessed = nTriggers; }

      /*!
       * \brief Compute sequence number to use in a V8::bheader
       *
       * This scales the number of processed physics events (m_nTriggersProcessed) by
       * the sampling factor.
       * \return sequence number
       */
      std::uint32_t computeSequence() const;

      /*!
       * \brief Recompute sampling factor given V10::CRingPhysicsEventCountItem
       *
       * Recomputes the sampling factor based on the event count and the numbers
       * of processed physics events.
       *
       * \param item - a V10::CRingPhysicsEventCountItem
       */
      void updateSamplingFactor(const InitialType& item);

      /*!
       * \brief Clear m_nTriggersProcessed and set sample factor to 1.0
       */
      void resetStatistics();

      /*!
       * \brief Replaces the current V10::CPhysicsEventBuffer with a new one
       */
      void startNewPhysicsBuffer();

    private:
      /*!
       * \brief Analogous to startNewPhysicsBuffer() but for text buffers
       * In this case, it does not replace the exist list but rather just
       * appends the new buffer to the list.
       *
       * \param type - type of buffer
       */
      void appendNewTextBuffer(std::uint16_t type);

      /*!
       * \brief Mapping between control types
       *
       * BEGIN_RUN  --> BEGRUNBF
       * END_RUN    --> ENDRUNBF
       * PAUSE_RUN  --> PAUSEBF
       * RESUME_RUN --> RESUMEBF
       *
       * \param type V10 type
       * \return V8 type
       */
      std::uint16_t mapControlType(std::uint16_t type) const;

      /*!
       * \brief Mapping between text types
       *
       * V10::MONITORED_VARIABLES --> V8::RUNVARBF;
       * V10::PACKET_TYPES        --> V8::PKTDOCBF;
       * \param type - V10 type
       * \return V8 type
       */
      std::uint16_t mapTextType(std::uint16_t type) const;
    };
    
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_CTRANSFORM10P0TO8P0_H
