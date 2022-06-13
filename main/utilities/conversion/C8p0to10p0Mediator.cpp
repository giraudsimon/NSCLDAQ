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

#include "C8p0to10p0Mediator.h"
#include <V10/CRingItem.h>
#include <V10/CPhysicsEventItem.h>
#include <V8/CRawBuffer.h>
#include <V8/DataFormatV8.h>
#include <V10/DataFormatV10.h>
#include <CDataSource.h>
#include <CDataSink.h>
#include <RingIOV10.h>
#include <BufferIOV8.h>
#include <exception>

namespace DAQ {
  namespace Transform {
    

    ///////////////////////////////////////////////////////////////////////////
    // C8p0to10p0MediatorCreator Implementation
    //

    CBaseMediator* C8p0to10p0MediatorCreator::operator ()(void* unused)  {
      return (new C8p0to10p0Mediator());
    }

    ///////////////////////////////////////////////////////////////////////////
    // C8p0to10p0Mediator Implementation
    //

    C8p0to10p0Mediator::C8p0to10p0Mediator(std::unique_ptr<CDataSource> source,
                                           std::unique_ptr<CDataSink> sink)
      : CBaseMediator(move(source), move(sink)),
        m_transform()
    {
    }

    //
    void C8p0to10p0Mediator::mainLoop()
    {
      // keep running until we fail
      while (processOne()) {}

    }

    //
    //
    bool C8p0to10p0Mediator::processOne() {

      CDataSource& source = *getDataSource();
      CDataSink& sink = *getDataSink();

      // extract data
      V8::CRawBuffer item1;
      source >> item1;
      if (source.eof()) {
        return false;
      }

      try {

        // convert between versions
        V10::CRingItem item2 = m_transform(item1);

        if (item2.type() != 0) {
          sink << item2;

          // handle the special case of physics events
          if (item2.type() == V10::PHYSICS_EVENT) {
            auto& events = m_transform.getRemainingEvents();
            for (auto & event : events) {
              sink << V10::CRingItem(event);
            }
          }

        }

      } catch (std::exception& exc) {
        std::cout << exc.what() << std::endl;
      } catch (...) {
        std::cout << "Caught an error" << std::endl;
      }

      return !source.eof();
    }

  } // namespace Transform
} // namespace DAQ
