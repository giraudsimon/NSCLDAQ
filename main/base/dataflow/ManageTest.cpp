// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <StateException.h>
#include <CRingBuffer.h>
#include <ringbufint.h>
#include <testcommon.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>



#include "Asserts.h"


using namespace std;



class ManageTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ManageTests);
  CPPUNIT_TEST(attach);
  CPPUNIT_TEST(forceproducer);
  CPPUNIT_TEST(forceconsumer);
  CPPUNIT_TEST_SUITE_END();


private:
    std::string SHM_TESTFILE;
public:
  void setUp() {
    SHM_TESTFILE= uniqueRing("mgrtest");
    CRingBuffer::create(string(SHM_TESTFILE));
  }
  void tearDown() {
    try {
      CRingBuffer::remove(SHM_TESTFILE);
    }
    catch (...) {}
  }
protected:
  void attach();
  void forceproducer();
  void forceconsumer();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ManageTests);

// Should be able to attach to the ring buffer as a manager.
// Data transfers should pop a state exception 
void ManageTests::attach() {
  CRingBuffer ring(string(SHM_TESTFILE), CRingBuffer::manager);
  

  // Should not be able to put:

  bool caught = false;
  try {
    char buffer[100];
    ring.put(buffer, sizeof(buffer));
  }
  catch (CStateException& state) {
    caught = true;
  }
  ASSERT(caught);

  caught = false;
  try {
    char buffer[100];
    ring.get(buffer, sizeof(buffer), 1);
  }
  catch (CStateException& state) {
    caught = true;
  }

  ASSERT(caught);
}

// Should be able to force the disconnect of a producer.
// that would allow another producer to connect:

void ManageTests::forceproducer()
{
  CRingBuffer p1(string(SHM_TESTFILE), CRingBuffer::producer);
  CRingBuffer mgr(string(SHM_TESTFILE), CRingBuffer::manager);

  // Disconnect the producer.

  mgr.forceProducerRelease();

  bool caught = false;

  try {
    CRingBuffer p2(string(SHM_TESTFILE), CRingBuffer::producer); // should be possible.
  }
  catch (...) {
    caught = true;
  }
  ASSERT(!caught);
}

// If I force a consumer to release, it's slot should get re-used.
//
void ManageTests::forceconsumer()
{
  CRingBuffer c1(string(SHM_TESTFILE), CRingBuffer::consumer);
  CRingBuffer mgr(string(SHM_TESTFILE), CRingBuffer::manager);

  mgr.forceConsumerRelease(0);

  CRingBuffer c2(string(SHM_TESTFILE), CRingBuffer::consumer);
  EQ((off_t)0, c2.getSlot());
}
