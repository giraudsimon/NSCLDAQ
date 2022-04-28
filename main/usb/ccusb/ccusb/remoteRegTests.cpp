// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <usb.h>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <iomanip>

#define protected public
#define private public
#include <CCCUSBRemote.h>
#undef protected 
#undef private

using namespace std;

static Warning msg(string("regTests requires at least one CC-USB interface"));

class RemoteRegisterTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RemoteRegisterTests);
//  CPPUNIT_TEST(action);
  CPPUNIT_TEST(fwid);
  CPPUNIT_TEST(globmode);
  CPPUNIT_TEST(delays); //< This hangs the CCUSB up and make it unhappy
  CPPUNIT_TEST(ledsrc);
  CPPUNIT_TEST(output);
  CPPUNIT_TEST(devsrc);
  CPPUNIT_TEST(dgg);
  CPPUNIT_TEST(lammask);
  CPPUNIT_TEST(bulksetup);
  CPPUNIT_TEST(c);
  CPPUNIT_TEST(z);
  CPPUNIT_TEST(inhibit);
  CPPUNIT_TEST(uninhibit);
  CPPUNIT_TEST(executeList0);
  CPPUNIT_TEST_SUITE_END();


private:
  struct usb_device*   m_dev;
  CCCUSB*  m_pInterface;
//  const CCCUSB::ShadowRegisters* m_pShadow;

public:
  void setUp() {
    m_pInterface = new CCCUSBRemote("test","localhost",27000);
//    m_pShadow = &m_pInterface->getShadowRegisters();
  }
  void tearDown() {
    delete m_pInterface;
//    m_pShadow=0;
  }
protected:
  void action();
  void fwid();
  void globmode();
  void delays();
  void scalercontrol();
  void ledsrc();
  void output();
  void devsrc();
  void dgg();
  void lammask();
  void bulksetup();

  void c();
  void z();
  void inhibit();
  void uninhibit();
  void executeList0();
};

CPPUNIT_TEST_SUITE_REGISTRATION(RemoteRegisterTests);

//write action register... can only determine that no throws happen.

//void RemoteRegisterTests::action() {
//  m_pInterface->writeActionRegister(0);	// this is really the only safe thing to write here.
				   
//}

// Read firmware: can only output and hope no exceptions.

void RemoteRegisterTests::fwid()
{
  uint32_t fw = m_pInterface->readFirmware();
  cout << hex << setw(8) << setprecision(0) << fw << dec << endl;
}
// Write 0's to the gobal mode register... all the 
// bits that count should match.. then turn them all  on too.
//
void RemoteRegisterTests::globmode()
{
  uint16_t validmask = 0x17f;	// Bits that matter in the register.
  m_pInterface->writeGlobalMode(0);
//  EQMSG("wrote shadow 0", (uint16_t)0, m_pShadow->globalMode);

  uint16_t mode = m_pInterface->readGlobalMode();
  EQMSG("wrote 0", static_cast<uint16_t>(0), uint16_t(mode & validmask));
//  EQMSG("read 0", (uint16_t)(mode & validmask), m_pShadow->globalMode);

  m_pInterface->writeGlobalMode(validmask);
//  EQMSG("wrote shadow 0xffff", (uint16_t)0xffff, m_pShadow->globalMode);
  mode   = m_pInterface->readGlobalMode();
  EQMSG("wrote 1s", validmask, (uint16_t)(validmask & mode));
 // EQMSG("read shadow 0xffff", validmask, (uint32_t)(validmask & m_pShadow->globalMode));

  // This left us with an invalid bus request level so:

//  m_pInterface->writeGlobalMode((uint32_t)(4 << CCCUSB::GlobalModeRegister::busReqLevelShift));

}


// Test the delays 
void RemoteRegisterTests::delays() 
{
  m_pInterface->writeDelays(0xff);
//  EQMSG("wrote shadow ones", (uint16_t)0xffff, m_pShadow->delays);
  uint16_t settings = m_pInterface->readDelays();
  EQMSG("lower 16", (uint16_t)0xff, settings);
//  EQMSG("read shadow ones", (uint16_t)0xffff, m_pShadow->delays);
 
  m_pInterface->writeDelays(0xff00);
//  EQMSG("wrote shadow zeroes", (uint16_t)0, m_pShadow->delays);
  settings = m_pInterface->readDelays();
  EQMSG("upper 16", (uint16_t)0xff00, settings);
//  EQMSG("read shadow zeroes", (uint16_t)0, m_pShadow->delays);

  m_pInterface->writeDelays(0);
//  EQMSG("wrote shadow zeroes", (uint16_t)0, m_pShadow->delays);
  settings = m_pInterface->readDelays();
  EQMSG("zero", (uint16_t)0, settings);
//  EQMSG("read shadow zeroes", (uint16_t)0, m_pShadow->delays);
}

