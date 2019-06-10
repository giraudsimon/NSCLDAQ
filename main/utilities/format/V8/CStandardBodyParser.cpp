#include "V8/CStandardBodyParser.h"

using namespace std;

namespace DAQ {
  namespace V8 {
    
    vector<shared_ptr<CPhysicsEvent> >
    CStandardBodyParser::operator()(std::size_t nEvents,
                                    Buffer::BufferPtr<uint16_t> beg,
                                    Buffer::BufferPtr<uint16_t> end)
    {
      vector<shared_ptr<CPhysicsEvent> > events;

      for (std::size_t index=0; (beg!=end) && (index<nEvents); ++index) {
        auto result = parseOne(beg, end);

        events.push_back( std::get<0>(result) );
        beg = std::get<1>(result);
      }

      return events;
    }

    pair<shared_ptr<CPhysicsEvent>, Buffer::BufferPtr<uint16_t> >
    CStandardBodyParser::parseOne(Buffer::BufferPtr<uint16_t> beg,
                                  Buffer::BufferPtr<uint16_t> max)
    {
      if (beg >= max) {
        std::string errmsg("DAQ::V8::CStandardBodyParser::parseOne() ");
        errmsg += "Iterator provided is out of bounds";
        throw std::runtime_error(errmsg);
      }

      size_t size = *beg;

      auto itEnd = beg + size;

      // Ensure that we don't read in meaningless data...
      if (itEnd > max) {
        std::string errmsg("DAQ::V8::CStandardBodyParser::parseOne() ");
        errmsg += "Size of buffer states more data exists than is present";
        throw std::runtime_error(errmsg);
      }

      if (itEnd == beg) { // size is 0 can cause infinite loops
        std::string errmsg("DAQ::V8::CStandardBodyParser::parseOne() ");
        errmsg += "Zero buffer size is invalid.";
        throw std::runtime_error(errmsg);
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


      return std::make_pair( pEvent, itEnd);

    }
    
  } // namespace V8
} // namespace DAQ
