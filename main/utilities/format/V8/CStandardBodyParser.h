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

#ifndef DAQ_V8_CSTANDARDBODYPARSER_H
#define DAQ_V8_CSTANDARDBODYPARSER_H

#include <ByteBuffer.h>
#include <BufferPtr.h>
#include <V8/CPhysicsEventBodyParser.h>

#include <memory>
#include <vector>
#include <utility>
#include <cstdint>

namespace DAQ {
  namespace V8 {

    /*!
     * \brief The CStandardBodyParser class
     *
     *  Parses the body of a physics event buffer  into physics events assuming
     *  an 16-bit wide, inclusive count of 16-bit words in the body. This is actually
     *  the parser for the body when the header.buffmt = 5 (StandardVsn). I
     *  consider removing this though because
     */
    class CStandardBodyParser : public CPhysicsEventBodyParser
    {
    public:
      std::vector<std::shared_ptr<CPhysicsEvent> >
      operator()(std::size_t nEvents,
                 Buffer::BufferPtr<std::uint16_t> pos,
                 Buffer::BufferPtr<std::uint16_t> end);

       std::pair<std::shared_ptr<CPhysicsEvent>, Buffer::BufferPtr<std::uint16_t> >
        parseOne(Buffer::BufferPtr<std::uint16_t> beg,
                 Buffer::BufferPtr<std::uint16_t> deadend);
    };
    
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CSTANDARDBODYPARSER_H