// Test the scaler control 
void RemoteRegisterTests::scalercontrol() 
{
  m_pInterface->writeScalerControl(0xff);
//  EQMSG("wrote shadow ones", (uint16_t)0xffff, m_pShadow->delays);
  uint16_t settings = m_pInterface->readScalerControl();
  EQMSG("lower 16", (uint16_t)0xff, settings);
//  EQMSG("read shadow ones", (uint16_t)0xffff, m_pShadow->delays);
 
  m_pInterface->writeScalerControl(0xff00);
//  EQMSG("wrote shadow zeroes", (uint16_t)0, m_pShadow->delays);
  settings = m_pInterface->readScalerControl();
  EQMSG("upper 16", (uint16_t)0xff00, settings);
//  EQMSG("read shadow zeroes", (uint16_t)0, m_pShadow->delays);

  m_pInterface->writeScalerControl(0);
//  EQMSG("wrote shadow zeroes", (uint16_t)0, m_pShadow->delays);
  settings = m_pInterface->readScalerControl();
  EQMSG("zero", (uint16_t)0, settings);
//  EQMSG("read shadow zeroes", (uint16_t)0, m_pShadow->delays);
}
// LEDSrc has somed dead bits.

void RemoteRegisterTests::ledsrc()
{
  uint32_t usedBits = 0x373737;

  m_pInterface->writeLedSelector(0xffffffff);
  uint32_t value = m_pInterface->readLedSelector();
  EQMSG("ones", usedBits, (value & usedBits));

  m_pInterface->writeLedSelector(0);
  value = m_pInterface->readLedSelector();

  EQMSG("0's", (uint32_t)0, (value & usedBits));

}

void RemoteRegisterTests::output()
{
  uint32_t usedBits = 0x373737;

  m_pInterface->writeOutputSelector(0xffffffff);
  uint32_t value = m_pInterface->readOutputSelector();
  EQMSG("ones", usedBits, (value & usedBits));

  m_pInterface->writeOutputSelector(0);
  value = m_pInterface->readOutputSelector();

  EQMSG("0's", (uint32_t)0, (value & usedBits));

}
// Dev src register also has some dead bits.

void RemoteRegisterTests::devsrc()
{
  uint32_t usedBits =  0x007071717; // 7777 not 77ff since reset is momentary.
  uint32_t clearBits = 0x00004040;
  uint32_t testBits  = 0x00000707;

  // disable scalers this must be done in a separate operation
  // from writing to selector bits
  m_pInterface->writeDeviceSourceSelectors(clearBits);
  m_pInterface->writeDeviceSourceSelectors(testBits);
//  EQMSG("wrote shadow ones", (uint32_t)usedBits, m_pShadow->deviceSources);
  uint32_t value = m_pInterface->readDeviceSourceSelectors();
  EQMSG("ones", testBits, (value & usedBits));
//  EQMSG("read shadow ones", (uint32_t)usedBits, (usedBits & m_pShadow->deviceSources));

  // Writing to the scalers and the dgg apparently need to be
  // done separately.
  testBits = 0x007070000;
  m_pInterface->writeDeviceSourceSelectors(testBits);
//  EQMSG("wrote shadow ones", (uint32_t)usedBits, m_pShadow->deviceSources);
  value = m_pInterface->readDeviceSourceSelectors();
  EQMSG("ones", testBits, (value & usedBits));
//  EQMSG("read shadow ones", (uint32_t)usedBits, (usedBits & m_pShadow->deviceSources));


  m_pInterface->writeDeviceSourceSelectors(0);
//  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->deviceSources);
  value = m_pInterface->readDeviceSourceSelectors();
  EQMSG("0's", (uint32_t)0, (value & usedBits));
//  EQMSG("read shadow zeroes", (uint32_t)0, (usedBits & m_pShadow->deviceSources));
}
// Three registers for the gate and delay register use all 32 bits.
// dgga, dggb, dggextended.

