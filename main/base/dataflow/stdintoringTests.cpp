// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CRingBuffer.h>

#include "testcommon.h"
#include "stdintoringUtils.h"
#include <string.h>

using namespace std;

class stdin2ringUtilsTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(stdin2ringUtilsTest);
  CPPUNIT_TEST(integerize_1);
  CPPUNIT_TEST(integerize_2);
  CPPUNIT_TEST(integerize_3);
  CPPUNIT_TEST(integerize_4);
  
  CPPUNIT_TEST(size_1);
  CPPUNIT_TEST(size_2);
  
  CPPUNIT_TEST(putdata_1);
  CPPUNIT_TEST(putdata_2);
  CPPUNIT_TEST(putdata_3);
  CPPUNIT_TEST(putdata_4);
  CPPUNIT_TEST_SUITE_END();


private:
  CRingBuffer* m_pProducer;
  CRingBuffer* m_pConsumer;
public:
  void setUp() {
    m_pProducer = CRingBuffer::createAndProduce(uniqueRing("urllocal"));
    m_pConsumer = new CRingBuffer(uniqueRing("urllocal"));
  }
  void tearDown() {
    delete m_pProducer;
    delete m_pConsumer;
    CRingBuffer::remove(uniqueRing("urllocal"));  
  }
protected:
  void integerize_1();
  void integerize_2();
  void integerize_3();
  void integerize_4();
  
  void size_1();
  void size_2();
  
  void putdata_1();
  void putdata_2();
  void putdata_3();
  void putdata_4();
};

CPPUNIT_TEST_SUITE_REGISTRATION(stdin2ringUtilsTest);

// Set/get proxy ring size:

void stdin2ringUtilsTest::integerize_1()
{
  //  Straight number becomes a straight number:
  
  int result = integerize("1234");
  EQ(1234, result);
}

void stdin2ringUtilsTest::integerize_2()
{
  // k suffix
  
  int result = integerize("666k");
  EQ(666*1024, result);
}

void stdin2ringUtilsTest::integerize_3()
{
  // m suffix:
  
  int result = integerize("44m");
  EQ(44*1024*1024, result);
}
void stdin2ringUtilsTest::integerize_4()
{
  // g suffix.
  
  int result = integerize("1g");
  EQ(1024*1024*1024, result);
}

void stdin2ringUtilsTest::size_1()
{
  header h = {1234, 1};          // no swap required
  uint32_t result = computeSize(&h);
  EQ(uint32_t(1234), result);
}
void stdin2ringUtilsTest::size_2()
{
  // requires swap:
  
  header h = {0x1230000, 0x10000};
  uint32_t result = computeSize(&h);
  EQ(uint32_t(0x2301), result);
}

void stdin2ringUtilsTest::putdata_1()
{
  // The buffer has a single complete 'ring item' it should get written to
  // the ring and the residual should be 0.
  
  uint8_t buffer[100];
  pHeader pH = reinterpret_cast<pHeader>(buffer);
  pH->s_size = sizeof(buffer);
  pH->s_type = 1;
  uint8_t* pBody = reinterpret_cast<uint8_t*>(pH+1);
  size_t   nPayload = sizeof(buffer) - sizeof(header);
  for (int i = 0; i < nPayload; i++) {
    pBody[i] = i;
  }
  
  size_t residual = putData(*m_pProducer, buffer, sizeof(buffer));
  EQ(size_t(0), residual);
  
  uint8_t fromRing[100];
  CRingBuffer::Usage u = m_pConsumer->getUsage();
  size_t getSize = u.s_maxGetSpace;
  EQ(sizeof(buffer), getSize);
  
  size_t gotten = m_pConsumer->get(fromRing, sizeof(fromRing), sizeof(fromRing));
  EQ(gotten, sizeof(fromRing));
  EQ(0, memcmp(buffer, fromRing, gotten));
}

