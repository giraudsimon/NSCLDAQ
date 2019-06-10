// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>

#include <Asserts.h>

#include <cppunit/Asserter.h>
#include <CSocket.h>
#include "CEventOrderClient.h"
#include <list>
#include <Thread.h>
#include <vector>
#include <stdlib.h>
#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <CRingItem.h>
#include <fragment.h>
#include <DataFormat.h>
#include <sys/socket.h>
#include <sstream>

static   int   m_nCurrentPort(2345);

// To satisfy the application level protocol, the server
// is going to need a thread of its own:

class  ServerThread : public Thread
{
private:
  CSocket* m_pSocket;
  bool     m_stopRequested;
public:
  std::vector<void*> m_messages;
public:
  ServerThread(CSocket* pSocket);
  ~ServerThread();
  virtual void run();
  void         stop();
private:
  void readMessage(CSocket* pSock);
  bool disconnect();
};

ServerThread::ServerThread(CSocket* pSocket) :
  m_pSocket(pSocket), m_stopRequested(false)
{}
ServerThread::~ServerThread() {
  for (auto p : m_messages) {
    free(p);
  }
}

void ServerThread::run()
{
  std::string client;
  CSocket* peer = m_pSocket->Accept(client);
  int fd = peer->getSocketFd();            // for polling.
  
  while(!m_stopRequested) {
    pollfd pstruct = {fd, POLLIN, 0};
    int s = poll(&pstruct, 1, 100);
    if (s > 0) {
      readMessage(peer);      // Request
      bool d = disconnect();
      if (!d) {
        readMessage(peer);       // body.
      }
      std::string reply("OK\n");
      peer->Write(reply.c_str(), reply.size());
      if (d) {
        return;
      }
    }
  }
}

bool
ServerThread::disconnect()
{
  // If last message was a disconnect return true:
  
  void* lastMsg = m_messages.back();
  uint8_t* p    = static_cast<uint8_t*>(lastMsg);
  p += sizeof(uint32_t);     // Point to any text that might be there.
  const char* disconnect="DISCONNECT";
  bool result = memcmp(p, disconnect, strlen(disconnect) - 1) == 0;
  return result;
  
}

void
ServerThread::readMessage(CSocket* pClient)
{
  // Read message size:
  
  uint32_t msgSize;
  pClient->Read(&msgSize, sizeof(uint32_t));
  uint8_t* pMessage = static_cast<uint8_t*>(malloc(msgSize + sizeof(uint32_t)));
  memcpy(pMessage, &msgSize, sizeof(uint32_t));
  pClient->Read(pMessage+sizeof(uint32_t), msgSize);
  
  m_messages.push_back(pMessage);
  
}
void ServerThread::stop()
{
  m_stopRequested = true;
  join();
}

class clienttests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(clienttests);
  CPPUNIT_TEST(initial);
  CPPUNIT_TEST(submit_1);
  CPPUNIT_TEST(submit_2);
  CPPUNIT_TEST_SUITE_END();


private:
  CSocket*           m_pServer;
  ServerThread*      m_Server;
  CEventOrderClient* m_pTestObj;

public:

  void setUp() {
    m_pServer = new CSocket;
    while(1) {
      try {
        std::string sPort(Itos(m_nCurrentPort));
        m_pServer->Bind(sPort);
        break;
      } catch(...) {m_nCurrentPort++; }      // Probably was in use.
    }
    m_pServer->Listen();
    
    m_Server = new ServerThread(m_pServer);
    m_Server->start();
    
    m_pTestObj = new CEventOrderClient("localhost", m_nCurrentPort++);
    m_pTestObj->Connect("me", {1, 2, 3});
  }
  void tearDown() {
    m_pTestObj->disconnect();
    delete m_pTestObj;
    m_Server->stop();
    delete m_Server;
    shutdown(m_pServer->getSocketFd(), SHUT_RD | SHUT_WR);
    delete m_pServer;

    
  }
protected:
  void initial();
  void submit_1();
  void submit_2();
private:
  static std::string countedString(void* pMsg);
  std::string Itos(int value);
};

CPPUNIT_TEST_SUITE_REGISTRATION(clienttests);

std::string clienttests::Itos(int value) {
  std::stringstream s;
  s << value;
  return s.str();
}

