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

#include "C10p0to8p0Mediator.h"
#include <V8/DataFormatV8.h>
#include <V10/DataFormatV10.h>
#include <V8/format_cast.h>
#include <V10/CRingItem.h>
#include <RingIOV10.h>
#include <BufferIOV8.h>

namespace DAQ {
  namespace Transform {
    
    ///////////////////////////////////////////////////////////////////////////
    // C10p08p0MediatorCreator Implementation
    //

    CBaseMediator* C10p0to8p0MediatorCreator::operator ()(void* unused)  {
      return (new C10p0to8p0Mediator());
    }


    ///////////////////////////////////////////////////////////////////////////
    // C10p08p0Mediator Implementation
    //

    //
    C10p0to8p0Mediator::C10p0to8p0Mediator(std::unique_ptr<CDataSource> source,
                                           std::unique_ptr<CDataSink> sink)
      : CBaseMediator(move(source), move(sink)),
        m_transform()
    {
    }


    //
    void C10p0to8p0Mediator::mainLoop()
    {

      // processOne returns false when eof is reached, otherwise it is true
      while (processOne()) {}

    }


    // Retrieve the remaining text buffers from the transform and output them
    void C10p0to8p0Mediator::outputExtraTextBuffers(CDataSink& sink)
    {
      auto& buffers = m_transform.getStagedTextBuffers();
      for (auto & buffer : buffers) {
        sink << (V8::format_cast<V8::CRawBuffer>(buffer));
      }

      // make sure to reset
      m_transform.clearStagedTextBuffers();
    }


    //
    //
    bool C10p0to8p0Mediator::processOne() {

      CDataSource& source = *getDataSource();
      CDataSink& sink = *getDataSink();

      // Extract
      V10::CRingItem item1(V10::VOID);
      source >> item1;

      if (source.eof()) {
        return false;
      }

      try {


        if ( typeDemandsFlush(item1.type()) && dataToFlush() ) {
          auto rawBuffer = V8::format_cast<V8::CRawBuffer>(m_transform.getCurrentPhysicsBuffer());
          sink << rawBuffer;

          m_transform.startNewPhysicsBuffer();
        }

        // convert
        V8::CRawBuffer item2 = m_transform(item1);

        if (item2.getHeader().type == V8::RUNVARBF
            || item2.getHeader().type == V8::PKTDOCBF
            || item2.getHeader().type == V8::STATEVARBF
            || item2.getHeader().type == V8::PARAMDESCRIP ) {

          outputExtraTextBuffers(sink);
        }

        if (item2.getHeader().type != V8::VOID) {
          sink << item2;
        }

      } catch (std::exception& exc) {
        std::cout << exc.what() << std::endl;
      } catch (...) {
        std::cout << "Caught an error" << std::endl;
      }

      return !source.eof();
    }



    //
    bool C10p0to8p0Mediator::typeDemandsFlush(std::uint32_t v10type) const
    {
      return (v10type != DAQ::V10::PHYSICS_EVENT
          && v10type != DAQ::V10::EVB_FRAGMENT
          && v10type != DAQ::V10::EVB_UNKNOWN_PAYLOAD
          && v10type != DAQ::V10::PHYSICS_EVENT_COUNT);
    }

    //
    bool C10p0to8p0Mediator::dataToFlush() const
    {
      std::size_t nEvents = m_transform.getCurrentPhysicsBuffer().size();
      return (nEvents > 0);
    }

  } // namespace Transform
} // namespace DAQ
