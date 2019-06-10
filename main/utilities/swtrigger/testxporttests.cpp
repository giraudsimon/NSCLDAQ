// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CTestTransport.h"

#include <stdlib.h>

class testxportTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(testxportTest);
  CPPUNIT_TEST(initial_1);
  
  CPPUNIT_TEST(send_1);
  CPPUNIT_TEST(send_2);
  CPPUNIT_TEST(send_3);
  
  CPPUNIT_TEST(add_1);
  CPPUNIT_TEST(add_2);
  
  CPPUNIT_TEST(recv_1);
  CPPUNIT_TEST_SUITE_END();


private:
  CTestTransport* m_pTestObject;
public:
  void setUp() {
    m_pTestObject = new CTestTransport;
  }
  void tearDown() {
    delete m_pTestObject;
  }
protected:
  void initial_1();
  
  void send_1();
  void send_2();
  void send_3();
  
  void add_1();
  void add_2();
  
  void recv_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(testxportTest);

void testxportTest::initial_1() {   // Should be no messages
  
  ASSERT(m_pTestObject->m_sentMessages.empty());
  ASSERT(m_pTestObject->m_messages.empty());
}

void testxportTest::send_1()          // Send single part message
{
  // Create and send a single part message:
  
  uint8_t message[256];
  for(int i =0; i < 256; i++) {
    message[i] = i;
  }
  iovec msg;
  msg.iov_base = message;
  msg.iov_len = sizeof(message);
  m_pTestObject->send(&msg, 1);
  
  // Should have been stored properly in m_sentMessages:
  
  EQ(size_t(1), m_pTestObject->m_sentMessages.size());
  auto& sentMsg = m_pTestObject->m_sentMessages.front();
  EQ(size_t(1), sentMsg.size());    // One message part.
  EQ(size_t(256), sentMsg[0].size());
  for (int i =0; i < 256; i++) {
    EQ(uint8_t(i), sentMsg[0][i]);
  }
  
}

void testxportTest::send_2()    // send a multipart message.
{
  uint8_t part1[256];
  uint8_t part2[128];
  
  for (int i =0; i < 256; i++) {
    part1[i] = i;
  }
  for (int i =0; i < 128; i++) {
    part2[i]  = 127-i;
  }
  
  iovec msg[2];
  msg[0].iov_base = part1;
  msg[0].iov_len  =sizeof(part1);
  
  msg[1].iov_base = part2;
  msg[1].iov_len  = sizeof(part2);
  
  m_pTestObject->send(msg, 2);
  
  // See if m_sentMessages has the 1 2-part message:
  
  EQ(size_t(1), m_pTestObject->m_sentMessages.size());
  auto message = m_pTestObject->m_sentMessages.front();
  EQ(size_t(2), message.size());
  EQ(size_t(256), message[0].size());
  EQ(size_t(128), message[1].size());
  
  for (int i =0; i < 256; i++) {
    EQ(uint8_t(i), message[0][i]);
  }
  for (int i = 0; i < 128; i++) {
    EQ(uint8_t(127-i), message[1][i]);
  }
}
void testxportTest::send_3()
{
  uint8_t part1[256];
  uint8_t part2[128];
  
  for (int i =0; i < 256; i++) {
    part1[i] = i;
  }
  for (int i =0; i < 128; i++) {
    part2[i]  = 127-i;
  }
  
  iovec msg[2];
  msg[0].iov_base = part1;
  msg[0].iov_len  =sizeof(part1);
  
  msg[1].iov_base = part2;
  msg[1].iov_len  = sizeof(part2);
  
  m_pTestObject->send(msg, 1);    // Just put in the first part.
  m_pTestObject->send(msg, 2);    // Put in both parys.
  m_pTestObject->send(&(msg[1]), 1);  // Put in the second part.
  
  // Now check the messages.
  
  EQ(size_t(3), m_pTestObject->m_sentMessages.size());
  auto& msg1 = m_pTestObject->m_sentMessages.at(0);
  auto& msg2 = m_pTestObject->m_sentMessages.at(1);
  auto& msg3 = m_pTestObject->m_sentMessages.at(2);
  
  EQ(size_t(1), msg1.size());
  EQ(size_t(2), msg2.size());
  EQ(size_t(1), msg3.size());
  
  for(int i =0; i < 256; i++) {
    EQ(uint8_t(i), msg1[0][i]);
    EQ(uint8_t(i), msg2[0][i]);
  }
  for (int i =0; i < 128; i++) {
    uint8_t sb = 127 - i;
    EQ(sb, msg2[1][i]);
    EQ(sb, msg3[0][i]);
  }
}

void testxportTest::add_1()    // adding a message should make it appear.
{
 uint8_t message[256];
  for(int i =0; i < 256; i++) {
    message[i] = i;
  }
  
  m_pTestObject->addMessage(message, sizeof(message));
  
  // This single message should be in m_messages.
  
  EQ(size_t(1), m_pTestObject->m_messages.size());
  auto& msg = m_pTestObject->m_messages.at(0);
  
  for(int i =0; i < 256; i++) {
    EQ(uint8_t(i), msg[i]);
  }
}
void testxportTest::add_2()   // Adding a pair of messages keeps the right.
{
  uint8_t message1[256];
  uint8_t message2[128];
  for (int i =0; i < 256; i++) {
    message1[i] = i;
  }
  for (int i =0; i < 128; i++) {
    message2[i] = 127-i;
  }
  
  m_pTestObject->addMessage(message1, sizeof(message1));
  m_pTestObject->addMessage(message2, sizeof(message2));
  
  EQ(size_t(2), m_pTestObject->m_messages.size());
  
  auto& msg1 = m_pTestObject->m_messages.at(0);
  auto& msg2 = m_pTestObject->m_messages.at(1);
  
  for (int i =0; i < 256; i++) {
    EQ(uint8_t(i), msg1[i]);
  }
  for (int i =0; i < 128; i++) {
    EQ(uint8_t(127-i), msg2[i]);
  }
}
void testxportTest::recv_1()
{
  uint8_t message1[256];
  uint8_t message2[128];
  for (int i =0; i < 256; i++) {
    message1[i] = i;
  }
  for (int i =0; i < 128; i++) {
    message2[i] = 127-i;
  }
  
  m_pTestObject->addMessage(message1, sizeof(message1));
  m_pTestObject->addMessage(message2, sizeof(message2));
  
  // Should be able to get these messages by recv as well:
  
  uint8_t* pMsg;
  size_t  nBytes;
  m_pTestObject->recv(reinterpret_cast<void**>(&pMsg), nBytes);
  EQ(sizeof(message1), nBytes);
  for (int i = 0; i < nBytes; i++) {
    EQ(message1[i], pMsg[i]);
  }
  free(pMsg);
  
  m_pTestObject->recv(reinterpret_cast<void**>(&pMsg), nBytes);
  EQ(sizeof(message2), nBytes);
  for (int i =0;  i < nBytes; i++) {
    EQ(message2[i], pMsg[i]);
  }
  free(pMsg);
  m_pTestObject->recv(reinterpret_cast<void**>(&pMsg), nBytes);
  EQ(size_t(0), nBytes);
  
}