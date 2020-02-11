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
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <CRingBuffer.h>

#define private public
#define protected public

#include "CDataSourceFactory.h"
#include "CFileDataSource.h"
#include "CRingDataSource.h"

#undef private
#undef protected


// A test suite 
class CDataSourceFactoryTest : public CppUnit::TestFixture
{
    public:
    CDataSourceFactoryTest();

    CPPUNIT_TEST_SUITE( CDataSourceFactoryTest );
    CPPUNIT_TEST ( testStdin );
    CPPUNIT_TEST ( testGoodFileSource );
    CPPUNIT_TEST ( testBadFileSource );
    CPPUNIT_TEST ( testRingSource );
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp();
    void tearDown();

    void testStdin();
    void testGoodFileSource();
    void testBadFileSource();
    void testRingSource();

};


// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CDataSourceFactoryTest );

CDataSourceFactoryTest::CDataSourceFactoryTest()
{}


void CDataSourceFactoryTest::setUp()
{}

void CDataSourceFactoryTest::tearDown()
{}

void CDataSourceFactoryTest::testStdin()
{
  CDataSourceFactory factory;

  CFileDataSource* sink = 
      dynamic_cast<CFileDataSource*>(factory.makeSource("-", 
                                                        std::vector<uint16_t>(), 
                                                        std::vector<uint16_t>()));
  CPPUNIT_ASSERT ( 0 != sink );
  CPPUNIT_ASSERT_EQUAL( STDIN_FILENO, sink->m_fd );

  delete sink;
}

void CDataSourceFactoryTest::testGoodFileSource()
{
  CDataSourceFactory factory;

  std::ofstream f("./blah");
  f.close();

  CFileDataSource* sink = 0;
  sink  = dynamic_cast<CFileDataSource*>(factory.makeSource("file://./blah",
                                                            std::vector<uint16_t>(), 
                                                            std::vector<uint16_t>())
                                        );
  CPPUNIT_ASSERT ( 0 != sink );
  delete sink;
}

void CDataSourceFactoryTest::testBadFileSource()
{
  CDataSourceFactory factory;

  CPPUNIT_ASSERT_THROW( factory.makeSource("file://./nofile", 
                                           std::vector<uint16_t>(), 
                                           std::vector<uint16_t>()),
                        CException );
}

void CDataSourceFactoryTest::testRingSource()
{
  // Ensure the ring exists:
  try {
    CRingBuffer::create("myring");
  } catch(...) {}
  //
  CDataSourceFactory factory;
  CDataSource* sink;
  try {
    sink = factory.makeSource("ring://localhost/myring", 
                              std::vector<uint16_t>(), 
                              std::vector<uint16_t>());
  } catch (int err) {
    std::stringstream errmsg; errmsg << "errno = " << err << std::endl;
    CPPUNIT_FAIL( errmsg.str().c_str() );
  } catch (CURIFormatException& exc) {
    CPPUNIT_FAIL( exc.ReasonText() );
  }


  if (sink!=0) { delete sink; }
}
