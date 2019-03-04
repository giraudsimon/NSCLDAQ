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



#ifndef CBASEMEDIATOR_H
#define CBASEMEDIATOR_H

#include <memory>
#include <CDataSource.h>
#include <CDataSink.h>

class CBaseMediator
{
  protected:
    std::unique_ptr<CDataSource> m_pSource; //!< the source
    std::unique_ptr<CDataSink>   m_pSink; //!< the sink 

  public:
    // The constructor
    CBaseMediator(std::unique_ptr<CDataSource> pSource = std::unique_ptr<CDataSource>(),
                  std::unique_ptr<CDataSink> pSink     = std::unique_ptr<CDataSink>());

    virtual ~CBaseMediator();

  private:
    // Copy and assignment do not make sense because ownership
    // is not transferrable of the CDataSource and CDataSink.
    CBaseMediator(const CBaseMediator&) = delete;
    CBaseMediator& operator=(const CBaseMediator&) = delete;

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

    /**! Set the source
      This transfers ownership of the object to this CBaseMediator.
      Ownership of the previous source is transfered to the caller.
  
      \param source the new source
      \return the old source 
    */
    CDataSource* setDataSource( CDataSource* source) 
    {
        CDataSource* old_source = m_pSource.release();
        m_pSource.reset(source);
        return old_source;
    }  
    virtual void setDataSource( std::unique_ptr<CDataSource>& pSource) {
      m_pSource.swap(pSource);
    }
    /**! Set the sink
      This transfers ownership of the object to this CBaseMediator.
      Ownership of the previous sink is transfered to the caller.
  
      \param sink the new sink
      \return the old sink 
    */
    CDataSink* setDataSink( CDataSink* sink) 
    {
        CDataSink* old_sink = m_pSink.get();
        m_pSink.reset(sink);
        return old_sink;
    }  
    virtual void setDataSink( std::unique_ptr<CDataSink>& pSink) {
      m_pSink.swap(pSink);
    }

    /**! Access to the source 
    */
    virtual CDataSource* getDataSource() { return m_pSource.get();}

    /**! Access to the sink 
    */
    virtual CDataSink* getDataSink() { return m_pSink.get();}

};

#endif
