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

#include "C10p0to11p0Mediator.h"
#include <CDataSink.h>
#include <V11/CDataFormatItem.h>

namespace DAQ {
  namespace Transform {

    ///////////////////////////////////////////////////////////////////////////
    // C10p0to11p0MediatorCreator IMplementation

    CBaseMediator* C10p0to11p0MediatorCreator::operator ()(void* unused)  {
      return (new C10p0to11p0Mediator());
    }



    ///////////////////////////////////////////////////////////////////////////
    // C10p0to11p0Mediator Implementation
    ///////////////////////////////////////////////////////////////////////////

    //
    C10p0to11p0Mediator::C10p0to11p0Mediator(std::unique_ptr<CDataSource> source,
                                           std::unique_ptr<CDataSink> sink)
      : CBaseMediator(std::unique_ptr<CDataSource>(), std::unique_ptr<CDataSink>()),
        m_mediator(std::move(source), std::move(sink))
    {
    }


    //
    void C10p0to11p0Mediator::mainLoop()
    {

      outputRingFormat();

      m_mediator.mainLoop();
    }

    //
    void C10p0to11p0Mediator::outputRingFormat()
    {
      CDataSink& sink = *m_mediator.getDataSink();
      sink << V11::CDataFormatItem();
    }

  } // namespace Transform
} // namespace DAQ
