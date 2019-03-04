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


#include <fstream>
#include <ios>
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <URL.h>

#include <CPhysicsEventItem.h>
#include <CRingStateChangeItem.h>
#include <CRingScalerItem.h>
#include <CRingTextItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingFragmentItem.h>
#include <CFileDataSource.h>
#include <CFileDataSink.h>
#include <CDataSinkFactory.h>
#include <CDataSourceFactory.h>

#include "CTransparentFilter.h"
#include "CNullFilter.h"
#include "CTestFilter.h"

#include <cppunit/extensions/HelperMacros.h>

#define private public
#define protected public
#include "CInfiniteMediator.h"
#undef private
#undef protected

// A test suite 
class CInfiniteMediatorTest : public CppUnit::TestFixture
{

  private:
    CFilter* m_filter;
    CDataSource* m_source;
    CDataSink* m_sink;
    CInfiniteMediator* m_mediator;

  public:
    CInfiniteMediatorTest();

    CPPUNIT_TEST_SUITE( CInfiniteMediatorTest );
    CPPUNIT_TEST ( testConstructor );

    CPPUNIT_TEST ( testSetMembers );

    CPPUNIT_TEST ( testHandleStateChangeItem );
    CPPUNIT_TEST ( testHandleScalerItem );
    CPPUNIT_TEST ( testHandleTextItem );
    CPPUNIT_TEST ( testHandlePhysicsEventItem );
    CPPUNIT_TEST ( testHandlePhysicsEventCountItem );
    CPPUNIT_TEST ( testHandleFragmentItem );
    CPPUNIT_TEST ( testHandleGenericItem );
    CPPUNIT_TEST ( testSkipNone );
    CPPUNIT_TEST ( testSkipSome );
    CPPUNIT_TEST ( testProcessSome );

    CPPUNIT_TEST ( testTransparentMainLoop );

    CPPUNIT_TEST ( testFilterReturnsNULL );

    CPPUNIT_TEST ( initialize_0 );
    CPPUNIT_TEST ( finalize_0 );
//    CPPUNIT_TEST ( testMultiSourceStates );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testConstructor();

    void testSetMembers();

    void testHandleStateChangeItem();
    void testHandleScalerItem();
    void testHandleTextItem();
    void testHandlePhysicsEventItem();
    void testHandlePhysicsEventCountItem();
    void testHandleFragmentItem();
    void testHandleGenericItem();

    void testSkipNone();
    void testSkipSome();
    void testProcessSome();

    void testTransparentMainLoop();
//    void testMultiSourceStates();

    void testFilterReturnsNULL();

    void initialize_0();
    void finalize_0();

  private:
    size_t writeRingItemToFile(CRingItem& item,
        std::string fname,
        std::ios::openmode mode);


    bool filesEqual(std::string fname0, std::string fname1);
    bool compareEqual(CRingItem& item0, CRingItem& item1);
    void setUpSkipTestFile(int nToSkip, 
                           std::string infname, 
                           std::string ofname);

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CInfiniteMediatorTest );

CInfiniteMediatorTest::CInfiniteMediatorTest()
    : m_filter(0),
    m_source(0),
    m_sink(0),
    m_mediator(0)
{}


void CInfiniteMediatorTest::setUp()
{
  std::vector<uint16_t> dummy;
  m_filter = new CTestFilter;
  m_source = new CFileDataSource(STDIN_FILENO, dummy);
  m_sink = new CFileDataSink(STDOUT_FILENO);

  m_mediator = new CInfiniteMediator(m_source,m_filter,m_sink);
  m_mediator->setDataSource(m_source);
  m_mediator->setFilter(m_filter);
  m_mediator->setDataSink(m_sink);
}

void CInfiniteMediatorTest::tearDown()
{
  // Call the destructor to free
  // owned memory
  delete m_mediator; m_mediator=0;
}


void CInfiniteMediatorTest::testConstructor()
{
  CPPUNIT_ASSERT_EQUAL( m_source, m_mediator->m_pSource);
  CPPUNIT_ASSERT_EQUAL( m_filter, m_mediator->m_pFilter);
  CPPUNIT_ASSERT_EQUAL( m_sink, m_mediator->m_pSink);
}

