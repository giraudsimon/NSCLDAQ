// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "ZMQFanoutBalancedRingSource.h"
#include <CRingBlockReader.h>
#include <DataFormat.h>
#include "CRingItemDataSink.h"
#include <pthread.h>
#include <set>
#include <stdlib.h>
#include <string.h>

/**
 * fakeSink
 *    Fake sink for the source to get data from.
 */
class FakeSink : public CRingItemDataSink
{
private:
  std::list<CRingBlockReader::pDataDescriptor> data;
  std::set<pthread_t>                          clients;
public:
  FakeSink() {}
  virtual ~FakeSink() {}
  
  void addData(CRingBlockReader::pDataDescriptor d) {
    data.push_back(d);
  }
  const std::set<pthread_t>& getClients() {return clients;}
  
  virtual void connect() {}             //< Connect to sink for 1:1 comms.
  virtual void disconnect() {}          //< Disconnect sink for 1:1 comms.
  virtual void send(MessageType::Message& msg) {} //< send to peer.
  virtual void onPullRequest(MessageType::Message& msg){}
  virtual void Register(
    MessageType::Message& regmsg) ;   //< Register as data sink
  virtual void unRegister(
    MessageType::Message& unreg
  );
  virtual bool haveMessage() {return true;}
  MessageType::Message getMessage() {MessageType::Message r; return r;}
  virtual void* connectSink();         //< If we are server.
  virtual void  closeSink(void* c);  //< If we are server.
  virtual MessageType::Message  requestData(void* c); //< Pull data
  
  // utilities.
  
  MessageType::Message createRequestMessage(uint32_t typeCode);
  MessageType::Message createEndItem();
  MessageType::Message createDataItem();
};

// test sink implementations.

void FakeSink::Register(MessageType::Message& msg)
{
  pthread_t* tid = static_cast<pthread_t*>(msg.s_dataParts.front().second);
  clients.insert(*tid);
  free(tid);
  
}
void FakeSink::unRegister(MessageType::Message& msg)
{
  pthread_t* tid = static_cast<pthread_t*>(msg.s_dataParts.front().second);
  clients.erase(*tid);
  free(tid);
}
// connect sink has to create a registration message, call Register and
// return a null pointer.

void* FakeSink::connectSink()
{
  MessageType::Message request = createRequestMessage(MessageType::REGISTRATION);
  Register(request);
  
  return nullptr;
}
void FakeSink::closeSink(void* p)
{
  MessageType::Message request = createRequestMessage(MessageType::UNREGISTRATION);
  unRegister(request);
}
/*
 *  Give the first item or END_ITEM if there are none left.
*/
MessageType::Message FakeSink::requestData(void* c)
{
  if (data.empty()) {
    return createEndItem();
  } else {
    return createDataItem();
  }
}

MessageType::Message FakeSink::createRequestMessage(uint32_t typeCode)
{
  MessageType::Message result;
  result.s_messageType = typeCode;
  std::pair<uint32_t, void*> data;
  data.first = sizeof(pthread_t);
  data.second = malloc(data.first);
  pthread_t me = pthread_self();
  memcpy(data.second, &me, data.first);
  
  return result;
}

MessageType::Message FakeSink::createEndItem()
{
  MessageType::Message result;
  result.s_messageType = MessageType::END_ITEM;
  return result;


}
MessageType::Message  FakeSink::createDataItem()
{
  MessageType::Message result;
  
  CRingBlockReader::pDataDescriptor pData = data.front();
  data.pop_front();
  
  result.s_messageType = MessageType::PROCESS_ITEM;
  std::pair<uint32_t, void*> count;
  std::pair<uint32_t, void*> data;
  
  count.first = sizeof(uint32_t);
  count.second = malloc(count.first);
  memcpy(count.second, &(pData->s_nItems), sizeof(uint32_t));
  
  result.s_dataParts.push_back(count);
  
  data.first  = pData->s_nBytes + sizeof(CRingBlockReader::DataDescriptor);
  data.second = pData;
  
  result.s_dataParts.push_back(data);
  
  return result;
}
class Testname : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Testname);
  CPPUNIT_TEST(aTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void aTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Testname);

void Testname::aTest() {
}
