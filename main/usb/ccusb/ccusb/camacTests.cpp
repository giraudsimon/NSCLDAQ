// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <usb.h>
#include <CCCUSBusb.h>
#include <CCCUSBReadoutList.h>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <iomanip>


using namespace std;

static Warning msg(string("camactests requires a CC-USB interface and a Kinetic Systems 3821 in slot 5"));

class registerTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(registerTests);
  CPPUNIT_TEST(writeRead16_0);
  CPPUNIT_TEST(writeRead24_0);
  CPPUNIT_TEST(read_0);
  CPPUNIT_TEST(qstop_0);
  CPPUNIT_TEST(qscan_0);
  CPPUNIT_TEST(z);
  CPPUNIT_TEST(c);
  CPPUNIT_TEST(inhibit);
  CPPUNIT_TEST(uninhibit);
  CPPUNIT_TEST_SUITE_END();


private:
  struct usb_device*   m_dev;
  CCCUSB*  m_pInterface;

public:
  void setUp() {
    vector<struct usb_device*> devices = CCCUSBusb::enumerate();
    if (devices.size() == 0) {
      cerr << " NO USB interfaces\n";
      exit(0);
    }
    m_pInterface = new CCCUSBusb(devices[0]);
  }
  void tearDown() {
    delete m_pInterface;
  }
protected:
  void writeRead16_0();
  void writeRead24_0();
  void read_0();
  void qstop_0();
  void qscan_0();

  void c();
  void z();
  void inhibit();
  void uninhibit();
};

CPPUNIT_TEST_SUITE_REGISTRATION(registerTests);

//write action register... can only determine that no throws happen.

void registerTests::writeRead16_0() {
  uint16_t value, qx;
  int status = m_pInterface->simpleWrite16(5, 0, 17, 443, qx);
  EQMSG("qx", uint16_t(0x3), qx);

  status = m_pInterface->simpleRead16(5, 0, 1, value, qx);
  EQMSG("qx", uint16_t(0x3), qx);
  EQMSG("read back", uint16_t(443), value);
}

void registerTests::writeRead24_0() {
  uint16_t qx;
  uint32_t value;
  int status = m_pInterface->simpleWrite24(5, 1, 17, 0, qx);
  EQMSG("qx", uint16_t(0x3), qx);

  for (uint16_t i=0; i<1024; ++i) {
      status = m_pInterface->simpleWrite24(5, 1, 16, i+1, qx);
      EQMSG("qx", uint16_t(0x3), qx);
  }

  status = m_pInterface->simpleWrite24(5, 1, 17, 0, qx);
  EQMSG("qx", uint16_t(0x3), qx);

  for (uint16_t i=0; i<1024; ++i) {
      status = m_pInterface->simpleRead24(5, 1, 0, value, qx);
      EQMSG("qx", uint16_t(0x3), qx);
      EQMSG("value", uint32_t(i+1), value);
  }
}


void registerTests::read_0() {

    uint16_t qx, value;
    int status = m_pInterface->simpleWrite16(5, 1, 17, 0, qx);
    EQMSG("qx", uint16_t(0x3), qx);

    for (uint16_t i=0; i<1024; ++i) {
        status = m_pInterface->simpleWrite16(5, 1, 16, i+1, qx);
        EQMSG("qx", uint16_t(0x3), qx);
    }

    status = m_pInterface->simpleWrite16(5, 1, 17, 0, qx);
    EQMSG("qx", uint16_t(0x3), qx);

    for (uint16_t i=0; i<1024; ++i) {
        status = m_pInterface->simpleRead16(5, 1, 0, value, qx);
        EQMSG("qx", uint16_t(0x3), qx);
        EQMSG("value", uint16_t(i+1), value);
    }

}

void registerTests::qstop_0() {

  uint16_t qx, value;
  int status = m_pInterface->simpleWrite16(5, 0, 17, 0, qx);
  EQMSG("qx", uint16_t(0x3), qx);

  for (uint16_t i=0; i<1024; ++i) {
      status = m_pInterface->simpleWrite16(5, 0, 16, i, qx);
      EQMSG("qx", uint16_t(0x3), qx);
  }

  status = m_pInterface->simpleWrite16(5, 0, 17, 0, qx);
  EQMSG("qx", uint16_t(0x3), qx);

  CCCUSBReadoutList list;
  list.addQStop(5, 0, 0, 1025, false);

  uint16_t data[1028];
  size_t nRead;
  status = m_pInterface->executeList(list, data, sizeof(data), &nRead);

  for (uint16_t i=0; i<1024; ++i) {
    EQMSG("value", i, data[i]);
  }

  EQMSG("count", sizeof(uint16_t)*1025, nRead);
}


void registerTests::qscan_0() {

    int status;
    uint16_t value;
    uint16_t qx;

    m_pInterface->simpleWrite16(5, 0, 17, 0, qx);

    m_pInterface->simpleWrite16(5, 0, 16, 0, qx);
    m_pInterface->simpleWrite16(5, 0, 16, 1, qx);
    m_pInterface->simpleWrite16(5, 0, 16, 2, qx);
    m_pInterface->simpleWrite16(5, 0, 16, 3, qx);


    m_pInterface->simpleWrite16(5, 0, 17, 0, qx);
    m_pInterface->simpleWrite16(5, 1, 17, 1, qx);
    m_pInterface->simpleWrite16(5, 2, 17, 2, qx);
    m_pInterface->simpleWrite16(5, 3, 17, 3, qx);

    CCCUSBReadoutList list;
    list.addQScan(5,0,0,4);

    uint16_t data[16];
    size_t nRead;
    status = m_pInterface->executeList(list, data, sizeof(data), &nRead);

    EQMSG("nRead", sizeof(uint16_t)*4, nRead);
    for (int i=0; i<4; ++i) {
        EQMSG("value", uint16_t(i), data[i]);
    }
}


// There really isn't much to test here but we can at least call the function and see 
// that it succeeds
void registerTests::c()
{
  EQMSG("c() good return status", 0, m_pInterface->c());
}

// There really isn't much to test here but we can at least call the function and see 
// that it succeeds
void registerTests::z()
{
  EQMSG("z() good return status", 0, m_pInterface->z());
}

// that it succeeds
void registerTests::inhibit()
{
  EQMSG("inhibit() good return status", 0, m_pInterface->inhibit());
}

// that it succeeds
void registerTests::uninhibit()
{
  EQMSG("uninhibit() good return status", 0, m_pInterface->uninhibit());
}