std::string clienttests::countedString(void* pMsg)
{
  uint32_t* pSize= static_cast<uint32_t*>(pMsg);
  uint8_t*  pBytes = static_cast<uint8_t*>(pMsg);
  
  uint32_t nChars = *pSize;
  pBytes += sizeof(uint32_t);
  
  std::string result;
  for (int i =0;  i < nChars; i++) {
    result.push_back(*pBytes++);
  }
  return result;
}

// When connected the server has a connection message.

void clienttests::initial() {
  EQ(size_t(2), m_Server->m_messages.size());
  void* connectMsg = m_Server->m_messages.front();
  
  EQ(std::string("CONNECT"), countedString(connectMsg));
}

// A single fragment is created/submitted.

void clienttests::submit_1()
{
  CRingItem item(PHYSICS_EVENT, 0x123456789, 1, 0);
  uint32_t* p = static_cast<uint32_t*>(item.getBodyCursor());
  for (int i =0; i < 10; i++) {
    *p++ = i;
  }
  item.setBodyCursor(p);
  item.updateSize();
  pRingItem pItem = static_cast<pRingItem>(item.getItemPointer());
  
  EVB::Fragment f;
  f.s_header.s_timestamp = item.getEventTimestamp();
  f.s_header.s_sourceId  = item.getSourceId();
  f.s_header.s_size      = pItem->s_header.s_size;
  f.s_header.s_barrier   = item.getBarrierType();
  f.s_pBody              = pItem;
  
  m_pTestObj->submitFragments(size_t(1), &f);
  EQ(size_t(4), m_Server->m_messages.size());   // Connection msg and frags.
  
  // The last message should be a uint32_t our fragment header and our
  // ring item.
  
  uint32_t* pLast = static_cast<uint32_t*>(m_Server->m_messages.back());
  pLast++;                   // the ring item.
  EVB::pFragmentHeader pH = reinterpret_cast<EVB::pFragmentHeader>(pLast);
  EQ(f.s_header.s_timestamp, pH->s_timestamp);
  EQ(f.s_header.s_sourceId, pH->s_sourceId);
  EQ(f.s_header.s_size, pH->s_size);
  EQ(f.s_header.s_barrier, pH->s_barrier);
  
  pH++;                   // Points to the ring item:
  pRingItem pI = reinterpret_cast<pRingItem>(pH);
  ASSERT(memcmp(pItem, pI, f.s_header.s_size) == 0);
}
// Submits 10 items.
void clienttests::submit_2()
{
  EVB::Fragment f[10];
  CRingItem* pItems[10];
  pRingItem  pRawItems[10];
  for (int i = 0; i < 10; i++) {
    pItems[i] = new CRingItem(PHYSICS_EVENT, i, 1, 0);
    uint32_t* p = static_cast<uint32_t*>(pItems[i]->getBodyCursor());
    for (int j = i; (j - i) < 10; j++) {
      *p++ = j;
    }
  
    pItems[i]->setBodyCursor(p);
    pItems[i]->updateSize();
    pRawItems[i] = static_cast<pRingItem>(pItems[i]->getItemPointer());
    f[i].s_header.s_timestamp = pItems[i]->getEventTimestamp();
    f[i].s_header.s_sourceId  = pItems[i]->getSourceId();
    f[i].s_header.s_size      = pRawItems[i]->s_header.s_size;
    f[i].s_header.s_barrier   = pItems[i]->getBarrierType();
    f[i].s_pBody              = pRawItems[i];
  }
  m_pTestObj->submitFragments(size_t(10), f);
  
  // The last message received by the server will have all of the headers and
  // ring items:
  
  uint32_t* p32 = static_cast<uint32_t*>(m_Server->m_messages.back());
  p32++;                 // Points to the first fragment header.
  for (int i = 0; i < 10; i ++) {
    EVB::pFragmentHeader pH = reinterpret_cast<EVB::pFragmentHeader>(p32);
    
    // Check the fragment header:
    
    EQ(f[i].s_header.s_timestamp, pH->s_timestamp);
    EQ(f[i].s_header.s_sourceId,  pH->s_sourceId);
    EQ(f[i].s_header.s_size,      pH->s_size);
    EQ(f[i].s_header.s_barrier,   pH->s_barrier);
    
    pH++;                    // Points to the ring item.
    pRingItem pI   = reinterpret_cast<pRingItem>(pH);
 
     
    // check the ring item.
    
    // Set up for the next loop pass:
    
    uint8_t* p = reinterpret_cast<uint8_t*>(pI);
    p += pI->s_header.s_size;
    p32 = reinterpret_cast<uint32_t*>(p);
  }
}