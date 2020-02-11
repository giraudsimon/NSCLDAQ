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

#include <fstream>
#include <CPhysicsEventItem.h>
#include <cppunit/extensions/HelperMacros.h>

#include <ios>
#include <algorithm>
#include <CRingStateChangeItem.h>
#include <CRingItemFactory.h>
#include <CPhysicsEventItem.h>
#include <CRingScalerItem.h>
#include <CFileDataSource.h>
#include <RingItemComparisons.h>
#include <fstream>
#include <string>
#include <fcntl.h>
#include <errno.h>
#include <URL.h>
#include <unistd.h>

#define private public

#include "CFileDataSink.h"


// A test suite 
class CFileDataSinkTest : public CppUnit::TestFixture
{
    private:
    CPhysicsEventItem m_item;

    public:
    CFileDataSinkTest();

    CPPUNIT_TEST_SUITE( CFileDataSinkTest );
//    CPPUNIT_TEST ( testReadOrThrow );
    CPPUNIT_TEST ( testConstructor1 );
    CPPUNIT_TEST ( testConstructor2 );
    CPPUNIT_TEST ( testConstructor3 );
    CPPUNIT_TEST ( testConstructor4 );
    CPPUNIT_TEST ( testPutItem );
    CPPUNIT_TEST ( testPut);
    CPPUNIT_TEST_SUITE_END();

    public:
    void setUp();
    void tearDown();

    void testConstructor1();
    void testConstructor2();
    void testConstructor3();
    void testConstructor4();

    void testPutItem();
    void testPut();

};



// Register it with the test factory
CPPUNIT_TEST_SUITE_REGISTRATION( CFileDataSinkTest );

CFileDataSinkTest::CFileDataSinkTest()
  : m_item(8192)
{}


void CFileDataSinkTest::setUp()
{
    // create a ring item and fill it
    m_item = CPhysicsEventItem();    
    uint16_t* pCursor = reinterpret_cast<uint16_t*>(m_item.getBodyCursor());
    for (int i=0; i<10; i++) {
        *pCursor = i;
        pCursor++;
    }
    m_item.setBodyCursor(pCursor);
    m_item.updateSize();
}

// Kill the temp files:

void CFileDataSinkTest::tearDown()
{
    
    // Ignore errors because different tests make different test file
    
    unlink("./dummy");
    unlink("./testOutFile0.bin");
}


//-------------------------------
// test stdout fd ctor
void CFileDataSinkTest::testConstructor1()
{
  CFileDataSink sink(STDOUT_FILENO);
  CPPUNIT_ASSERT( STDOUT_FILENO == sink.m_fd );    
} // test fd


//-------------------------------
// test stdout string ctor
void CFileDataSinkTest::testConstructor2()
{
  CFileDataSink sink("stdout");
  CPPUNIT_ASSERT( STDOUT_FILENO != sink.m_fd );    
} // end test string ctor


//-------------------------------
// test fails b/c not write only 
void CFileDataSinkTest::testConstructor3()
{
  int fd = open("./dummy",O_RDONLY|O_CREAT, S_IWUSR|S_IRUSR);
  if (fd<0) {
    std::cout << "after open...errno " << errno << std::endl;
  }
  CPPUNIT_ASSERT( fd >= 0 );

  bool caught = false;
  try {
    CFileDataSink sink(fd);
  } catch (std::string) {
    caught = true; 
  } 

  CPPUNIT_ASSERT(caught);

} // end test fail


//-------------------------------
// test closes fd
void CFileDataSinkTest::testConstructor4()
{
  int fd = -1;
  // Enclose the ctor in a block to make sure
  // the FileDataSink goes out of scope
  {
    try {
      CFileDataSink sink(std::string("dummy"));

      // reach in and store the fd
      fd = sink.m_fd;

    } catch (CException& err) {
      std::cout << err.ReasonText() << std::endl;
      CPPUNIT_ASSERT( false );
    }
    // this should go out of scope and close 
    // the file
  }
  // check that this is a bad file descriptor
  // when I try to work with it
  int status = fcntl(fd,F_GETFD);
  CPPUNIT_ASSERT( -1 == status  );
  CPPUNIT_ASSERT( EBADF == errno );
}

void CFileDataSinkTest::testPutItem()
{
    using namespace std;

    std::string fname = "./testOutFile0.bin";
    {
      CFileDataSink sink(fname);
      sink.putItem(m_item);
      sink.flush();
    }
    
    std::vector<uint16_t> dummy;
    URL uri(string("file://") + fname);
    
    CFileDataSource source(uri, dummy);

    CRingItem* new_item = source.getItem();

    CPPUNIT_ASSERT( *m_item.getItemPointer() == *new_item->getItemPointer() );
}

void CFileDataSinkTest::testPut()
{
    std::string fname="./testOutFile0.bin";
    const char* pData = "This is a test";
    {
        CFileDataSink sink(fname);
        
        sink.put(pData, strlen(pData)+1);    /// string w null terminator.
    }
    int fd = open(fname.c_str(), O_RDONLY );
    CPPUNIT_ASSERT(fd != -1);                // File must be made:
    
    char buffer[1000];
    size_t nBytes = read(fd, buffer, strlen(pData) +1);
    CPPUNIT_ASSERT_EQUAL(strlen(pData)+1, nBytes);
    CPPUNIT_ASSERT_EQUAL(0, strcmp(pData, buffer));
    
    close(fd);
    
    
}
