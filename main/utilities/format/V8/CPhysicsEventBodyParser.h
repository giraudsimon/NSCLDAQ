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

#ifndef DAQ_V8_CPHYSICSEVENTBODYPARSER_H
#define DAQ_V8_CPHYSICSEVENTBODYPARSER_H

#include <V8/CPhysicsEventBuffer.h>
#include <ByteBuffer.h>
#include <memory>
#include <vector>
#include <cstdint>

namespace DAQ {
  namespace V8 {


    class CPhysicsEvent;


    /*!
     * \brief Abstract base class for body parsers
     */
    class CPhysicsEventBodyParser
    {
    public:
      virtual std::vector<std::shared_ptr<CPhysicsEvent> >
      operator()(std::size_t nEvents,
                 DAQ::Buffer::BufferPtr<std::uint16_t> beg,
                 DAQ::Buffer::BufferPtr<std::uint16_t> end) = 0;
    };
    
  } // namespace V8
} // namespace DAQ

#endif // DAQ_V8_CPHYSICSEVENTBODYPARSER_H
