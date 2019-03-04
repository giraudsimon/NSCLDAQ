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



#ifndef CMEDIATOR_H
#define CMEDIATOR_H

#include <CBaseMediator.h>
#include <CFilter.h>
#include <memory>

class CDataSource;
class CDataSink;
class CRingItem;
class CRingStateChangeItem;


class CMediator : public CBaseMediator
{
  private:
    std::unique_ptr<CFilter>     m_pFilter; //!< the filter 
    int  m_nToProcess; //!< number to process
    int  m_nToSkip; //!< number to skip

  public:
    // The constructor
    CMediator(CDataSource* source, CFilter* filter, CDataSink* sink);
    CMediator(std::unique_ptr<CDataSource> source, std::unique_ptr<CFilter> filter, 
              std::unique_ptr<CDataSink> sink);

    virtual ~CMediator();

  private:
    // Copy and assignment do not make sense because ownership
    // is not transferrable of the CDataSource and CDataSink.
    CMediator(const CMediator&);
    CMediator& operator=(const CMediator&);

  public:

    /**! The main loop
    *   
    *  This is to be defined by the derived class.
    *
    */
    virtual void mainLoop() = 0;

    /**! Initialize procedure 
     *
     *  Depending on the mediator, this may look different.
     *
     */
    virtual void initialize() = 0;

    /**! Finalization procedure.
     *
     *  Depending on the mediator, this may look different.
     *
     */
    virtual void finalize() = 0;

    /**! Set the filter
      This transfers ownership of the object to this CMediator.
      Ownership of the previous filter is transfered to the caller.
  
      \param filter the new filter
      \return the old filter 
    */
    CFilter* setFilter( CFilter* filter) 
    {
        CFilter* old_filter = m_pFilter.release();
        m_pFilter.reset(filter);
        return old_filter;
    }  


    /**! Access to the filter 
    */
    CFilter* getFilter() { return m_pFilter.get();}

    /**! Set the number to skip */
    void setSkipCount(int nEvents) { m_nToSkip = nEvents; }
    /**! Get the number to skip */
    int getSkipCount(void) const { return m_nToSkip; }

    /**! Set the number to process */
    void setProcessCount(int nEvents) { m_nToProcess = nEvents; }
    /**! Get the number to process */
    int getProcessCount(void) const { return m_nToProcess; }

  protected:
    /**! Delegate item to proper handler of filter
    */
    virtual CRingItem* handleItem(CRingItem* item); 

};

#endif