void RemoteRegisterTests::dgg()
{

  // DGG A control register.

  m_pInterface->writeDGGA(0xffffffff);
//  EQMSG("wrote shadow ones", (uint32_t)0xffffffff, m_pShadow->dggA);
  uint32_t value = m_pInterface->readDGGA();
  EQMSG("DGGA 1's", (uint32_t)0xffffffff, value);
//  EQMSG("read shadow ones", (uint32_t)0xffffffff, m_pShadow->dggA);

  m_pInterface->writeDGGA(0);
//  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->dggA);
  value = m_pInterface->readDGGA();
  EQMSG("DGGA 0's", (uint32_t)0, value);
//  EQMSG("read shadow zeroes", (uint32_t)0, m_pShadow->dggA);

  // DGG B control register.

  m_pInterface->writeDGGB(0xffffffff);
//  EQMSG("wrote shadow ones", (uint32_t)0xffffffff, m_pShadow->dggB);
  value = m_pInterface->readDGGB();
  EQMSG("DGGB 1's", (uint32_t)0xffffffff, value);
//  EQMSG("read shadow ones", (uint32_t)0xffffffff, m_pShadow->dggB);

  m_pInterface->writeDGGB(0);
//  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->dggB);
  value = m_pInterface->readDGGB();
  EQMSG("DGGB 0's", (uint32_t)0, value);
//  EQMSG("read shadow zeroes", (uint32_t)0, m_pShadow->dggB);

  // DGG Extended control register.

  m_pInterface->writeDGGExt(0xffffffff);
//  EQMSG("wrote shadow ones", (uint32_t)0xffffffff, m_pShadow->dggExtended);
  value = m_pInterface->readDGGExt();
  EQMSG("Extended 1's", (uint32_t)0xffffffff, value);
//  EQMSG("read shadow ones", (uint32_t)0xffffffff, m_pShadow->dggExtended);

  m_pInterface->writeDGGExt(0);
//  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->dggExtended);
  value = m_pInterface->readDGGExt();
  EQMSG("Extended 0's", (uint32_t)0, value);
//  EQMSG("read shadow zeroes", (uint32_t)0, m_pShadow->dggExtended);
  
}

//
// only some bits of the bulk transfer setup register are used.

void RemoteRegisterTests::lammask()
{
  uint32_t usedBits = 0xfff;
  m_pInterface->writeLamTriggers(0xffffffff);
//  EQMSG("wrote shadow onees", (uint32_t)0xffffffff, m_pShadow->bulkTransferSetup);
  uint32_t value = m_pInterface->readLamTriggers();
  EQMSG("1's", usedBits, (value & usedBits));
//  EQMSG("read shadow ones", usedBits, (usedBits & m_pShadow->bulkTransferSetup));

  m_pInterface->writeLamTriggers(0);
//  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->bulkTransferSetup);
  value = m_pInterface->readLamTriggers();
  EQMSG("0's", (uint32_t)0, (value & usedBits));
//  EQMSG("read shadow zeroes", (uint32_t)0, (usedBits & m_pShadow->bulkTransferSetup));
}
//
// only some bits of the bulk transfer setup register are used.

void RemoteRegisterTests::bulksetup()
{
  uint32_t usedBits = 0xfff;
  m_pInterface->writeUSBBulkTransferSetup(0xffffffff);
//  EQMSG("wrote shadow onees", (uint32_t)0xffffffff, m_pShadow->bulkTransferSetup);
  uint32_t value = m_pInterface->readUSBBulkTransferSetup();
  EQMSG("1's", usedBits, (value & usedBits));
//  EQMSG("read shadow ones", usedBits, (usedBits & m_pShadow->bulkTransferSetup));

  m_pInterface->writeUSBBulkTransferSetup(0);
//  EQMSG("wrote shadow zeroes", (uint32_t)0, m_pShadow->bulkTransferSetup);
  value = m_pInterface->readUSBBulkTransferSetup();
  EQMSG("0's", (uint32_t)0, (value & usedBits));
//  EQMSG("read shadow zeroes", (uint32_t)0, (usedBits & m_pShadow->bulkTransferSetup));
}

// There really isn't much to test here but we can at least call the function and see 
// that it succeeds
void RemoteRegisterTests::c()
{
  EQMSG("c() good return status", 0, m_pInterface->c());
}

// There really isn't much to test here but we can at least call the function and see 
// that it succeeds
void RemoteRegisterTests::z()
{
  EQMSG("z() good return status", 0, m_pInterface->z());
}

// that it succeeds
void RemoteRegisterTests::inhibit()
{
  EQMSG("inhibit() good return status", 0, m_pInterface->inhibit());
}

// that it succeeds
void RemoteRegisterTests::uninhibit()
{
  EQMSG("uninhibit() good return status", 0, m_pInterface->uninhibit());
}

// After a disconnection has occurred, make sure that the executeList doesn't 
// seg fault. 
void RemoteRegisterTests::executeList0()
{
  // force a disconnection
  static_cast<CCCUSBRemote*>(m_pInterface)->disconnect();

  CCCUSBReadoutList list;
  list.addMarker(0x0000);
  char buffer[128];
  size_t nbytes;
  int ret = m_pInterface->executeList(list,buffer, sizeof(buffer), &nbytes);

  EQMSG("executing list when disconnected", -4, ret);
}
