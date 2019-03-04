// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CEventOrderClient.h>
#include <errno.h>
#include <ErrnoException.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <CPortManager.h>
#include <CSocket.h>
#include <string.h>
#include <iostream>


class disconnectTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(disconnectTest);
  CPPUNIT_TEST(normalDisconnect);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void normalDisconnect();
};

CPPUNIT_TEST_SUITE_REGISTRATION(disconnectTest);



//
// After connection disconnect should return normally.
//
void
disconnectTest::normalDisconnect()
{
  // fork a server off... the server will accept one connection
  // then handle a disconnect.
  //

  pid_t pid = fork();
  if (pid) {			// parent process
    int status;
    sleep(3);			// Let the server get going.

    CEventOrderClient client("localhost", CEventOrderClient::Lookup("localhost"));
    client.Connect("Test connection");
    bool thrown = false;
    try {
      client.disconnect();
    }
    catch(...) {
      thrown = true;
    }
    ASSERT(thrown);


    pid_t exited = wait(&status);

    EQ(pid, exited);
  } else {			// child process
    char portString[80];
    CSocket* pClient(0);
    CSocket* server;
    try {
    CPortManager m;
    int port = m.allocatePort("ORDERER");
    sprintf(portString, "%d", port);
    server = new CSocket;
    server->Bind(portString);
    server->Listen();
    server->setLinger(false, 0);


    std::string client;
    pClient = server->Accept(client);

    // Process the connect string:

    uint32_t stringSize;
    pClient->Read(&stringSize, sizeof(uint32_t));
    char request[stringSize+1];
    memset(request, 0, stringSize+1);


    pClient->Read(request, stringSize);
    
    // "test connection" string
    pClient->Read(&stringSize, sizeof(uint32_t));
    
    char body[stringSize+1];
    memset(body, 0, stringSize+1);
    pClient->Read(body, stringSize);

     
    // Reply with OK"

    std::string reply("OK\n");
    pClient->Write(reply.c_str(), strlen(reply.c_str()));

    // Wait for disconnect:

    pClient->Read(&stringSize, sizeof(uint32_t));
    char disconnect[stringSize+1];
    memset(disconnect, 0, stringSize+1);
    pClient->Read(disconnect, stringSize);

    pClient->Shutdown();
    } 
    catch(...) {

    }
    delete server;
    delete pClient;
    exit(0);

    
  }
}
