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


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";

#include "CFilterMain.h"
#include "CCompositeFilter.h"
#include "CMediator.h"
#include "COneShotMediator.h"
#include "CInfiniteMediator.h"
#include "CDataSourceFactory.h"
#include "CDataSinkFactory.h"
#include <string>
#include <DataFormat.h>
#include <StringsToIntegers.h>
#include <vector>
#include <iostream>
#include <stdlib.h>
#include "filterargs.h"
#include "CFatalException.h"
#include <stdexcept>
#include "CFilterTestSink.h"
#include "CFilterTestSource.h"

bool CFilterMain::m_testing(false);

/**! Constructor
  Constructs a mediator object with a CCompositeFilter
  as the default filter. We also set up the proper 
  skip and processing counts that user supplied.

  \throw can throw a CFatalException

*/
CFilterMain::CFilterMain(int argc, char** argv)
  : m_mediator(0),
  m_argsInfo(new gengetopt_args_info),
  m_pSink(nullptr)
{
  cmdline_parser(argc,argv,m_argsInfo);  

  try {

    if (m_argsInfo->oneshot_given) {
      m_mediator = new COneShotMediator(0,new CCompositeFilter,0,
          m_argsInfo->number_of_sources_arg); 
    } else {
      m_mediator = new CInfiniteMediator(0,new CCompositeFilter,0);
    }
    
    // Set up the data source 
    CDataSource* source = constructDataSource(); 
    m_mediator->setDataSource(source);

    // Set up the sink source 
    CDataSink* sink = constructDataSink();
    m_pSink = sink;
    m_mediator->setDataSink(sink);

    // set up the skip and count args
    if (m_argsInfo->skip_given) {
      m_mediator->setSkipCount(m_argsInfo->skip_arg);
    }  

    if (m_argsInfo->count_given) {
      m_mediator->setProcessCount(m_argsInfo->count_arg);
    }  



  } catch (CException& exc) {
    std::cout << exc.ReasonText() << std::endl;
    std::cout << exc.WasDoing() << std::endl;
    throw CFatalException();
  }
  catch (std::exception& e) {
    std::cout << e.what() << std::endl;
    throw CFatalException();
  }
  catch (...) {
    std::cout << "Unanticipated exception type\n";
    throw CFatalException();
  }
}


CFilterMain::~CFilterMain()
{
  delete m_argsInfo;
  delete m_mediator;
}

/**! Append a filter to the mediator's ccomposite filter
  Note that because the filter argument will be used solely 
  to call a clone, it is very important that any derived class
  will provide its own clone method.

  \param filter a template of the filter to register
 */
void CFilterMain::registerFilter(const CFilter* filter)
{
  // We will always have a composite filter in this main
  CCompositeFilter* main_filter=0;
  main_filter = dynamic_cast<CCompositeFilter*>(m_mediator->getFilter());

  main_filter->registerFilter(filter);
}
/**
 * putRingItem
 *    Puts a ring item to the data sink.  This is our solution to
 *    daqdev/NSCLDAQ#804 - support filter elements creating several ring items
 *    per input ring item.
 *
 * @param pRingItem - pointer to the ring item to put to the sink.
 */
 void
 CFilterMain::putRingItem(CRingItem* pRingItem)
 {
  m_pSink->putItem(*pRingItem);
 }

/////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
////// Private utilities

/**! Construct a data source
    A data source must provide a sample and an excludes list along
    with a uri. The default value for this is stdin. 
  
    \exception May propagate CErrnoException,s CURLFormatException,
        or CFatalException
*/
CDataSource* CFilterMain::constructDataSource()
{
 if (m_testing) {
  return new CFilterTestSource;
 } else {
  // Set up default source type
  std::string source_name("-");
  if (m_argsInfo->source_given) {
    source_name = std::string(m_argsInfo->source_arg);
  } 

  // Set up the sampling 
  std::vector<uint16_t> sample = constructSampleList();
  
  // Set up the excludes
  std::vector<uint16_t> exclude = constructExcludesList();
  
  CDataSource* source=0;
  return CDataSourceFactory().makeSource(source_name,sample,exclude);
 }
}

/**! Set up the data sink
  
    Based on user's argument --sink, generates the appropriate source
    type.
*/
CDataSink* CFilterMain::constructDataSink()
{
 if (m_testing) {
  return new CFilterTestSink;
 } else {
  // Set up default source type
  std::string sink_name("-");
  if (m_argsInfo->sink_given) {
    sink_name = std::string(m_argsInfo->sink_arg);
  } 

  CDataSink* sink=CDataSinkFactory().makeSink(sink_name);
  return sink;
 }
}

/**! The main loop
    This is just a wrapper around the mediator's mainLoop. It is here
    that the processing occurs in the application. */
void CFilterMain::operator()()
{
  try {
    m_mediator->initialize();

    // allow the finalize operations to be called if an exception is 
    // thrown from the main loop. Consider the arrival of an 
    // ABNORMAL_ENDRUN
    try {
      m_mediator->mainLoop();
    } catch (CException& exc) {
      std::cerr << exc.WasDoing() << " : " << exc.ReasonText() << std::endl;
    } catch (...) {
      std::cerr << "Caught unknown exception thrown from main loop. ";
      std::cerr << "Shutting down filter." << std::endl;
    }

    m_mediator->finalize();
  } catch (CException& exc) {
    std::cerr << exc.WasDoing() << " : " << exc.ReasonText() << std::endl;
    throw CFatalException(); 
  }

}


/**! Convert user's string arguments to integers 
  This translates the user's command line input for the --sample option
  from ring item type names to their numerical identifiers. 

  \return a list of ring-item numbers corresponding to user's --sample option
*/
std::vector<uint16_t> 
CFilterMain::constructSampleList() 
{

  std::vector<uint16_t> sample;
  std::vector<int> s;
  if (m_argsInfo->sample_given) {
    try {
      s = stringListToIntegers(std::string(m_argsInfo->sample_arg));
    }
    catch (...) {
      std::cerr << "Invalid value for --sample, must be a list of item types was: "
        << std::string(m_argsInfo->sample_arg) << std::endl;
      throw CFatalException();
    }
    for(int i=0; i < s.size(); i++) {
      sample.push_back(s[i]);
    }
  }

  return sample;
}

/**! 
  Makes a call to the stringListToIntegers that maps the
  the names of ring item types to their associated identifying
  numbers. 

  \return a list of ring item numbers
*/
std::vector<uint16_t> 
CFilterMain::constructExcludesList()
{
  std::vector<uint16_t> exclude;
  std::vector<int> e;
  if (m_argsInfo->exclude_given) {
    try {
      e = stringListToIntegers(std::string(m_argsInfo->exclude_arg));
    }
    catch (...) {
      std::cerr << "Invalid value for --exclude, must be a list of item types was: "
        << std::string(m_argsInfo->sample_arg) << std::endl;
      throw CFatalException();
      
    }
    for (int i = 0; i < e.size(); i++) {
      exclude.push_back(e[i]);
    }
  }
  return exclude;
}
