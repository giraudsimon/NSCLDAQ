/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
             NSCL
             Michigan State University
             East Lansing, MI 48824-1321
*/



#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>



#include <V8/DataFormatV8.h>
#include <V8/CRawBuffer.h>
#include <ByteBuffer.h>
#include <DebugUtils.h>
#include <V8/ChangeBufferSize.h>

#define private public
#define protected public
#include <V8/CPhysicsEventBuffer.h>
#undef protected
#undef private

#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;

using namespace DAQ::V8;
using namespace DAQ::Buffer;

class physicseventtest : public CppUnit::TestFixture {
protected:
  bheader m_header;
  std::vector<uint16_t> m_bodyData;
  CPhysicsEventBuffer m_physicsBuffer;

public:
  CPPUNIT_TEST_SUITE(physicseventtest);
  CPPUNIT_TEST(totalShorts_0);
  CPPUNIT_TEST(ctor_0);
  CPPUNIT_TEST(copyCtor_0);
  CPPUNIT_TEST(copyCtor_1);
  CPPUNIT_TEST(copyCtor_2);
  CPPUNIT_TEST(unsupportedVersion_0);
  CPPUNIT_TEST(rawBufferCtor_0);
  CPPUNIT_TEST(rawBufferCtor_1);
  CPPUNIT_TEST(rawBufferCtor_2);
  CPPUNIT_TEST(rawBufferCtor_3);
  CPPUNIT_TEST(rawBufferCtor_4);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(toRawBuffer_1);
  CPPUNIT_TEST_SUITE_END();

public:
  virtual void setUp() {


    m_header.nwds = 21;
    m_header.type = DAQ::V8::DATABF;
    m_header.cks  = 0;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = 2;
    m_header.nlam = 0;
    m_header.cpu  = 0;
    m_header.nbit = 0;
    m_header.buffmt = DAQ::V8::StandardVsn;
    m_header.ssignature = DAQ::V8::BOM16;
    m_header.lsignature = DAQ::V8::BOM32;

    m_bodyData = std::vector<uint16_t>({3, 0, 1,
                                       2, 3});
    m_physicsBuffer = CPhysicsEventBuffer(m_header, m_bodyData);

  }

  virtual void tearDown()
  {
  }

void totalShorts_0() {
  std::vector<uint16_t> data = {2, 3};
  ByteBuffer buffer;
  buffer << data;

  const CPhysicsEvent& event = CPhysicsEvent(buffer, false);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Total size for standard buffer makes sense",
                               std::size_t(2), event.getNTotalShorts());

}

void ctor_0() {

  CPPUNIT_ASSERT_EQUAL_MESSAGE("Buffer should be parsed nevt number of times",
                               std::size_t(2), m_physicsBuffer.size());

}

void copyCtor_0 () {
  CPhysicsEventBuffer buffer(m_physicsBuffer);
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of events in body is preserved during copy",
                               buffer.size(), m_physicsBuffer.size());
}
void copyCtor_1 () {
  CPhysicsEventBuffer buffer(m_physicsBuffer);
  CPPUNIT_ASSERT_MESSAGE("Data of first event is preserved during copy",
                               std::equal(buffer.at(0)->begin(), buffer.at(0)->end(),
                                          m_physicsBuffer.at(0)->begin()));
}
void copyCtor_2 () {
  CPhysicsEventBuffer buffer(m_physicsBuffer);
  CPPUNIT_ASSERT_MESSAGE("Data of second event is preserved during copy",
                               std::equal(buffer.at(1)->begin(), buffer.at(1)->end(),
                                          m_physicsBuffer.at(1)->begin()));
}

void unsupportedVersion_0() {
  m_header.buffmt = DAQ::V8::JumboVsn;

  CPPUNIT_ASSERT_THROW_MESSAGE("Parsing jumbo buffers is a failure",
                               m_physicsBuffer = CPhysicsEventBuffer(m_header, m_bodyData),
                               std::runtime_error);

}


CRawBuffer createRawBuffer() {

  CRawBuffer rawBuf;

  DAQ::Buffer::ByteBuffer buffer;
  buffer << m_header;
  buffer << m_bodyData;

  rawBuf.setBuffer(buffer);

  return rawBuf;
}

void rawBufferCtor_0() {
  CPhysicsEventBuffer physBuf(createRawBuffer());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("Construct from a raw buffer",
                               std::size_t(2), physBuf.size());
}