void CInfiniteMediatorTest::testSetMembers()
{
    CFilter* new_filter = 0;
    CFilter* old_filter = m_mediator->setFilter(new_filter);
    CPPUNIT_ASSERT_EQUAL( m_filter, old_filter );
    CPPUNIT_ASSERT_EQUAL( new_filter, m_mediator->m_pFilter );

    CDataSource* new_source = 0;
    CDataSource* old_source = m_mediator->setDataSource(new_source);
    CPPUNIT_ASSERT_EQUAL( m_source,  old_source );
    CPPUNIT_ASSERT_EQUAL( new_source, m_mediator->m_pSource );

    CDataSink* new_sink = 0;
    CDataSink* old_sink = m_mediator->setDataSink(new_sink);
    CPPUNIT_ASSERT_EQUAL( m_sink , old_sink );
    CPPUNIT_ASSERT_EQUAL( new_sink , m_mediator->m_pSink );

}


void CInfiniteMediatorTest::testHandleStateChangeItem()
{
    const CRingItem* new_item=0; 

    // Create some generic item type to force testing of handleStateChangeItem()
    {
      CRingStateChangeItem my_state_item(END_RUN);    
      new_item = m_mediator->handleItem(&my_state_item);
      // Test filter should always return type BEGIN_RUN
      CPPUNIT_ASSERT( BEGIN_RUN == new_item->type() );
      delete new_item;
    }
} 
    // Create scaler item
void CInfiniteMediatorTest::testHandleScalerItem()
{
  CRingScalerItem my_sclr_item(300);

  const CRingItem* new_item = m_mediator->handleItem(&my_sclr_item);

  const CRingScalerItem* new_sclr = dynamic_cast<const CRingScalerItem*>(new_item);
  // Test filter should always return 200 scalers
  CPPUNIT_ASSERT( 200 == new_sclr->getScalerCount() );
  delete new_item;
}
  
    // Text item
void CInfiniteMediatorTest::testHandleTextItem()
{
  std::vector<std::string> str_vec;
  str_vec.push_back("testing 123");
  CRingTextItem my_text_item(MONITORED_VARIABLES,str_vec);
  const CRingItem* new_item = m_mediator->handleItem(&my_text_item);
  // Test filter should always return 200 scalers
  const CRingTextItem* new_text = dynamic_cast<const CRingTextItem*>(new_item);

  CPPUNIT_ASSERT( 3 == new_text->getStrings().size() );
  CPPUNIT_ASSERT( "0000" == new_text->getStrings()[0] );
  CPPUNIT_ASSERT( "1111" == new_text->getStrings()[1] );
  CPPUNIT_ASSERT( "2222" == new_text->getStrings()[2] );
  delete new_item;
}

// PhysicsEvent item
void CInfiniteMediatorTest::testHandlePhysicsEventItem()
{
  CPhysicsEventItem my_evt_item(8192);

  const CRingItem* new_item = m_mediator->handleItem(&my_evt_item);

  const CPhysicsEventItem* new_evt = dynamic_cast<const CPhysicsEventItem*>(new_item);

  CPPUNIT_ASSERT( 4096 == new_evt->getStorageSize() );
  delete new_item;
}    

    // PhysicsEventCount item
void CInfiniteMediatorTest::testHandlePhysicsEventCountItem()
{
  CRingPhysicsEventCountItem my_cnt_item(static_cast<uint64_t>(100),
                                         static_cast<uint32_t>(100));

  const CRingItem* new_item = m_mediator->handleItem(&my_cnt_item);
  const CRingPhysicsEventCountItem* new_cnt 
    = dynamic_cast<const CRingPhysicsEventCountItem*>(new_item);

  CPPUNIT_ASSERT( static_cast<uint64_t>(4) == new_cnt->getEventCount() );
  CPPUNIT_ASSERT( static_cast<uint32_t>(1001) == new_cnt->getTimeOffset() );
  delete new_item;
}    

