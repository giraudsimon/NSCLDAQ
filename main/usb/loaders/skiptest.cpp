/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/

/**
 * @file skiptest.cpp
 * @brief unit test for the skipHeader function.
 */

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>

#include "Asserts.h"
#include "skipHeader.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <arpa/inet.h>


static const char* pBitFile = SRCDIR "ccusb_0603_123117.bit";
static const char* pBinFile = SRCDIR "vmeusb_0A00_031913.bin";
static const char* pOldBitFile = SRCDIR "ccusb_0600_072012.bit";

static const size_t MEGABYTE(1024*1024);

class SkipTests : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(SkipTests);
    CPPUNIT_TEST(skip);
    CPPUNIT_TEST(skipbin);
    CPPUNIT_TEST(skipold);
    CPPUNIT_TEST_SUITE_END();
    
protected:
    void skip();
    void skipbin();
    void skipold();
};
CPPUNIT_TEST_SUITE_REGISTRATION(SkipTests);

 
void SkipTests::skip()
{
    
    // Have to be able to open the test bit file
    
    int fd = open(pBitFile, O_RDONLY);
    ASSERT(fd >= 0);
    
    // A megabyte should be enough of a read:
    
    char configFile[MEGABYTE];
    ssize_t nRead = read(fd, configFile, MEGABYTE);
    ASSERT(nRead > 0);
    
    uint32_t* pBody = reinterpret_cast<uint32_t*>(skipHeader(configFile));
    close(fd);
    
    ASSERT(pBody);
    
    EQ((uint32_t)0xffffffff,*pBody);
    EQ((uint32_t)ntohl(0xaa995566), pBody[1]);
    
    
}
// Check that skiptest works on a bin file

void SkipTests::skipbin()
{
    
    // Have to be able to open the test .bin file
    
    int fd = open(pBinFile, O_RDONLY);
    ASSERT(fd >= 0);
    
    // A megabyte should be enough of a read:
    
    char configFile[MEGABYTE];
    ssize_t nRead = read(fd, configFile, MEGABYTE);
    ASSERT(nRead > 0);
    
    uint32_t* pBody = reinterpret_cast<uint32_t*>(skipHeader(configFile));
    close(fd);
    
    ASSERT(pBody);
    
    EQ((uint32_t)0xffffffff,*pBody);
    EQ((uint32_t)ntohl(0xaa995566), pBody[1]);
}

// Check that we can skip old files as well:

void SkipTests::skipold()
{
    
    // Have to be able to open the test .bin file
    
    int fd = open(pOldBitFile, O_RDONLY);
    ASSERT(fd >= 0);
    
    // A megabyte should be enough of a read:
    
    char configFile[MEGABYTE];
    ssize_t nRead = read(fd, configFile, MEGABYTE);
    ASSERT(nRead > 0);
    
    uint32_t* pBody = reinterpret_cast<uint32_t*>(skipHeader(configFile));
    close(fd);
    
    ASSERT(pBody);
    
    EQ((uint32_t)0xffffffff,*pBody);
    EQ((uint32_t)ntohl(0xaa995566), pBody[1]);
}
