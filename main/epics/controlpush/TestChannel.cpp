// Template for a test suite.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CChannel.h"


using namespace std;



// Tests for the channel class.


class TestChannel : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestChannel);
  CPPUNIT_TEST(ConstructGood);
  CPPUNIT_TEST(Connect);
  CPPUNIT_TEST(Updates);	// Best done with a jittery channel.
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void ConstructGood();
  void Connect();
  void Updates();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestChannel);


const string goodname = "Z001DV"; // Good at nscl.

// On constructing a good channel, it should 
// have:
//    Name        == goodname.
//    Connected   == false.
//
void
TestChannel::ConstructGood()
{
  CChannel good(goodname);

  EQMSG("Name comparison", goodname, good.getName());
  EQMSG("Not Connected",   false,    good.isConnected());
}
// Construct a good channel, connect it and ensure that it gets connected within
// a 'short time'
//
void
TestChannel::Connect()
{
  CChannel good(goodname);
  good.Connect();
  CChannel::doEvents(0.5);

  EQMSG("Connected", true, good.isConnected());

}
//
// Constructs a good channel and then sees if it gets updated.
//
void
TestChannel::Updates()
{
  CChannel good(goodname);
  good.Connect();

  CChannel::doEvents(0.5);	// Should get connected/updated
  

  // We should have at least one update by now..since
  // Jan 1, 1970 in any event.

  time_t firstupdate = good.getLastUpdate();
  ASSERT(firstupdate != 0);

  // After a few seconds, we should have a different update time I'm betting.

  CChannel::doEvents(3.0);	// Let the channel jitter...

  ASSERT(firstupdate != good.getLastUpdate());

  ASSERT(string("") != good.getValue());

}
