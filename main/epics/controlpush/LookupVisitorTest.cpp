// Template for a test suite.

#include <config.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CUnitChannel.h"
#include "CChannelList.h"
#include "CLookupVisitor.h"
#include "CBuildChannelData.h"


using namespace std;





class LookupVisitorTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LookupVisitorTests);
  CPPUNIT_TEST(TestLookup);
  CPPUNIT_TEST(BuildDataTest);
  CPPUNIT_TEST_SUITE_END();


private:
  CChannelList*  m_pChannels;
public:
  void setUp() {
    m_pChannels = new CChannelList;
    Stock();
  }
  void tearDown() {
    delete m_pChannels;
  }
protected:
  void Stock();
  void TestLookup();
  void BuildDataTest();

};

CPPUNIT_TEST_SUITE_REGISTRATION(LookupVisitorTests);

const string Names[] = {
  "Z002DH",
  "Z001DV",
  "FLTCHAN59",
  "FLTCHAN67",
  "FLTCHAN70"  
};
const int nNames = (sizeof(Names)/sizeof(string));

void
LookupVisitorTests::Stock()
{
  for (int i =0; i < nNames; i++) {
    CUnitChannel* pChannel = new CUnitChannel(Names[i]);
    m_pChannels->push_back(*pChannel);
  }
}

void
LookupVisitorTests::TestLookup()
{
  CLookupVisitor v;
  m_pChannels->foreach(v);
  CChannel::doEvents(0.5);

  // By now, everyone should be connected.

  CChannelList::ChannelIterator p = m_pChannels->begin();
  while(p != m_pChannels->end()) {
    EQMSG((*p)->getName(), true, (*p)->isConnected());

    p++;
  }
}

void 
LookupVisitorTests::BuildDataTest()
{
  CLookupVisitor v;
  m_pChannels->foreach(v);

  CChannel::doEvents(0.5);	// Connect and data should be there.

  CBuildChannelData  bv;
  m_pChannels->foreach(bv);	// Collect channel data.

  CBuildChannelData::ChannelItemIterator  p = bv.begin();
  int i = 0;
  while(p != bv.end()) {
    EQ(Names[i], p->first);
    CBuildChannelData::ChannelData info = p->second;
    ASSERT(info.m_sValue != "");
    // ASSERT(info.m_sUnits != "");
    ASSERT(info.m_Updated != 0);
    p++;
    i++;
  }
}
