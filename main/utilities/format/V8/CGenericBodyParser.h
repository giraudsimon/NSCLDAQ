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

#ifndef DAQ_V8_CGENERICBODYPARSER_H
#define DAQ_V8_CGENERICBODYPARSER_H

#include <ByteBuffer.h>
#include <BufferPtr.h>
#include <V8/CPhysicsEventBodyParser.h>
#include <V8/DataFormatV8.h>

#include <memory>
#include <cstdint>
#include <utility>
#include <vector>

namespace DAQ {
  namespace V8 {

    // Fwd declaration
    class CPhysicsEvent;

    /*!
     * \brief Parses V8::CPhysicsEventBuffer body into V8::CPhysicsEvent objects
     *
     * This is a more generic form of V8::CStandardBodyParser. It provides 4 options
     * for interpreting the size of the events in a V8::CPhysicsEventBuffer body. The
     * default and well-defined behavior for a standard V8 buffer is for the first
     * 16-bit word of the event to be interpretable as an inclusive word count. The
     * problem is that data converted from 10.x to 8.x format does not necessarily have
     * this same paradigm and the rules of conversion mandate that one cannot interpret
     * the body of the data. It is kept invariant. This class aims to solve this problem
     * and provide a one-stop shop for parsing up a physics event buffer (i.e. DATABF)
     * into the events that make it up. It can handle data whose size is:
     *
     * 1. A 16-bit wide, inclusive count of 16-bit data words
     * 2. A 32-bit wide, inclusive count of 16-bit data words
     * 3. A 32-bit wide, inclusive count of bytes
     * 4. A 16-bit wide, exclusive count of 16-bit data words
     *
     * The V8::CPhysicsEventBuffer uses an instance of this class to parse the data
     * if it is not set to use the preference of the buffer header.
     *
     */
    class CGenericBodyParser : public CPhysicsEventBodyParser
    {
    public:

    private:
      PhysicsEventSizePolicy m_sizeType;

    public:
      /*!
       * \brief Constructs a specific type of parser
       *
       * \param sizeType - the size type
       */
      CGenericBodyParser(PhysicsEventSizePolicy sizeType)
        : m_sizeType(sizeType) {}

      /*!
       * \brief Parse multiple buffers within a range of data
       *
       * This will parse between 0 and nEvents physics events from the range of
       * data provided. It is the callers responsibility to ensure that the
       * pos and end iterators refer to the same iterable set of data.The parsing will
       * stop when either the entire range has been parsed or the number of events
       * has been found.
       *
       * Ultimately, this is a just loop that calls parseOne()
       *
       * \param nEvents  max number of events to extract
       * \param pos      start of data to parse
       * \param end      off-the-end iterator
       *
       * \return list of extract events
       *
       * \throws see parseOne() for the possible errors
       */
      std::vector<std::shared_ptr<CPhysicsEvent> >
      operator()(std::size_t nEvents,
                 Buffer::BufferPtr<std::uint16_t> pos,
                 Buffer::BufferPtr<std::uint16_t> end);

      /*!
        * \brief Parse a single event from the data range
        *
        * This will parse one and only one event from a data range provided. It uses
        * the size policy (aka size type) to traverse the appropriate amount of data.
        *
        * \param beg      iterator referring to beginning of data range
        * \param deadend  off-the-end iterator marking the end of the data range
        * \return pair of values:
        *         pair.first  = the extracted physics event
        *         pair.second = iterator marking position just after extracted event
        *
        * \throws std::runtime_error if:
        *         - size of buffer is computed to be 0
        *         - invalid range (beg > deadend)
        *         - incomplete data present (beg + size > deadend)
        */
       std::pair<std::shared_ptr<CPhysicsEvent>, Buffer::BufferPtr<std::uint16_t> >
        parseOne(Buffer::BufferPtr<std::uint16_t> beg,
                 Buffer::BufferPtr<std::uint16_t> deadend);

       // Utility methods for computing size

      private:

       /*!
        * \brief Dispatches to appropriate size computation
        *
        * \param beg      data to begin interpreting
        * \param deadend  off-the-end iterator indicating location where data is no longer valid
        *
        * \return size of physics event in units of 16-bit words
        *
        * \throws std::runtime_error if insufficient data exists to compute size
        */
       std::size_t computeSize(Buffer::BufferPtr<std::uint16_t> beg,
                               Buffer::BufferPtr<std::uint16_t> deadend) const;

       ///////////// Interpreters of the size returned by compute16BitSize() and
       ///////////// compute32BitSize()
       std::size_t computeIncl32BitByteSize(Buffer::BufferPtr<std::uint16_t> beg,
                                            Buffer::BufferPtr<std::uint16_t> deadend) const;

       std::size_t computeIncl32BitWordSize(Buffer::BufferPtr<std::uint16_t> beg,
                                            Buffer::BufferPtr<std::uint16_t> deadend) const;

       std::size_t computeIncl16BitWordSize(Buffer::BufferPtr<std::uint16_t> beg,
                                            Buffer::BufferPtr<std::uint16_t> deadend) const;

       std::size_t computeExcl16BitWordSize(Buffer::BufferPtr<std::uint16_t> beg,
                                            Buffer::BufferPtr<std::uint16_t> deadend) const;

       ///////////// Extractors of the size
       //
       // Both of these throw std::runtime_error if insufficient data exists to
       // properly extract the data.

       std::size_t compute32BitSize(Buffer::BufferPtr<std::uint16_t> beg,
                                    Buffer::BufferPtr<std::uint16_t> deadend) const;

       std::size_t compute16BitSize(Buffer::BufferPtr<std::uint16_t> beg,
                                    Buffer::BufferPtr<std::uint16_t> deadend) const;

    }; // end class
    
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CGENERICBODYPARSER_H
