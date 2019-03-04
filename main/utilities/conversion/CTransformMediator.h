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


#ifndef CTRANSFORMMEDIATOR_H
#define CTRANSFORMMEDIATOR_H

#include <CPredicate.h>
#include <CBaseMediator.h>

#include <memory>

class CDataSource;
class CDataSink;

namespace DAQ {
  namespace Transform {


/**! \brief A mediator that never quits unless count is satisfied or stream ends.
 *
 *  This provides a very basic mediator that will read from a source and then 
 *  output to a file forever. The only reason it would stop is if there is a 
 *  count parameter specified or if the source fails.
 *
 */
template<class Transform>
class CTransformMediator  : public CBaseMediator
{
  private:
    Transform m_transform;
    std::unique_ptr<CPredicate> m_pPredicate;

  public:
    // The constructor
    CTransformMediator(std::unique_ptr<CDataSource> source = std::unique_ptr<CDataSource>(),
                       std::unique_ptr<CDataSink> sink = std::unique_ptr<CDataSink>(),
                       Transform trans=Transform());

    virtual ~CTransformMediator();

  private:
    // Copy and assignment do not make sense because ownership
    // is not transferrable of the CDataSource and CDataSink.
    CTransformMediator(const CTransformMediator&);
    CTransformMediator& operator=(const CTransformMediator&);

  public:
    /**! The main loop
    */
    virtual void mainLoop();

    /**! Initialize operations 
     *
     *  This simply calls the initialize method of the filter.
     *
     */
    virtual void initialize();

    /**! Finalization operations 
     *
     *  This simply calls the finalize method of the filter.
     */
    virtual void finalize();

  private:
    void updatePredicate();
    bool processOne();

};

  } // end of Transform
} // end of DAQ

// include the implementation
#include <CTransformMediator.hpp>



#endif
