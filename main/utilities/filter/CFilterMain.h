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




#ifndef CFILTERMAIN_H
#define CFILTERMAIN_H

#include <vector> 
#include <stdint.h>
#include <CFatalException.h>

class CMediator;
class CDataSource;
class CFilter;
class CDataSink;
class CRingItem;
struct gengetopt_args_info;

class CFilterMain
{
  
  private:
    CMediator* m_mediator; //!< The mediator
    struct gengetopt_args_info* m_argsInfo; //!< The parsed options
    CDataSink* m_pSink;    //!< data sink.
public:
    static bool m_testing;      //!<  forces use of test sources and sinks.


  public:
    /**! Constructor
      Constructs a mediator object with a CCaesarFilter
      as the default filter. 
    
      \param argc the number of command line args
      \param argv the command line args

      \throw can throw a CFatalException

     */
    CFilterMain(int argc, char** argv);

    /**! Destructor
      This does absolutely nothing.
    */
    ~CFilterMain();

    /**! Main loop 
      Executes the CMediator::mainLoop() 
    */
    void operator()();

    /**! Append a filter to the mediator's ccomposite filter
      Note that because the filter argument will be used solely 
      to call a clone, it is very important that any derived class
      will provide its own clone method.
  
      \param filter a template of the filter to register
    */
    void registerFilter(const CFilter* filter);

    /**! Retrieve the mediator
     *
     * Ownership of the mediator remains with the CFilterMain instance.
     *
     * \returns ptr to the mediator
     */
    CMediator* getMediator() { return m_mediator; }
    void putRingItem(CRingItem* pRingItem);

  private:
    // Private utility functions 
    CDataSource* constructDataSource();
    CDataSink* constructDataSink();

    std::vector<uint16_t> constructExcludesList();
    std::vector<uint16_t> constructSampleList();
    
};

#endif
