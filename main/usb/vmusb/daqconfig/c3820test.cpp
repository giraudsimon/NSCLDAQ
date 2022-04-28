// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <CVMUSB.h>
#include <CVMUSBReadoutList.h>
#include <CReadoutModule.h>
#include <C3820.h>
#include <iostream>
#include <errno.h>

using namespace std;

static const uint32_t base(0x01000000);
static const string   baseString("0x01000000");
static Warning msg("c3820test requires an SIS 3820 module base address 0x01000000");
static const uint8_t amod = CVMUSBReadoutList::a32UserData;

class c3820test : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(c3820test);
  CPPUNIT_TEST(attach);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST(init);
  CPPUNIT_TEST(read);
  CPPUNIT_TEST_SUITE_END();


private:
  C3820*          m_pModule;
  CReadoutModule* m_pConfig;

public:
  void setUp() {
    m_pModule = new C3820;
    m_pConfig = new CReadoutModule("test", *m_pModule);
  }
  void tearDown() {
    delete m_pConfig;
    delete m_pModule;

  }
protected:
  void attach();
  void config();
  void init();
  void read();
};

CPPUNIT_TEST_SUITE_REGISTRATION(c3820test);

// Attaching should give me a single configuration parameter: -base:
void c3820test::attach() {
  XXUSB::CConfigurableObject::ConfigurationArray citems = m_pConfig->cget();
  EQMSG("no. items", (size_t)1, citems.size());

  EQMSG("Config param name", string("-base"), citems[0].first);
}

// Should be able to configure the base address:

void c3820test::config()
{
  m_pConfig->configure("-base", baseString);
  EQ(baseString, m_pConfig->cget("-base"));
}

// Init should set the module up properly.


void c3820test::init()
{
  vector<struct usb_device*> interfaces = CVMUSB::enumerate();
  CVMUSB vme(interfaces[0]);

  m_pConfig->configure("-base", baseString);
  try {
    m_pConfig->Initialize(vme);
  }
  catch (string msg) {
    cerr << "c3820test::init init failed: " << msg << endl;
    throw;
  }

  uint32_t value;		// SIS3820 has 32 bit registers.
  vme.vmeRead32(base + 0x100, amod, &value);
  EQMSG("acqmode", (uint32_t)0x41000, value);

  vme.vmeRead32(base, amod, &value); // status reg.
  EQMSG("status", (unsigned int)0x010000, value & 0x1010000);
}
// First read the scaler.. no pulses on the input channel, so all chans
// should be zero (should get 32 longs).
// Second, enable the reference pulser and wait for 100us.  
// Read again.. first channel should be non zero rest zero.
//

void c3820test::read()
{
  vector<struct usb_device*> interfaces = CVMUSB::enumerate();
  CVMUSB vme(interfaces[0]);

  m_pConfig->configure("-base", baseString);
  try {
    m_pConfig->Initialize(vme);
  }
  catch (string msg) {
    EQMSG(msg.c_str(), true, false);

  }

  CVMUSBReadoutList list;
  m_pConfig->addReadoutList(list);

  uint32_t scalers[32 ];	// NOte, if I use too many
  size_t   readSize;		// I get a timeout from linux.


  int status = vme.executeList(list, scalers, sizeof(scalers), &readSize);
  if (status != 0) {
    char* msg = strerror(errno);
    EQMSG(msg, 0, status);

  }
  EQMSG("read-size", 32*sizeof(uint32_t), readSize);
  for(int i =0; i < 32; i++) {
    EQMSG("first result", (uint32_t)0, scalers[i]);
  }
}
