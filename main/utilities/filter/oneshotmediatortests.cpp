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
#include <sys/types.h>
#include <sys/stat.h>

#include <URL.h>

#include <CRingStateChangeItem.h>
#include <CDataFormatItem.h>

#include <CFileDataSource.h>
#include <CFileDataSink.h>
#include <CDataSinkFactory.h>
#include <CDataSourceFactory.h>

#include "CTransparentFilter.h"
#include "CNullFilter.h"
#include "CTestFilter.h"
#include <CMediatorException.h>

#include <cppunit/extensions/HelperMacros.h>

#define private public
#define protected public
#include "COneShotMediator.h"
#undef private
#undef protected

using namespace std;

// A test suite 
class COneShotMediatorTest : public CppUnit::TestFixture
{

  private:
    CFilter* m_filter;
    CDataSource* m_source;
    CDataSink* m_sink;
    COneShotMediator* m_mediator;

  public:
    COneShotMediatorTest();

    CPPUNIT_TEST_SUITE( COneShotMediatorTest );
    CPPUNIT_TEST ( testWaitForBegin_0 );
    CPPUNIT_TEST ( initialize_0 );
    CPPUNIT_TEST ( finalize_0 );
    CPPUNIT_TEST ( abnormalEndRun_0 );
    CPPUNIT_TEST_SUITE_END();

  public:
    void setUp();
    void tearDown();

    void testWaitForBegin_0();
    void initialize_0();
    void finalize_0();
    void abnormalEndRun_0();

  private:
    size_t writeRingItemToFile(CRingItem& item,
        string fname,
        ios::openmode mode);


    bool filesEqual(string fname0, string fname1);
    bool compareEqual(CRingItem& item0, CRingItem& item1);
    void setUpSkipTestFile(int nToSkip, 
                           string infname, 
                           string ofname);

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( COneShotMediatorTest );

COneShotMediatorTest::COneShotMediatorTest()
    : m_filter(0),
    m_source(0),
    m_sink(0),
    m_mediator(0)
{}


void COneShotMediatorTest::setUp()
{
  vector<uint16_t> dummy;
  m_filter = new CTestFilter;
  m_source = new CFileDataSource(STDIN_FILENO, dummy);
  m_sink = new CFileDataSink(STDOUT_FILENO);

  m_mediator = new COneShotMediator(m_source,m_filter,m_sink);
  m_mediator->setDataSource(m_source);
  m_mediator->setFilter(m_filter);
  m_mediator->setDataSink(m_sink);
}

void COneShotMediatorTest::tearDown()
{
  // Call the destructor to free
  // owned memory
  delete m_mediator; m_mediator=0;
}

void COneShotMediatorTest::testWaitForBegin_0() 
{
  tearDown();
  

  // Set up the mediator
  string proto("file://");
  string infname("./run-0001-00.evt");
  string outfname("./copy2-run-0000-00.evt");

  // create the test file in a block so that it gets destroyed by 
  // going out of scope
  {
    CFileDataSink test_infile(infname);
    test_infile.putItem(CDataFormatItem());
    test_infile.putItem(CRingStateChangeItem(BEGIN_RUN));
    test_infile.putItem(CRingStateChangeItem(END_RUN));
  }


  try {
    URL uri(proto+infname);
    m_source = new CFileDataSource(uri, vector<uint16_t>());
    m_sink = new CFileDataSink(outfname);
    m_filter = new CTransparentFilter;

    m_mediator = new COneShotMediator(m_source, m_filter, m_sink);
    m_mediator->mainLoop();

    // kill all of the sinks and sources
    tearDown();
    // set up defaults so that we don't segfault at tearDown
    setUp();
  } catch (CException& exc) {
    stringstream errmsg; errmsg << "Caught exception:" << exc.ReasonText();
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (int errcode) {
    stringstream errmsg; errmsg << "Caught integer " << errcode;
    CPPUNIT_FAIL(errmsg.str().c_str()); 
  } catch (string errmsg) {
    CPPUNIT_FAIL(errmsg.c_str()); 
  }

  CPPUNIT_ASSERT(filesEqual(infname, outfname));

  // cleanup
  remove(infname.c_str());
  remove(outfname.c_str());
}


bool COneShotMediatorTest::filesEqual(string fname0, string fname1)
{
  ifstream filein(fname0.c_str());
  ifstream fileout(fname1.c_str());

  istream_iterator<char> end_of_stream;
  istream_iterator<char> in_iter(filein);
  istream_iterator<char> out_iter(fileout);

//  cout << "start infile" << endl;
//  copy(in_iter, end_of_stream, ostream_iterator<char>(cout,"\n"));
//  cout << "end infile" << endl;

  return equal(in_iter,end_of_stream,out_iter);

}


void COneShotMediatorTest::initialize_0() 
{ 
  CTestFilter* filt = dynamic_cast<CTestFilter*>(m_filter);
  CPPUNIT_ASSERT_EQUAL(false, filt->m_initCalled );
  m_mediator->initialize();
  CPPUNIT_ASSERT_EQUAL(true, filt->m_initCalled );
}

void COneShotMediatorTest::finalize_0() 
{ 
  CTestFilter* filt = dynamic_cast<CTestFilter*>(m_filter);
  CPPUNIT_ASSERT_EQUAL(false, filt->m_finalCalled );
  m_mediator->finalize();
  CPPUNIT_ASSERT_EQUAL(true, filt->m_finalCalled );

}

void COneShotMediatorTest::abnormalEndRun_0 () {

  tearDown();

  // Set up the mediator
  string proto("file://");
  string infname("./run-0001-00.evt");
  string outfname("./copy2-run-0000-00.evt");
  string expectedResultFname("./copy2-run-0000-00.exp.evt");

  // create the test file in a block so that it gets destroyed by 
  // going out of scope
  {
    CFileDataSink test_infile(infname);
    test_infile.putItem(CRingStateChangeItem(BEGIN_RUN));
    test_infile.putItem(CRingStateChangeItem(ABNORMAL_ENDRUN));
    test_infile.putItem(CDataFormatItem());
  }

  {
    CFileDataSink exp_file(expectedResultFname);
    exp_file.putItem(CRingStateChangeItem(BEGIN_RUN));
    exp_file.putItem(CRingStateChangeItem(ABNORMAL_ENDRUN));
    // no data format item
  }

  URL uri(proto+infname);
  m_source = new CFileDataSource(uri, vector<uint16_t>());
  m_sink = new CFileDataSink(outfname);
  m_filter = new CTransparentFilter;

  m_mediator = new COneShotMediator(m_source, m_filter, m_sink);
  
  CPPUNIT_ASSERT_NO_THROW(m_mediator->mainLoop());

  CPPUNIT_ASSERT(filesEqual(expectedResultFname, outfname));

  // cleanup
  remove(infname.c_str());
  remove(outfname.c_str());
  remove(expectedResultFname.c_str());
}
