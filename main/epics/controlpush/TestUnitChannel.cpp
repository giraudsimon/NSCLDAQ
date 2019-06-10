// Tests for channels that may have units.
// This tests channesl that both have and don't have
// units.
#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "CUnitChannel.h"


using namespace std;



class TestUnitChannel : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestUnitChannel);
  CPPUNIT_TEST(Construction);
  CPPUNIT_TEST(Connection);
  CPPUNIT_TEST(HaveUnits);
  CPPUNIT_TEST(UnitValue);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
  }
protected:
  void Construction();
  void Connection();
  void HaveUnits();
  void UnitValue();
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestUnitChannel);

// Channel names below are nscl specific.
//    haveunits is a device with units (volts).
//    havnounits is a device without units.
// Units have an associated constant channel
//     name.EGU  (EGU - EnGineering Units).
//

const string haveunits("Z001DV");
const string havenounits("STRCHAN100");

void
TestUnitChannel::Construction()
{
  CUnitChannel channel(haveunits);

  EQMSG("Name", haveunits, channel.getName());
  
  string unitsname = haveunits + ".EGU";

  EQMSG("name", unitsname, channel.getUnitsName());

}

void
TestUnitChannel::Connection()
{
  CUnitChannel channel(haveunits);
  channel.Connect();

  CChannel::doEvents(0.5);	// Connct

  ASSERT(channel.isConnected());
  ASSERT(channel.getUnitsChannel()->isConnected());

  CUnitChannel nounits(havenounits);
  nounits.Connect();
  CChannel::doEvents(0.5);
  ASSERT(nounits.isConnected());
  ASSERT(!(nounits.getUnitsChannel()->isConnected()));
}
void
TestUnitChannel::HaveUnits()
{
  CUnitChannel have(haveunits);
  CUnitChannel havenot(havenounits);

  have.Connect();
  havenot.Connect();
  CChannel::doEvents(0.5);

  ASSERT(have.hasUnits());
  ASSERT(!havenot.hasUnits());

}

void
TestUnitChannel::UnitValue()
{
  CUnitChannel have(haveunits);
  CUnitChannel havenot(havenounits);

  have.Connect();
  havenot.Connect();

  CChannel::doEvents(0.5);

  EQMSG(haveunits, string("Amps"), have.getUnits());
  EQMSG(havenounits, string(""), havenot.getUnits());
}