void rawBufferCtor_1() {

  // because we are building our raw buffer from the same data that m_physicsBuffer was
  // constructed from, the contents better be the same.
  CPhysicsEventBuffer physBuf(createRawBuffer());
  CPPUNIT_ASSERT_MESSAGE("RawBuffer ctor has proper 1st event",
                         std::equal(physBuf.at(0)->begin(), physBuf.at(0)->end(),
                                    m_physicsBuffer.at(0)->begin()));
}

void rawBufferCtor_2() {

  // because we are building our raw buffer from the same data that m_physicsBuffer was
  // constructed from, the contents better be the same.
  CPhysicsEventBuffer physBuf(createRawBuffer());
  CPPUNIT_ASSERT_MESSAGE("RawBuffer ctor has proper 2nd event",
                         std::equal(physBuf.at(1)->begin(), physBuf.at(1)->end(),
                                    m_physicsBuffer.at(1)->begin()));
}

void rawBufferCtor_3 () {
  m_header.type = SCALERBF; // poison the buffer type to be something we cannot construct from

  CRawBuffer rawBuf = createRawBuffer();

  CPPUNIT_ASSERT_THROW_MESSAGE("RawBuffer ctor throws if not of type DATABF",
                               CPhysicsEventBuffer physBuf(rawBuf),
                                std::runtime_error);
}


virtual DAQ::Buffer::ByteBuffer createSwappedEvent()
{
  bheader header;
  header.nwds = 0x1500;
  header.type = 0x0100;
  header.nevt = 0x0200;
  header.buffmt = 0x0500;
  header.ssignature = 0x0201;
  header.lsignature = 0x04030201;
  header.cks = 0;
  header.cpu = 0;
  header.nbit = 0;
  header.nlam = 0;
  header.run = 0x0100;
  header.seq = 0x02000000;
  header.unused[0] = 0;
  header.unused[1] = 0;

  std::vector<std::uint16_t> data({0x0300, 0x0000, 0x0100,
                                   0x0200, 0x0300});


  DAQ::Buffer::ByteBuffer buffer;
  buffer << header;
  buffer << data;
  return buffer;
}

// construct from a byte swapped buffer produces the correct result
void rawBufferCtor_4() {

  DAQ::Buffer::ByteBuffer buffer = createSwappedEvent();

  CRawBuffer rawBuf(8192);
  rawBuf.setBuffer(buffer);

  CPhysicsEventBuffer physBuf(rawBuf);

  CPPUNIT_ASSERT_EQUAL_MESSAGE("raw ctor swaps header properly",
                         m_header,  physBuf.getHeader());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("raw ctor specifies properly whether data needs a swap",
                               true, physBuf.at(0)->dataNeedsSwap());
  CPPUNIT_ASSERT_EQUAL_MESSAGE("raw ctor specifies properly whether data needs a swap",
                               true, physBuf.at(1)->dataNeedsSwap());
  }

  // original byte order is maintained for PhysicsEvents
  void toRawBuffer_0() {

    DAQ::Buffer::ByteBuffer origBuffer = createSwappedEvent();

    CRawBuffer rawBuf;
    rawBuf.setBuffer(origBuffer);

    // construct from a buffer that needs swapping
    CPhysicsEventBuffer physBuf(rawBuf);

    // fill a CRawBuffer with it.
    CRawBuffer newBuf;
    physBuf.toRawBuffer(newBuf);

    origBuffer.resize(gBufferSize); // make sure that the origBuffer is a full buffer size, b/c
                             // CPhysicsEventBuffer::toRawBuffer will make sure that
                             // the raw buffer is the appropriate size.

    CPPUNIT_ASSERT_EQUAL_MESSAGE("physics buffer is returned to raw buffer unswapped",
                                 origBuffer, newBuf.getBuffer());
    }

   void toRawBuffer_1() {

    DAQ::Buffer::ByteBuffer origBuffer = createSwappedEvent();

    CRawBuffer rawBuf(8192);
    rawBuf.setBuffer(origBuffer);

    // in case this fails, we want to change back the buffer size to what it was.
    // Note that this must follow rawBuf.setBuffer() as it does, because setBuffer()
    // will resize the buffer to gBufferSize and destroy the data.
    ::DAQ::V8::Test::ChangeBufferSize newSize(1);

    // construct from a buffer that needs swapping
    CPhysicsEventBuffer physBuf(rawBuf);

    CPPUNIT_ASSERT_THROW_MESSAGE("Throws exception if cannot fit into buffer",
                                 physBuf.toRawBuffer(rawBuf),
                                 std::runtime_error);
   }

};

