/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/



#ifndef CONESHOTMEDIATOR_H
#define CONESHOTMEDIATOR_H

#include <COneShotHandler.h>
#include <CMediator.h>

class CDataSource;
class CFilter;
class CDataSink;
class CRingItem;
class CRingStateChangeItem;


/**! \brief A mediator that understands one-shot logic
 *
 * The one shot logic is built into the COneShotHandler class. Basically,
 * it should be understood as causing operations to occur throughout a run 
 * and then automatically shutting down when the sufficient number of 
 * end run events have been observed.
 *
 */
class COneShotMediator : public CMediator
{
  private:
    COneShotHandler m_oneShot; //!< logic handler for oneshot operations

  public:
    // The constructor
    COneShotMediator(CDataSource* source, CFilter* filter, CDataSink* sink);
    // The constructor
    COneShotMediator(CDataSource* source, CFilter* filter, CDataSink* sink, 
                      int nsources);

    virtual ~COneShotMediator();

  private:
    // Copy and assignment do not make sense because ownership
    // is not transferrable of the CDataSource and CDataSink.
    COneShotMediator(const COneShotMediator&);
    COneShotMediator& operator=(const COneShotMediator&);

  public:

    /**! The main loop
    */
    void mainLoop();

    /**! Initialize operations 
     *
     *  This simply calls the initialize method of the filter.
     *
     */
    void initialize(); 

    /**! Finalization operations 
     *
     *  This simply calls the finalize method of the filter.
     */
    void finalize(); 

};

#endif
