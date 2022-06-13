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

#ifndef DAQ_TRANSFORM_C10P0TO8P0MEDIATOR_H
#define DAQ_TRANSFORM_C10P0TO8P0MEDIATOR_H

#include <CBaseMediator.h>
#include <CTransformFactory.h>
#include <CTransform10p0to8p0.h>
#include <memory>

class CDataSource;
class CDataSink;

namespace DAQ {
  namespace Transform {

    // Fwd declaration for the creator
    class C10p0to8p0Mediator;

    /*!
     * \brief The creator class for the extensible factory
     */
    class C10p0to8p0MediatorCreator : public CTransformCreator {
    public:
      CBaseMediator* operator()(void* unused);
    };




    /*! \brief Mediator to handle buffering issues
     *
     * There is not necessarily a 1-to-1 correspondence between ring items and
     * buffers. This class handles that logic. It handles getting data form the
     * source to the transform and then sending the data from the transform
     * to the sink.
     *
     * In this implementation, the main loop is just while loop that calls the
     * processOne() method. That is where the real logic of this mediator lives.
     */
    class C10p0to8p0Mediator : public CBaseMediator
    {
      CTransform10p0to8p0 m_transform;

    public:
      C10p0to8p0Mediator(std::unique_ptr<CDataSource> source = std::unique_ptr<CDataSource>(),
                         std::unique_ptr<CDataSink> sink = std::unique_ptr<CDataSink>());

      void initialize() {} // no op

      /*!
       * \brief Continue processing until EOF is encountered
       *
       */
      void mainLoop();

      // no op
      void finalize() {}

      /*!
       * \brief The logic.
       *
       * If a physics event item arrives, this ensures that the current buffer is flushed
       * if it is necessary.
       *
       * If a type arrives that demands a flush, the current physics event buffer will be flushed if
       * there is data in it.
       *
       * If a text item arrives, this ensures all of the buffers produced by the single
       * text item get outputted.
       *
       * \return true  - EOF not reached
       *         false - EOF reached
       */
      bool processOne();

      ////// HELPER METHODS /////////////////////////////////////////////////////
    private:
      /*!
       * \brief Logic to output overflowed text buffers from transform
       * \param sink
       */
      void outputExtraTextBuffers(CDataSink& sink);

      /*!
       * \brief Determines if the type warrants flushing the current physics buffer
       *
       * \param v10type  - type id of V10 ring item
       * \return false - PHYSICS_EVENT, EVB_FRAGMENT, EVB_UKNOWN_PAYLOAD,
       *                 PHYSICS_EVENT_COUNT
       *         true - otherwise
       */
      bool typeDemandsFlush(std::uint32_t v10type) const;

      /*!
       * \brief Does the current physics event buffer contain data?
       * \return true - data exists
       *         false - otherwise
       */
      bool dataToFlush() const;
    };
  } // namespace Transform
} // namespace DAQ

#endif // DAQ_TRANSFORM_C10P0TO8P0MEDIATOR_H
