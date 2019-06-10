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


#include "CGenericBodyParser.h"
#include <iterator>
#include <stdexcept>
#include <string>

using namespace  std;

namespace DAQ {
  namespace V8 {

    ///////////////////////////////////////////////////////////////////////////

    vector<shared_ptr<CPhysicsEvent> >
    CGenericBodyParser::operator()(size_t nEvents,
                                    Buffer::BufferPtr<uint16_t> beg,
                                    Buffer::BufferPtr<uint16_t> end)
    {
      vector<shared_ptr<CPhysicsEvent> > events;

      for (size_t index=0; (beg!=end) && (index<nEvents); ++index) {
        auto result = parseOne(beg, end);

        events.push_back( get<0>(result) );
        beg = get<1>(result);
      }

      return events;
    }


    //
    pair<shared_ptr<CPhysicsEvent>, Buffer::BufferPtr<uint16_t> >
    CGenericBodyParser::parseOne(Buffer::BufferPtr<uint16_t> beg,
                                 Buffer::BufferPtr<uint16_t> max)
    {
      // Ensure that we don't read in meaningless data...
      if (beg >= max) {
        string errmsg("DAQ::V8::CGenericBodyParser::parseOne() ");
        errmsg += "Begin iterator is invalid";
        throw runtime_error(errmsg);
      }

      size_t nWords = computeSize(beg, max);

      auto itEnd = beg + nWords;

      // Ensure that we don't read in meaningless data...
      if (itEnd > max) {
        string errmsg("DAQ::V8::CGenericBodyParser::parseOne() ");
        errmsg += "Size of buffer states more data exists than is present";
        throw runtime_error(errmsg);
      }

      if (itEnd == beg) { // size is 0 can cause infinite loops
        string errmsg("DAQ::V8::CGenericBodyParser::parseOne() ");
        errmsg += "Zero buffer size is invalid.";
        throw runtime_error(errmsg);
      }

      // we need to copy in the chunk of the body
      // but because the byte buffer deals with bytes, we need to use
      // the fundamental iterators that the translatorptrs use so that we
      // don't incorrectly skip
      Buffer::ByteBuffer buffer (beg.getBaseIterator(),
                                 itEnd.getBaseIterator());

      shared_ptr<CPhysicsEvent> pEvent (
            new CPhysicsEvent(move(buffer),
                              beg.getSwapper().isSwappingBytes())
            );


      return make_pair( pEvent, itEnd);

    }

    //
    std::size_t CGenericBodyParser::computeSize(Buffer::BufferPtr<uint16_t> beg,
                                                Buffer::BufferPtr<uint16_t> deadend) const
    {
      std::size_t result = 0;

      switch (m_sizeType) {
        case Inclusive32BitBytes:
          result = computeIncl32BitByteSize(beg, deadend);
          break;
        case Inclusive32BitWords:
          result = computeIncl32BitWordSize(beg, deadend);
          break;
        case Exclusive16BitWords:
          result = computeExcl16BitWordSize(beg, deadend);
          break;
        case Inclusive16BitWords:
          result = computeIncl16BitWordSize(beg, deadend);
          break;
        default:
          throw runtime_error(
                "DAQ::V8::CGenericBodyParser::computeSize() invalid SizeType"
                );
      }

      return result;
    }


    //
    std::size_t
    CGenericBodyParser::computeIncl32BitByteSize(Buffer::BufferPtr<uint16_t> beg,
                                                 Buffer::BufferPtr<uint16_t> deadend) const
    {
      auto result = compute32BitSize(beg, deadend);

      if ((result%2)==1) {
        string errmsg("DAQ::V8::CGenericBodyParser::computeIncl32BitByteSize() ");
        errmsg += "Odd number of bytes found. Only parsing of an even number of bytes supported.";
        throw runtime_error(errmsg);
      }

      result /= sizeof(uint16_t);
      return result;
    }


    //
    std::size_t
    CGenericBodyParser::computeIncl32BitWordSize(Buffer::BufferPtr<uint16_t> beg,
                                                 Buffer::BufferPtr<uint16_t> deadend) const
    {
      return compute32BitSize(beg, deadend);
    }


    //
    std::size_t
    CGenericBodyParser::computeIncl16BitWordSize(Buffer::BufferPtr<uint16_t> beg,
                                                 Buffer::BufferPtr<uint16_t> deadend) const
    {
      return compute16BitSize(beg, deadend);
    }


    //
    std::size_t
    CGenericBodyParser::computeExcl16BitWordSize(Buffer::BufferPtr<uint16_t> beg,
                                                 Buffer::BufferPtr<uint16_t> deadend) const
    {
      auto result = compute16BitSize(beg, deadend);
      result += 1; // correct for exclusive count

      return result;
    }


    //
    std::size_t
    CGenericBodyParser::compute32BitSize(Buffer::BufferPtr<uint16_t> beg,
                                         Buffer::BufferPtr<uint16_t> deadend) const
    {
      size_t size = 0;
      if ( distance(beg, deadend) >= sizeof(uint32_t)/sizeof(uint16_t) ) {
        Buffer::BufferPtr<uint32_t> p32(beg);
        size = *p32;
      } else {
        string errmsg("DAQ::V8::CGenericBodyParser::compute32BitSize() ");
        errmsg += "Incomplete 32-bit integer for size provided";
        throw runtime_error(errmsg);
      }

      return size;
    }


    //
    std::size_t
    CGenericBodyParser::compute16BitSize(Buffer::BufferPtr<uint16_t> beg,
                                         Buffer::BufferPtr<uint16_t> deadend) const
    {
      size_t size = 0;
      if ( distance(beg, deadend) >= 1) {
        size = *beg;
      } else {
        string errmsg("DAQ::V8::CGenericBodyParser::compute16BitSize() ");
        errmsg += "Incomplete 16-bit integer for size provided";
        throw runtime_error(errmsg);
      }

      return size;
    }

  } // namespace V8
} // namespace DAQ
