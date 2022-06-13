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

#ifndef DAQ_TRANSFORM_C8PO0TO10P0MEDIATOR_H
#define DAQ_TRANSFORM_C8PO0TO10P0MEDIATOR_H

#include <CBaseMediator.h>
#include <CTransform8p0to10p0.h>
#include <CTransformFactory.h>

#include <memory>

class CDataSource;
class CDataSink;

namespace DAQ {
  namespace Transform {

    ///////////////////////////////////////////////////////////////////////////

    // fwd declaration of mediator for creator class
    class C8p0to10p0Mediator;

    /*!
     * \brief The C8p0to10p0MediatorCreator class
     */
    class C8p0to10p0MediatorCreator : public CTransformCreator {
    public:
      CBaseMediator* operator()(void* unused);
    };


    ///////////////////////////////////////////////////////////////////////////

    /*!  \brief Specialized mediator for 8->10 conversions
     *
     * Because there is not a 1-to-1 relationship between version 8 buffers and
     * version 10 ring items, the mediator has to work with the transform to
     * output the correct amount of ring items. This does just that.
     */
    class C8p0to10p0Mediator : public CBaseMediator
    {
      CTransform8p0to10p0 m_transform;

    public:
      C8p0to10p0Mediator(std::unique_ptr<CDataSource> source = std::unique_ptr<CDataSource>(),
                         std::unique_ptr<CDataSink> sink = std::unique_ptr<CDataSink>());

      void initialize() {}

      /*!
       * \brief Repeatedly call processOne() until the EOF is reached
       */
      void mainLoop();

      void finalize() {}

      /*!
       * \brief Process a single version 8 buffer
       *
       * A single version  8 buffer is read from the data source, transformed into version 10 item,
       * and then all of the resulting items are outputted to the sink.
       *
       * \return true - EOF has not be reached in source
       *         false - EOF has been reached
       */
      bool processOne();
    };
    
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_C8PO0TO10P0MEDIATOR_H
