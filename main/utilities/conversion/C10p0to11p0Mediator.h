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


#ifndef DAQ_TRANSFORM_C10P0TO11P0MEDIATOR_H
#define DAQ_TRANSFORM_C10P0TO11P0MEDIATOR_H

#include <CBaseMediator.h>
#include <CTransformFactory.h>
#include <CTransformMediator.h>
#include <CTransform10p0to11p0.h>
#include <memory>



namespace DAQ {
  namespace Transform {
    
    class C10p0to11p0Mediator;

    /*!
     * \brief The C10p0to11p0MediatorCreator class
     */
    class C10p0to11p0MediatorCreator : public CTransformCreator {
    public:
      CBaseMediator* operator()(void* unused);
    };



    /*! \brief Decorate a 10 -> 11 transform mediator
     *
     * The only unique thing this does is spit out a data format item
     * before entering the main loop. It is just a decorator pattern on top
     * of a standard CTransformMediator.
     *
     */
    class C10p0to11p0Mediator : public CBaseMediator
    {
      CTransformMediator<CTransform10p0to11p0> m_mediator; // actual mediator

    public:
      /*!
       * \brief C10p0to11p0Mediator
       *
       * Simply passes on the sink and source to the actual transform mediator.
       *
       * \param source data source
       * \param sink   data sink
       */
      C10p0to11p0Mediator(std::unique_ptr<CDataSource> source = std::unique_ptr<CDataSource>(),
                         std::unique_ptr<CDataSink> sink = std::unique_ptr<CDataSink>());

      /*!
       * \brief mainLoop
       *
       *  OUtputs a ring data format item and then calls the main loop of the m_mediator
       */
      virtual void mainLoop();

      virtual void initialize() { m_mediator.initialize();}
      virtual void finalize() { m_mediator.finalize();}

      virtual CDataSource* getDataSource() { return m_mediator.getDataSource(); }
      virtual CDataSink*   getDataSink()   { return m_mediator.getDataSink(); }

      virtual void setDataSource(std::unique_ptr<CDataSource> &pSource)
      { m_mediator.setDataSource(pSource); }

      virtual void setDataSink(std::unique_ptr<CDataSink> &pSink)
      { m_mediator.setDataSink(pSink); }

    private:

      /*!
       * \brief Send a ring format item to the sink
       */
      void outputRingFormat();
    };
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_C10P0TO11P0MEDIATOR_H