CPPUNIT_TEST_SUITE_REGISTRATION(physicseventtest);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


class physicseventtest_Incl16Word : public physicseventtest {

public:
  CPPUNIT_TEST_SUITE(physicseventtest_Incl16Word);
  CPPUNIT_TEST(totalShorts_0);
  CPPUNIT_TEST(ctor_0);
  CPPUNIT_TEST(copyCtor_0);
  CPPUNIT_TEST(copyCtor_1);
  CPPUNIT_TEST(copyCtor_2);
  CPPUNIT_TEST(rawBufferCtor_0);
  CPPUNIT_TEST(rawBufferCtor_1);
  CPPUNIT_TEST(rawBufferCtor_2);
  CPPUNIT_TEST(rawBufferCtor_3);
  CPPUNIT_TEST(rawBufferCtor_4);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(toRawBuffer_1);
  CPPUNIT_TEST_SUITE_END();

public:
  virtual void setUp() {

    CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::Inclusive16BitWords;

    physicseventtest::setUp();
  }

  virtual void tearDown()
  {
    CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::BufferPreference;
    physicseventtest::tearDown();
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(physicseventtest_Incl16Word);


class physicseventtest_Excl16Word : public physicseventtest {

public:
  CPPUNIT_TEST_SUITE(physicseventtest_Excl16Word);
  CPPUNIT_TEST(totalShorts_1);
  CPPUNIT_TEST(ctor_0);
  CPPUNIT_TEST(copyCtor_0);
  CPPUNIT_TEST(copyCtor_1);
  CPPUNIT_TEST(copyCtor_2);
  CPPUNIT_TEST(rawBufferCtor_0);
  CPPUNIT_TEST(rawBufferCtor_1);
  CPPUNIT_TEST(rawBufferCtor_2);
  CPPUNIT_TEST(rawBufferCtor_3);
  CPPUNIT_TEST(rawBufferCtor_4);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(toRawBuffer_1);
  CPPUNIT_TEST_SUITE_END();

public:
  virtual void setUp() {

    CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::Exclusive16BitWords;

    m_header.nwds = 21;
    m_header.type = DAQ::V8::DATABF;
    m_header.cks  = 0;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = 2;
    m_header.nlam = 0;
    m_header.cpu  = 0;
    m_header.nbit = 0;
    m_header.buffmt = DAQ::V8::StandardVsn;
    m_header.ssignature = DAQ::V8::BOM16;
    m_header.lsignature = DAQ::V8::BOM32;

    m_bodyData = std::vector<uint16_t>({2, 0, 1,
                                       1, 3});
    m_physicsBuffer = CPhysicsEventBuffer(m_header, m_bodyData);

  }

  virtual void tearDown()
  {
    CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::BufferPreference;
  }

  void totalShorts_1() {
    std::vector<uint16_t> data = {1, 3};
    ByteBuffer buffer;
    buffer << data;

    const CPhysicsEvent& event = CPhysicsEvent(buffer, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Total size for standard buffer makes sense",
                                 std::size_t(2), event.getNTotalShorts());

  }


  virtual DAQ::Buffer::ByteBuffer createSwappedEvent()
  {
    bheader header;
    header.nwds = 0x1500;
    header.type = 0x0100;
    header.nevt = 0x0200;
    header.buffmt = 0x0500;
    header.ssignature = 0x0201;
    header.lsignature = 0x04030201;
    header.cks = 0;
    header.cpu = 0;
    header.nbit = 0;
    header.nlam = 0;
    header.run = 0x0100;
    header.seq = 0x02000000;
    header.unused[0] = 0;
    header.unused[1] = 0;

    std::vector<std::uint16_t> data({0x0200, 0x0000, 0x0100,
                                     0x0100, 0x0300});


    DAQ::Buffer::ByteBuffer buffer;
    buffer << header;
    buffer << data;
    return buffer;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(physicseventtest_Excl16Word);


class physicseventtest_Incl32Word : public physicseventtest {

public:
  CPPUNIT_TEST_SUITE(physicseventtest_Incl32Word);
  CPPUNIT_TEST(totalShorts_1);
  CPPUNIT_TEST(ctor_0);
  CPPUNIT_TEST(copyCtor_0);
  CPPUNIT_TEST(copyCtor_1);
  CPPUNIT_TEST(copyCtor_2);
  CPPUNIT_TEST(rawBufferCtor_0);
  CPPUNIT_TEST(rawBufferCtor_1);
  CPPUNIT_TEST(rawBufferCtor_2);
  CPPUNIT_TEST(rawBufferCtor_3);
  CPPUNIT_TEST(rawBufferCtor_4);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(toRawBuffer_1);
  CPPUNIT_TEST_SUITE_END();

public:
  virtual void setUp() {

    CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::Inclusive32BitWords;

    m_header.nwds = 22;
    m_header.type = DAQ::V8::DATABF;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = 2;

    m_bodyData = std::vector<uint16_t>({3, 0, 1,
                                        3, 0, 3});
    m_physicsBuffer = CPhysicsEventBuffer(m_header, m_bodyData);

  }

  virtual void tearDown()
  {
    CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::BufferPreference;
  }

  void totalShorts_1() {
    std::vector<uint16_t> data = {2, 0};
    ByteBuffer buffer;
    buffer << data;

    const CPhysicsEvent& event = CPhysicsEvent(buffer, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Total size for standard buffer makes sense",
                                 std::size_t(2), event.getNTotalShorts());

  }


  virtual DAQ::Buffer::ByteBuffer createSwappedEvent()
  {
    bheader header;
    header.nwds = 0x1600;
    header.type = 0x0100;
    header.nevt = 0x0200;
    header.buffmt = 0x0500;
    header.ssignature = 0x0201;
    header.lsignature = 0x04030201;
    header.cks = 0;
    header.cpu = 0;
    header.nbit = 0;
    header.nlam = 0;
    header.run = 0x0100;
    header.seq = 0x02000000;
    header.unused[0] = 0;
    header.unused[1] = 0;

    std::vector<std::uint16_t> data({0x0000, 0x0300, 0x0100,
                                     0x0000, 0x0300, 0x0300});


    DAQ::Buffer::ByteBuffer buffer;
    buffer << header;
    buffer << data;
    return buffer;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(physicseventtest_Incl32Word);



class physicseventtest_Incl32Byte : public physicseventtest {

public:
  CPPUNIT_TEST_SUITE(physicseventtest_Incl32Byte);
  CPPUNIT_TEST(totalShorts_1);
  CPPUNIT_TEST(ctor_0);
  CPPUNIT_TEST(copyCtor_0);
  CPPUNIT_TEST(copyCtor_1);
  CPPUNIT_TEST(copyCtor_2);
  CPPUNIT_TEST(rawBufferCtor_0);
  CPPUNIT_TEST(rawBufferCtor_1);
  CPPUNIT_TEST(rawBufferCtor_2);
  CPPUNIT_TEST(rawBufferCtor_3);
  CPPUNIT_TEST(rawBufferCtor_4);
  CPPUNIT_TEST(toRawBuffer_0);
  CPPUNIT_TEST(toRawBuffer_1);
  CPPUNIT_TEST_SUITE_END();

public:
  virtual void setUp() {

    CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::Inclusive32BitBytes;

    m_header.nwds = 22;
    m_header.type = DAQ::V8::DATABF;
    m_header.run  = 1;
    m_header.seq  = 2;
    m_header.nevt = 2;

    m_bodyData = std::vector<uint16_t>({6, 0, 1,
                                        6, 0, 3});
    m_physicsBuffer = CPhysicsEventBuffer(m_header, m_bodyData);

  }

  virtual void tearDown()
  {
    CPhysicsEventBuffer::m_bodyType = CPhysicsEventBuffer::BufferPreference;
  }

  void totalShorts_1() {
    std::vector<uint16_t> data = {2, 0};
    ByteBuffer buffer;
    buffer << data;

    const CPhysicsEvent& event = CPhysicsEvent(buffer, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Total size for standard buffer makes sense",
                                 std::size_t(2), event.getNTotalShorts());

  }


  virtual DAQ::Buffer::ByteBuffer createSwappedEvent()
  {
    bheader header;
    header.nwds = 0x1600;
    header.type = 0x0100;
    header.nevt = 0x0200;
    header.buffmt = 0x0500;
    header.ssignature = 0x0201;
    header.lsignature = 0x04030201;
    header.cks = 0;
    header.cpu = 0;
    header.nbit = 0;
    header.nlam = 0;
    header.run = 0x0100;
    header.seq = 0x02000000;
    header.unused[0] = 0;
    header.unused[1] = 0;

    std::vector<std::uint16_t> data({0x0000, 0x0600, 0x0100,
                                     0x0000, 0x0600, 0x0300});


    DAQ::Buffer::ByteBuffer buffer;
    buffer << header;
    buffer << data;
    return buffer;
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(physicseventtest_Incl32Byte);





