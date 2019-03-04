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


static const char* Copyright = "(C) Copyright Michigan State University 2014, All rights reserved";
#include <fstream>
#include <cppunit/extensions/HelperMacros.h>

#include <URIFormatException.h>
#include <Asserts.h>
#include <string>
#include <stdexcept>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <wait.h>

#define private public

#include "CDataSinkFactory.h"
#include <CFileDataSink.h>
#include <CRingBuffer.h>

using namespace std;

// A test suite 
class CDataSinkFactoryTest : public CppUnit::TestFixture
{
    public:
    CDataSinkFactoryTest();

    CPPUNIT_TEST_SUITE( CDataSinkFactoryTest );
    CPPUNIT_TEST ( testStdoutDash );
//    CPPUNIT_TEST ( testFailOnStdin );
    CPPUNIT_TEST ( testRingSink );
    CPPUNIT_TEST ( testTCPRingSink );
//    CPPUNIT_TEST ( testFailRingSink );
    CPPUNIT_TEST (testRemoteFail);
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp();
    void tearDown();

    void testStdout();
    void testStdoutDash();
//    void testFailOnStdin();
    void testRingSink();
    void testTCPRingSink();
    void testFailRingSink();
    void testRemoteFail();

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CDataSinkFactoryTest );

CDataSinkFactoryTest::CDataSinkFactoryTest()
{}


void CDataSinkFactoryTest::setUp()
{}

void CDataSinkFactoryTest::tearDown()
{}

void CDataSinkFactoryTest::testStdoutDash()
{
  CDataSinkFactory factory;
  CFileDataSink* sink = dynamic_cast<CFileDataSink*>(factory.makeSink("-"));
  CPPUNIT_ASSERT ( 0 != sink );
  CPPUNIT_ASSERT_EQUAL( STDOUT_FILENO, sink->m_fd );
  delete sink;
}

//void CDataSinkFactoryTest::testFailOnStdin()
//{
//  CDataSinkFactory factory;
//  CDataSink* sink=0;
//
//  CPPUNIT_ASSERT_THROW( sink = factory.makeSink("file:///stdin"), CErrnoException );
////  try {
////
////sink = factory.makeSink("file:///stdin");
////
////  } catch (int err) {
////    std::stringstream errmsg; errmsg << "errno = " << err << std::endl;
////    CPPUNIT_FAIL(errmsg.str().c_str());
////
////  } catch (CURIFormatException& exc) {
////    CPPUNIT_FAIL(exc.ReasonText());
////  }
//
//  if (sink!=0) { delete sink; }
//}
//
void CDataSinkFactoryTest::testRingSink()
{
  CDataSinkFactory factory;
  CDataSink* sink;
  try {
    sink = factory.makeSink("ring://localhost/myring");
  } catch (int err) {
    std::stringstream errmsg; errmsg << "errno = " << err << std::endl;
    CPPUNIT_FAIL( errmsg.str().c_str() );
  } catch (CURIFormatException& exc) {
    CPPUNIT_FAIL( exc.ReasonText() );
  }


  if (sink!=0) { delete sink; }
}

// Note that this will not fail b/c the single producer requirement is a 
// process-level requirement.
void CDataSinkFactoryTest::testFailRingSink()
{
  CRingBuffer *ring = 0;
  std::string rname = "test";
  if (CRingBuffer::isRing(rname)) {
    ring = new CRingBuffer(rname, CRingBuffer::producer);
  } else {
    ring = CRingBuffer::createAndProduce(rname);
  }
  CPPUNIT_ASSERT(ring != 0);

  std::string rsink="ring://localhost/";
  rsink += rname;

  // fork this to make a new process

  pid_t pid = fork();
  // if the pid is 0 then it is the child
  if (! pid) {
    // child process
    CDataSinkFactory factory;
    CDataSink* sink = 0;

    cout << "here" << endl;
    CPPUNIT_ASSERT_THROW( sink=factory.makeSink(rsink), CException );

    CPPUNIT_ASSERT( 0 == sink );

  } else {
    // Parent process

    // give child 200 seconds to complete (this so it doesn't alarm while I am debugging)
    alarm(200);
    int status=0;
    if (waitpid(pid, &status, 0)==pid) {
      alarm(0);
    } else {
      alarm(0);
      kill(pid,SIGTERM);
    }

//    CRingBuffer::remove(rname);
  }
}

void CDataSinkFactoryTest::testTCPRingSink()
{
  CDataSinkFactory factory;
  CDataSink* sink;
  try {
    sink = factory.makeSink("tcp://localhost/myring");
  } catch (int err) {
    std::stringstream errmsg; errmsg << "errno = " << err << std::endl;
    CPPUNIT_FAIL( errmsg.str().c_str() );
  } catch (CURIFormatException& exc) {
    CPPUNIT_FAIL( exc.ReasonText() );
  }


  if (sink!=0) { delete sink; }
}

// Using any node other than empty or localhost shoudl fail

void CDataSinkFactoryTest::testRemoteFail()
{
    CDataSinkFactory factory;
    CDataSink*       sink;
    bool threw = false;

    try {
        sink = factory.makeSink("ring://www.google.com/somering");     // must fail.    
    }
    catch(CErrnoException& e) {
        threw = true;
        EQ(EREMOTE, e.ReasonCode());
    }
    ASSERT(threw);
    
    threw = false;
    try {
        sink = factory.makeSink("tcp://www.google.com/somering");     // Must fail.    
    }
    catch(CErrnoException& e) {
        threw = true;
        EQ(EREMOTE, e.ReasonCode());
    }
    ASSERT(threw);
    
    
}