// RingFragmentItem
void CInfiniteMediatorTest::testHandleFragmentItem()
{
  CRingFragmentItem my_frag_item(static_cast<uint64_t>(0),
      static_cast<uint32_t>(0),
      static_cast<uint32_t>(0),
      reinterpret_cast<void*>(0),
      static_cast<uint32_t>(0));
  const CRingItem* new_item = m_mediator->handleItem(&my_frag_item);
  const CRingFragmentItem* new_frag = dynamic_cast<const CRingFragmentItem*>(new_item);
  CPPUNIT_ASSERT( static_cast<uint64_t>(10101) == new_frag->timestamp());
  CPPUNIT_ASSERT( static_cast<uint32_t>(1) == new_frag->source());
  CPPUNIT_ASSERT( static_cast<uint32_t>(2) == const_cast<CRingFragmentItem*>(new_frag)->payloadSize());
  CPPUNIT_ASSERT( static_cast<uint32_t>(3) == new_frag->barrierType());
  delete new_item;
}

// Create some generic item type to force testing of handleRingItem()
void CInfiniteMediatorTest::testHandleGenericItem()
{
  CRingItem my_generic_item(1000);    
  const CRingItem* new_item = m_mediator->handleItem(&my_generic_item);
  // Test filter should always return type 100
  CPPUNIT_ASSERT( 100 == new_item->type() );
  delete new_item;
}


