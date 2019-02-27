// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "ModuleReader.h"
#include "ZeroCopyHit.h"


class MReaderTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(MReaderTest);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(read_1);
  CPPUNIT_TEST(read_2);
  CPPUNIT_TEST(module);
  CPPUNIT_TEST_SUITE_END();


private:
  DDASReadout::ModuleReader* m_pTestObj;
public:
  void setUp() {
    m_pTestObj = new DDASReadout::ModuleReader(1, 4,0x1234, 10.0);
  }
  void tearDown() {
    delete m_pTestObj;
    m_pTestObj = nullptr;
  }
protected:
  void construct();
  void read_1();
  void read_2();
  void module();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MReaderTest);

void MReaderTest::construct() {
  EQ(unsigned(1), m_pTestObj->module());
}

// Single hit
void MReaderTest::read_1()
{
  DDASReadout::ModuleReader::HitList hits;
  m_pTestObj->read(hits, 4);
  EQ(size_t(1), hits.size());
  DDASReadout::ModuleReader::HitInfo& hit(hits[0]);
  EQ(m_pTestObj, hit.first);
  DDASReadout::ZeroCopyHit* pHit = hit.second;
  
  EQ(2, pHit->s_chanid);
  EQ(0.0, pHit->s_time);
  EQ(4, pHit->s_channelLength);
  hits.pop_front();
  DDASReadout::ModuleReader::freeHit(hit);
}

// several hits:

void MReaderTest::read_2()
{
  DDASReadout::ModuleReader::HitList hits;
  m_pTestObj->read(hits, 4*100);           // 100 hits.
  EQ(size_t(100), hits.size());
  double tstamp(0.0);                       // Note the calibration is 10.0ns/tick
  for(auto p = hits.begin(); p != hits.end(); p++, tstamp += 10.0) {
    EQ(m_pTestObj, p->first);
    EQ(2, p->second->s_chanid);
    EQ(tstamp, p->second->s_time);
    EQ(4, p->second->s_channelLength);
    DDASReadout::ModuleReader::freeHit(*p);
  }
}
void MReaderTest::module()
{
  EQ(1U, m_pTestObj->module());
}
// Make a hit:

unsigned long* makeHit(
    uint32_t* pData, unsigned crate, unsigned slot, unsigned chan,
    uint64_t timestamp
  )
{
  uint32_t desc = chan | (slot << 4) | (crate << 8) | (4 << 12) | (4 << 17);
  *pData++  = desc;
  *pData++  = timestamp & 0xffffffff;
  *pData++  = (timestamp >> 32) & 0xffff;
  *pData++  = 0x5a5a;                       // "energy"
  
  return (unsigned long*)(pData);                 // Next unused word.
}

// Fake pixie reader.

extern "C" {

  static const unsigned EventLength(4);    // For now just the header.

  int Pixie16ReadDataFromExternalFIFO(
    unsigned long* pData, unsigned long nFIFOWords, unsigned short mNum
  )
  {
    EQ(unsigned(4), EventLength);
    ASSERT((nFIFOWords % 4) == 0);            // Data must be multiple of event len.
    uint64_t ts = 0;
    while (nFIFOWords) {
      pData = makeHit((uint32_t*)(pData), 0, 1, 2, ts++);    // ts is an event number for this.
      nFIFOWords -= EventLength;
    }
    
    return 0;
  }
}