void stdin2ringUtilsTest::putdata_2()
{
  // Put a pair of full ring items in the ring .. should be able to get them
  // both out -- all as one message blob:
  
  uint8_t buffer[200];                     // Split in two for ring items.
  pHeader h1 = reinterpret_cast<pHeader>(buffer);
  pHeader h2 = reinterpret_cast<pHeader>(&(buffer[100]));  // last 100 bytes.
  
  h1->s_size = 100;
  h1->s_type = 1;
  uint8_t* p = reinterpret_cast<uint8_t*>(h1+1);
  size_t bodySize = 100 - sizeof(header);
  for (int i = 0; i < bodySize; i++) {
    *p++ = i;
  }
  h2->s_size = 100;
  h2->s_type = 2;
  p = reinterpret_cast<uint8_t*>(h2+1);
  for (int i = 0; i < bodySize; i++) {
    *p = 2*i;
  }
  size_t resid = putData(*m_pProducer, buffer, sizeof(buffer));
  EQ(size_t(0), resid);
  
  CRingBuffer::Usage u = m_pConsumer->getUsage();
  size_t getSize = u.s_maxGetSpace;
  EQ(sizeof(buffer), getSize);
  
  uint8_t gotData[200];
  size_t gotten = m_pConsumer->get(gotData, sizeof(gotData), sizeof(gotData), 0);
  EQ(getSize, gotten);
  EQ(0, memcmp(buffer, gotData, sizeof(buffer)));
  
}
void stdin2ringUtilsTest::putdata_3()
{
  // the buffer has a full ring item and the header o the next one.
  // putData then finishing off the ring item and putdata again
  // should allow the recovery of both ring items.
  
  uint8_t buffer[100+sizeof(header)];
  pHeader h1 = reinterpret_cast<pHeader>(buffer);
  h1->s_size = 100;
  h1->s_type = 1;
  uint8_t* p = reinterpret_cast<uint8_t*>(h1+1);
  size_t bodySize = 100 - sizeof(header);
  for (int i =0; i < bodySize; i++) {
    *p++ = i;
  }
  pHeader h2 = reinterpret_cast<pHeader>(p);
  h2->s_size = 100;
  h2->s_type = 2;
  
  size_t residual = putData(*m_pProducer, buffer, sizeof(buffer));
  EQ(sizeof(header), residual);
  
  // The header should have been slid down to the front of the buffer:
  // h2 now points to it.
  
  p = reinterpret_cast<uint8_t*>(h1+1);
  for (int i = 0; i < bodySize; i++) {
    *p++ = i*2;
  }
  residual = putData(*m_pProducer, buffer, 100);
  EQ(size_t(0), residual);
  
  // SHould be 200 byts of data inthe ring:
  
  CRingBuffer::Usage u = m_pConsumer->getUsage();
  size_t getSize = u.s_maxGetSpace;
  EQ(size_t(200), getSize);
  
  // Get the data:
  
  uint8_t gotten[200];
  size_t nGot = m_pConsumer->get(gotten, sizeof(gotten), sizeof(gotten), 1);
  EQ(sizeof(gotten), nGot);
  
  h1 = reinterpret_cast<pHeader>(gotten);
  EQ(uint32_t(100), h1->s_size);
  EQ(uint32_t(1),   h1->s_type);
  p = reinterpret_cast<uint8_t*>(h1+1);
  for (int i =0; i < bodySize; i++) {
    EQ(uint8_t(i), *p);
    p++;
  }
}
void stdin2ringUtilsTest::putdata_4()
{
  // This test will put a full ring item and then one byte of the next one.
  // we'll then complete the second ring item and get it put.
  //
  
  uint8_t buffer[101];
  pHeader ph1 = reinterpret_cast<pHeader>(buffer);
  ph1->s_size = 100;
  ph1->s_type = 1;
  
  size_t datasize = 100 - sizeof(header);
  uint8_t* p = reinterpret_cast<uint8_t*>(ph1 + 1);
  for (int i = 0; i < datasize; i++) {
    *p++ = i;
  }
  // Put a known pattern in the last cell of buffer:
  
  buffer[100] = 0xa5;
  
  size_t resid = putData(*m_pProducer, buffer, sizeof(buffer));
  EQ(size_t(1), resid);
  EQ(uint8_t(0xa5), buffer[0]);    // CHeck the slide.
  ph1->s_size=100;
  ph1->s_type= 2;
  p = reinterpret_cast<uint8_t*>(ph1+1);
  for (int i = 0; i < datasize; i++) {
    *p++ =  i*2;
  }
  resid = putData(*m_pProducer, buffer, 100);
  EQ(size_t(0), resid);
  
  
  // 2 100 byte objects should be in the ring:
  
  CRingBuffer::Usage u = m_pConsumer->getUsage();
  size_t getSize = u.s_maxGetSpace;
  EQ(size_t(200), getSize);
  
  uint8_t gotten[200];
  size_t ngot  =  m_pConsumer->get(gotten, sizeof(gotten), sizeof(gotten), 1);
  EQ(sizeof(gotten), ngot);
  
  ph1 = reinterpret_cast<pHeader>(gotten);
  EQ(uint32_t(100), ph1->s_size);
  EQ(uint32_t(1),   ph1->s_type);
  p = reinterpret_cast<uint8_t*>(ph1+1);
  for (int i =0; i < datasize; i++) {
    EQ(uint8_t(i), *p);
    p++;
  }
  
  ph1 = reinterpret_cast<pHeader>(p);
  EQ(uint32_t(100), ph1->s_size);
  EQ(uint32_t(2), ph1->s_type);
  p = reinterpret_cast<uint8_t*>(ph1+1);
  for (int i =0; i < datasize; i++) {
    EQ(uint8_t(i*2), *p);
    p++;
  }
  
}