void CInfiniteMediatorTest::testSkipNone()
{
  tearDown();

  // This should have no effect on any default behavior
  // We will simply test this as the TransparentMainLoop
  m_mediator = new CInfiniteMediator(0,0,0);
  m_mediator->setSkipCount(0);

  std::string proto("file://");
  std::string infname("./run-0000-00.evt");
  std::string outfname("./copy-run-0000-00.evt");

  try {
    URL uri(proto+infname);
    m_source = new CFileDataSource(uri, std::vector<uint16_t>());
    m_sink = new CFileDataSink(outfname);

    m_mediator->setDataSource(m_source);
    m_mediator->setDataSink(m_sink);
    m_mediator->setFilter(new CTransparentFilter);

    m_mediator->mainLoop();

    // kill all of the sinks and sources
    tearDown();
    // set up defaults so that we don't segfault at tearDown
    setUp();
  } catch (CException& exc) {
    std::stringstream errmsg; errmsg << "Caught exception:" << exc.ReasonText();
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (int errcode) {
    std::stringstream errmsg; errmsg << "Caught integer " << errcode;
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (std::string errmsg) {
    CPPUNIT_FAIL(errmsg.c_str()); 
  }

  CPPUNIT_ASSERT( filesEqual(infname,outfname) );

  remove(outfname.c_str());

}

void CInfiniteMediatorTest::testSkipSome()
{
  tearDown();
 
  int nToSkip=4;

  std::string proto("file://");
  std::string infname("./run-0000-00.evt");
  std::string outfname("./copy-run-0000-00.evt");
  std::string modeloutfname("./model-skip-run-0000-00.evt");

  // Generate the file to compare against that we manually skipped over
  setUpSkipTestFile(nToSkip, proto+infname, modeloutfname);

  try {
    URL uri(proto+infname);
    m_source = new CFileDataSource(uri, std::vector<uint16_t>());
    m_sink = new CFileDataSink(outfname);

    m_mediator = new CInfiniteMediator(0,0,0);
    m_mediator->setDataSource(m_source);
    m_mediator->setDataSink(m_sink);
    m_mediator->setFilter(new CTransparentFilter);

    m_mediator->setSkipCount(nToSkip);

    m_mediator->mainLoop();

    // kill all of the sinks and sources
    tearDown();
    // set up defaults so that we don't segfault at tearDown
    setUp();
  } catch (CException& exc) {
    std::stringstream errmsg; errmsg << "Caught exception:" << exc.ReasonText();
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (int errcode) {
    std::stringstream errmsg; errmsg << "Caught integer " << errcode;
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (std::string errmsg) {
    CPPUNIT_FAIL(errmsg.c_str()); 
  }

  CPPUNIT_ASSERT( filesEqual(modeloutfname,outfname) );

  // cleanup 
  remove(outfname.c_str());
  remove(modeloutfname.c_str());
}

void CInfiniteMediatorTest::setUpSkipTestFile(int nToSkip, 
                                      std::string infname, 
                                      std::string ofname)
{
  URL uri(infname);
  CDataSource* source = new CFileDataSource(uri, std::vector<uint16_t>());
  CDataSink* sink = new CFileDataSink(ofname);
  CFilter* filt = new CTransparentFilter;
  CInfiniteMediator* med = new CInfiniteMediator(source,filt,sink);

  // Extract the number of events we want to skip
  // These will not end up in the sink
  for (int i=0; i<nToSkip; ++i) { 
    const CRingItem* item = source->getItem();
    if (item!=0) delete item;
  }

  // Then run the main loop
  med->mainLoop();

  // Cleanup (the source, sink, and filter are owned by the mediator)
  delete med;
}

void CInfiniteMediatorTest::testProcessSome()
{
  tearDown();
 
  int nToProcess=11;

  std::string proto("file://");
  std::string infname("./run-0000-00.evt");
  std::string outfname("./copy-run-0000-00.evt");

  try {
    URL uri(proto+infname);
    m_source = new CFileDataSource(uri, std::vector<uint16_t>());
    m_sink = new CFileDataSink(outfname);
    m_filter = new CTestFilter;

    m_mediator = new CInfiniteMediator(0,0,0);
    m_mediator->setDataSource(m_source);
    m_mediator->setDataSink(m_sink);
    m_mediator->setFilter(m_filter);

    m_mediator->setProcessCount(nToProcess);

    m_mediator->mainLoop();

    CPPUNIT_ASSERT_EQUAL(nToProcess, 
                         static_cast<CTestFilter*>(m_filter)->getNProcessed());

    // kill all of the sinks and sources
    tearDown();
    // set up defaults so that we don't segfault at tearDown
    setUp();
  } catch (CException& exc) {
    std::stringstream errmsg; errmsg << "Caught exception:" << exc.ReasonText();
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (int errcode) {
    std::stringstream errmsg; errmsg << "Caught integer " << errcode;
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (std::string errmsg) {
    CPPUNIT_FAIL(errmsg.c_str()); 
  }

  remove(outfname.c_str());

}

void CInfiniteMediatorTest::testTransparentMainLoop()
{
  tearDown();
  
  // Set up the mediator
  std::string proto("file://");
  std::string infname("./run-0000-00.evt");
  std::string outfname("./copy-run-0000-00.evt");

//  std::ifstream ifile (infname.c_str());
//  std::ofstream ofile (outfname.c_str());
//  m_source = new CIStreamDataSource(ifile);
//  m_sink = new COStreamDataSink(ofile);
  try {
    URL uri(proto+infname);
    m_source = new CFileDataSource(uri, std::vector<uint16_t>());
    m_sink = new CFileDataSink(outfname);
    m_filter = new CTransparentFilter;

    m_mediator = new CInfiniteMediator(0,0,0);
    m_mediator->setDataSource(m_source);
    m_mediator->setDataSink(m_sink);
    m_mediator->setFilter(m_filter);

    m_mediator->mainLoop();

    // kill all of the sinks and sources
    tearDown();
    // set up defaults so that we don't segfault at tearDown
    setUp();
  } catch (CException& exc) {
    std::stringstream errmsg; errmsg << "Caught exception:" << exc.ReasonText();
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (int errcode) {
    std::stringstream errmsg; errmsg << "Caught integer " << errcode;
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (std::string errmsg) {
    CPPUNIT_FAIL(errmsg.c_str()); 
  }

  CPPUNIT_ASSERT( filesEqual(infname,outfname) );

  remove(outfname.c_str());
}

void CInfiniteMediatorTest::testFilterReturnsNULL() 
{
  tearDown();
  
  // Set up the mediator
  std::string proto("file://");
  std::string infname("./run-0000-00.evt");
  std::string outfname("./copy2-run-0000-00.evt");

//  std::ifstream ifile (infname.c_str());
//  std::ofstream ofile (outfname.c_str());
//  m_source = new CIStreamDataSource(ifile);
//  m_sink = new COStreamDataSink(ofile);
  try {
    URL uri(proto+infname);
    m_source = new CFileDataSource(uri, std::vector<uint16_t>());
    m_sink = new CFileDataSink(outfname);
    m_filter = new CNullFilter;

    m_mediator = new CInfiniteMediator(0,0,0);
    m_mediator->setDataSource(m_source);
    m_mediator->setDataSink(m_sink);
    m_mediator->setFilter(m_filter);

    m_mediator->mainLoop();

    // kill all of the sinks and sources
    tearDown();
    // set up defaults so that we don't segfault at tearDown
    setUp();
  } catch (CException& exc) {
    std::stringstream errmsg; errmsg << "Caught exception:" << exc.ReasonText();
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (int errcode) {
    std::stringstream errmsg; errmsg << "Caught integer " << errcode;
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (std::string errmsg) {
    CPPUNIT_FAIL(errmsg.c_str()); 
  }

  struct stat st;
  stat(outfname.c_str(), &st);
  CPPUNIT_ASSERT_EQUAL( 0, int(st.st_size) );

  remove(outfname.c_str());
}


//void CInfiniteMediatorTest::testMultiSourceStates()
//{
//  tearDown();
//  CRingStateChangeItem  begin (BEGIN_RUN);
//  CRingStateChangeItem  end (END_RUN);
//  CDataSinkFactory factory;
//  CDataSink* sink = factory.makeSink("file://./test.evt");
//
//  sink->putItem(begin);
//  sink->putItem(begin);
//  sink->putItem(end);
//  sink->putItem(end);
//  delete sink;
//
//  CDataSinkFactory snkfactory;
//  CDataSource* source = CDataSourceFactory::makeSource("file://./test.evt",
//                                                        std::vector<uint16_t>(),
//                                                        std::vector<uint16_t>());
//  sink = snkfactory.makeSink("-");
//  
//  CTestFilter* filt = new CTestFilter();
//  m_mediator->setFilter(filt);
//  m_mediator->setDataSource(source);
//  m_mediator->setDataSink(sink);
//
//  m_mediator->mainLoop();
//  
//  // If we got here we succeeded...
//  // Make sure we processed all of the events
//  CPPUNIT_ASSERT( 4 == filt->getNProcessed() );
//
//  tearDown();
//  setUp();
//}
//
//

bool CInfiniteMediatorTest::filesEqual(std::string fname0, std::string fname1)
{
  std::ifstream filein(fname0.c_str());
  std::ifstream fileout(fname1.c_str());

  std::istream_iterator<char> end_of_stream;
  std::istream_iterator<char> in_iter(filein);
  std::istream_iterator<char> out_iter(fileout);

  return std::equal(in_iter,end_of_stream,out_iter);

}


size_t CInfiniteMediatorTest::writeRingItemToFile(CRingItem& item, 
        std::string fname,
                                                std::ios::openmode mode)
{
    RingItem* pItem = item.getItemPointer();
    std::ofstream ofile(fname.c_str(), mode);
    size_t nbytes = pItem->s_header.s_size;
    char* pBuffer = reinterpret_cast<char*>(pItem);
    ofile.write(pBuffer,nbytes);
    ofile.close();

    return nbytes;
}


void CInfiniteMediatorTest::initialize_0 () 
{
  CTestFilter* filt = dynamic_cast<CTestFilter*>(m_filter);
  CPPUNIT_ASSERT_EQUAL(false, filt->m_initCalled );
  m_mediator->initialize();
  CPPUNIT_ASSERT_EQUAL(true, filt->m_initCalled );
}

void CInfiniteMediatorTest::finalize_0 () 
{
  CTestFilter* filt = dynamic_cast<CTestFilter*>(m_filter);
  CPPUNIT_ASSERT_EQUAL(false, filt->m_finalCalled );
  m_mediator->finalize();
  CPPUNIT_ASSERT_EQUAL(true, filt->m_finalCalled );
}


