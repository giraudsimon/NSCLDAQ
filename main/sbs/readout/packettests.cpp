// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <string.h>

#include <stdint.h>

#include <string>

#include <CDocumentedPacket.h>
#include <CDocumentedPacketManager.h>
#include <CInvalidPacketStateException.h>
#include <CNoSuchObjectException.h>

using namespace std;

// Packets start out looking like this:

typedef struct _header {
  uint32_t size  __attribute__((packed));
  uint16_t type  __attribute__((packed));
  uint16_t body[1]  __attribute__((packed));		// really however many words.
} header, *pheader;

class PacketTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PacketTests);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(empty);
  CPPUNIT_TEST(nonempty);
  CPPUNIT_TEST(exceptions);
  CPPUNIT_TEST(format);
  CPPUNIT_TEST(mgrgoodinquiry);
  CPPUNIT_TEST(mgrmissing);
  CPPUNIT_TEST_SUITE_END();


private:
  CDocumentedPacket* m_pPacket;

public:
  void setUp() {
    m_pPacket = new CDocumentedPacket(0x1234,
				      string("test"),
				      string("packet for testing purposes"),
				      string("V1.0-001"));
  }
  void tearDown() {
    delete m_pPacket;
  }
protected:
  void construct();
  void empty();
  void nonempty();
  void exceptions();
  void format();
  void mgrgoodinquiry();
  void mgrmissing();
};

CPPUNIT_TEST_SUITE_REGISTRATION(PacketTests);

// Check that construction gives the right stuff fromt he selectors:

void PacketTests::construct() {
  EQ((uint16_t)0x1234, m_pPacket->getTag());
  EQ(string("test"), m_pPacket->getName());
  EQ(string("packet for testing purposes"), m_pPacket->getDescription());
  EQ(string("V1.0-001"), m_pPacket->getVersion());
     
}
// Empty packet:
//  - In progress should track.
//  - Header should have the correct contents.
//
void
PacketTests::empty()
{
  header packet;
  pheader ppacket = &packet;

  ASSERT(!m_pPacket->getPacketInProgress());
  m_pPacket->Begin(reinterpret_cast<uint16_t*>(ppacket));
  ASSERT(m_pPacket->getPacketInProgress());
  m_pPacket->End(reinterpret_cast<uint16_t*>(&(ppacket[1])));
  ASSERT(!m_pPacket->getPacketInProgress());

  EQ((uint32_t)(sizeof(packet)/sizeof(uint16_t)), packet.size);
  EQ((uint16_t)0x1234, packet.type);


}
//
// Construct a packet with some body of a counting pattern.
// Ensure that the header is right.
//
void
PacketTests::nonempty()
{
  struct _mypacket {
    header h;
    uint16_t body[10] __attribute__((packed));
  } pkt;
  struct _mypacket* pP = &pkt;

  uint16_t* b = pkt.body;
  uint16_t* ppkt = reinterpret_cast<uint16_t*>(&pkt);

  m_pPacket->Begin(ppkt);
  for (int i=0; i < 10; i++) {
    *b++ = i;
  }
  m_pPacket->End(reinterpret_cast<uint16_t*>(&(pP[1])));

  EQ(sizeof(struct _mypacket)/sizeof(uint16_t), (size_t)(pkt.h.size));
  EQ((uint16_t)0x1234, pkt.h.type);

}
// check some of the common exception throws.
//
void
PacketTests::exceptions()
{
  header h;
  uint16_t* ph = (reinterpret_cast<uint16_t*>(&h));

  // End without begin is an invalid packet state:

  CPPUNIT_ASSERT_THROW(m_pPacket->End(ph), CInvalidPacketStateException );
  
  // Two opens without a close throws:

  m_pPacket->Begin(ph);
  CPPUNIT_ASSERT_THROW(m_pPacket->Begin(ph), CInvalidPacketStateException);
  
  // need to end the packet for destruction:

  m_pPacket->End(ph);


}
/*
  Check the format string which should be:
  id:name:description:version:timestamp
  We're not going to check the timestamp.
*/
void 
PacketTests::format()
{
  string description = m_pPacket->Format();
  size_t n           = description.size();
  char*  sdescr      = new char[n+1];
  strcpy(sdescr, description.c_str());
  char*  context;

  string name  = strtok_r(sdescr, ":", &context);
  string id    = strtok_r(NULL,   ":", &context);
  string dsc   = strtok_r(NULL,   ":", &context);
  string vsn   = strtok_r(NULL,   ":", &context);

  EQ(string("0x1234"), id);
  EQ(string("test"), name);
  EQ(string("packet for testing purposes"), dsc);

  

  delete []sdescr;
}
//
// The packet should be in the packet manager (it's constructor registers it).
//
void
PacketTests::mgrgoodinquiry()
{
  CDocumentedPacketManager* pMgr = CDocumentedPacketManager::getInstance();

  EQ((unsigned)1, pMgr->size());
  EQ(m_pPacket, *(pMgr->begin()));
  DocumentationPacketIterator i = pMgr->begin();
  i++;
  ASSERT(pMgr->end() == i);
}
// 
// Deleting a manager that does not exist should throw a nosuchobject exception.
//
void
PacketTests::mgrmissing()
{
  CDocumentedPacketManager* pMgr = CDocumentedPacketManager::getInstance();
  CPPUNIT_ASSERT_THROW(pMgr->DeletePacket("george"), CNoSuchObjectException);

}
