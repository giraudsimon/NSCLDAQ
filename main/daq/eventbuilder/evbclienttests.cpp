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
      if (!disconnect()) {
        readMessage(peer);       // body.
      }
      std::string reply("OK\n");
      peer->Write(reply.c_str(), reply.size());
    }
  }
}

bool
ServerThread::disconnect()
{
  // If last message was a disconnect return true:
  
  void* lastMsg = m_messages.back();
  uint8_t* p    = static_cast<uint8_t*>(lastMsg);
  const char* disconnect="DISCONNECT";
  return memcmp(p, disconnect, strlen(disconnect) - 1);
  
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
  CPPUNIT_TEST_SUITE_END();


private:
  CSocket*           m_pServer;
  ServerThread*      m_Server;
  CEventOrderClient* m_pTestObj;
public:
  void setUp() {
    m_pServer = new CSocket;
    m_pServer->Bind("2345");
    m_pServer->Listen();
    
    m_Server = new ServerThread(m_pServer);
    m_Server->start();
    
    m_pTestObj = new CEventOrderClient("localhost", 2345);
    m_pTestObj->Connect("me", {1, 2, 3});
  }
  void tearDown() {
    m_pTestObj->disconnect();
    delete m_pTestObj;
    m_Server->stop();
    delete m_Server;
    delete m_pServer;

    
  }
protected:
  void initial();
private:
  std::string countedString(void* pMsg);
};

CPPUNIT_TEST_SUITE_REGISTRATION(clienttests);

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
