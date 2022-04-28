// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <CVMUSBusb.h>
#include <CVMUSBReadoutList.h>
#include <iostream>
#include <algorithm>
#include <vector>
#include <iomanip>
#include <iterator>
#include <numeric>

#include <stdio.h>
#include <stdlib.h>


#include "Asserts.h"

using namespace std;

// If your memory is in a different place.. change the three lines
// below:

static Warning msg("vmeTests requires an XLM72V in slot 3");
static const uint32_t vmebase = 0x80000000;
static const uint8_t  amod    = CVMUSBReadoutList::a32UserData;

static const uint32_t blockBase = 0x80000000;
static const uint8_t a32amod  = CVMUSBReadoutList::a32UserData;
static const uint8_t blkamod  = CVMUSBReadoutList::a32UserBlock;


class vmeTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(vmeTests);
  CPPUNIT_TEST(rdwr32);
  CPPUNIT_TEST(rdwr16);
  CPPUNIT_TEST(rdwr8);
  CPPUNIT_TEST(rdwr8);
  CPPUNIT_TEST(iterativeBlockRead);
  CPPUNIT_TEST(fifoTest);
  CPPUNIT_TEST(iterativeVariableBlockRead);
  CPPUNIT_TEST(iterativeBlockWrite);
  CPPUNIT_TEST_SUITE_END();

private:
  struct usb_device*   m_dev;
  CVMUSB*  m_pInterface;
public:
  void setUp() {
    vector<struct usb_device*> devices = CVMUSB::enumerate();
    if (devices.size() == 0) {
      cerr << " NO USB interfaces\n";
      exit(0);
    }
    m_pInterface = new CVMUSBusb(devices[0]);
    
    uint32_t one=1;
    m_pInterface->vmeWrite32(vmebase | 0x00800000,amod,one);
    m_pInterface->vmeWrite32(vmebase | 0x0080000c,amod,one);
  }
  void tearDown() {
    uint32_t zero=0;
    m_pInterface->vmeWrite32(vmebase | 0x00800000,amod,zero);
    m_pInterface->vmeWrite32(vmebase | 0x0080000c,amod,zero);
    delete m_pInterface;
  }
protected:
  void rdwr32();
  void rdwr16();
  void rdwr8();
  void iterativeBlockRead();
  void iterativeVariableBlockRead();
  void fifoTest();
  void iterativeBlockWrite();
  void blockWrite(uint32_t base_addr, vector<uint32_t>& data);


  void blockReadTest(uint32_t startAddr, size_t ntransfers, 
                     vector<uint32_t>& pattern);

  void variableBlockRead32Test(uint32_t countAddr, 
                               uint32_t readAddr, 
                               uint32_t ntransfers, 
                               vector<uint32_t>& pattern);

  vector<uint32_t> writeCyclicPattern(uint32_t startAddr, uint32_t startVal, 
                                      size_t nTransfers);
};

CPPUNIT_TEST_SUITE_REGISTRATION(vmeTests);


// 32 bit read/writes (can't test one without the other.
// We're going to roll a bit around the 32 bits.
void vmeTests::rdwr32() {
  int iter=0;
  uint32_t bit = 1;
  while (bit && iter<32) {

    m_pInterface->vmeWrite32(vmebase, amod, bit);
    uint32_t value;
    int status = m_pInterface->vmeRead32(vmebase, amod, &value);
    EQMSG("status", 0, status);
    EQMSG("value", bit, value);

    bit = bit << 1;
    ++iter;
  }

  
}
// 16 bit reads/writes run the bit in the opposite direction of the
// 32 bit version.
void vmeTests::rdwr16() {
  uint16_t bit = 0x8000;
  while (bit) {
    m_pInterface->vmeWrite16(vmebase, amod, bit);
    uint16_t value;
    int status = m_pInterface->vmeRead16(vmebase, amod, &value);
    EQMSG("status", 0, status);
    EQMSG("value", bit, value);
    bit = bit >> 1;
  }
}
// 8 bit reads/writes.  there are few enough patterns that we can do
// all of them.

void vmeTests::rdwr8() {
  uint8_t pattern;
  for (int p = 0; p <= 255; p++) {
    pattern = p;
    m_pInterface->vmeWrite8(vmebase, amod, pattern);
    uint8_t value;
    int status = m_pInterface->vmeRead8(vmebase, amod, &value);
    EQMSG("status", 0, status);
    EQMSG("value", pattern, value);
  }
}
// For block read we need to test a few cases:
//    < 64 (less than one block) - is non MB
//    > 64 multiple of 64        - is MB
//    > 64 not multiple of 64    - MB + odd one left over.
// Block read o every transfer count between 1 and 8192. This is exhaustive and
// takes a VERY long time. 
//
void vmeTests::iterativeBlockRead()
{
//  size_t maxTransfers=8192;
  size_t maxTransfers=4096;

  vector<uint32_t> pattern = writeCyclicPattern(blockBase,0,maxTransfers);
  for (size_t transfers=1; transfers<maxTransfers; transfers++) {
      // only try for integral multiples of block transfers 
      if ((transfers%128) != 0) {
        blockReadTest(blockBase, transfers, pattern);
      }
      
  }
}

// To test the variable block read, we will do a similar test
// to iterate through every possible scenario. This is slightly different
// because we will write the number of transfers to use into memory
// and then use it to setup the transfer.
void vmeTests::iterativeVariableBlockRead()
{
//  size_t maxTransfers=8192;
  size_t maxTransfers=4096;
  vector<uint32_t> pattern = writeCyclicPattern(blockBase+1*sizeof(uint32_t), 
                                                1, 
                                                maxTransfers);

  for (uint32_t transfers=1; transfers<maxTransfers; ++transfers) {
    if ((transfers%128) != 0) {
      variableBlockRead32Test(blockBase, blockBase+1*sizeof(uint32_t), 
                              transfers, pattern);
    }

  }
}
//  Fifo reads just read the memory without autoinc.  uh... since the
//  Board is responsible for handling addresses within a block what we will
//  do is write 180 longs to memory, we will then read 180 longs with
//  FIFO mode. What I >think< we should see is 0..63,0..63,0..51
//
void vmeTests::fifoTest()
{
  const size_t patternSize(180);
  uint32_t     pattern[patternSize];
  for (int i =0; i < patternSize; i ++) {
    pattern[i] = i;
    m_pInterface->vmeWrite32(blockBase+i*sizeof(uint32_t), a32amod, pattern[i]);
  }
  uint32_t rdblock[patternSize];
  size_t   transferred;
  m_pInterface->vmeFifoRead(blockBase, blkamod, rdblock, patternSize, &transferred);
  EQMSG("count", patternSize, transferred);

  for (int i=0; i < patternSize; i++) {
    char msg[100];
    sprintf(msg, "offset 0x%04x", i);
    EQMSG(msg, pattern[i % 64], rdblock[i]);
  }
}


// To test the variable block read, we will do a similar test
// to iterate through every possible scenario. This is slightly different
// because we will write the number of transfers to use into memory
// and then use it to setup the transfer.
void vmeTests::iterativeBlockWrite()
{

  size_t maxTransfers=4096;

  for (size_t transfers=2; transfers<maxTransfers; transfers++) {

//    cout << "Testing BLT write : " << transfers << endl;
    // create a vec and fill it with a sequence of values
    vector<uint32_t> pattern (transfers);
    iota(pattern.begin(), pattern.end(), 1);
//    copy(pattern.begin(), pattern.end(), ostream_iterator<uint32_t>(cout,"\n"));

    if ((transfers%128)!=0) {
      blockWrite(blockBase,pattern);
      blockReadTest(blockBase, transfers, pattern);
    }      
  }
}
//! blockReadTest 
//
//  The common code for a single block read test.  We demand the following:
//  1. The return status is 0
//  2. The transfer count read back is correct
//  3. The data transfered is what we expect
//
//  \param startAddr where to begin the block transfer from
//  \param ntransfers how many transfers to attempt
//  \param pattern the data to compare against
//
void vmeTests::blockReadTest(uint32_t startAddr, size_t ntransfers, 
                             vector<uint32_t>& pattern)
{
  const size_t patternSize(ntransfers);

  uint32_t rdblock[patternSize];
  size_t   transferred;
  int status = m_pInterface->vmeBlockRead(startAddr, blkamod, rdblock, 
                                          patternSize, &transferred);
  EQMSG("return status", 0, status);
  EQMSG("transfercount", patternSize, transferred);
  for(int i =0; i < patternSize; i++) {
    EQMSG("compare", pattern[i], rdblock[i]);
  }

}

//! blockWriteTest 
//
void vmeTests::blockWrite(uint32_t base_addr, std::vector<uint32_t>& data)
{

  size_t data_size = data.size();
  CVMUSBReadoutList rdolist;
  rdolist.addBlockWrite32(base_addr,0x0b,data.data(),data_size);

  auto stack = rdolist.get();
/*
  cout.flags(ios::hex);
  cout.fill('0');
  copy(stack.begin(), stack.end(), ostream_iterator<uint32_t>(cout,"\n"));
  cout.fill(' ');
  cout.flags(ios::dec);

  cout << "---" << endl;
*/

  uint32_t rdblock[data_size];
  size_t   transferred;
  int status = m_pInterface->executeList(rdolist,rdblock,data_size,
                                          &transferred);
  EQMSG("return status", 0, status);

}
//! variableBlockReadTest 
//
//  The common code for a single variable block read test.  We demand the following:
//  1. The return status is 0
//  2. The transfer count read back is correct
//  3. The data transfered is what we expect
//
//  This writes the number of transfers to the countaddress and then set up the 
//  the block transfer to use that value.
//
//  \param countAddr where to store the transfer count
//  \param startAddr where to begin the block transfer from
//  \param ntransfers how many transfers to attempt
//  \param pattern the data to compare against
//
void vmeTests::variableBlockRead32Test(uint32_t countAddr, uint32_t readAddr, 
                                       uint32_t ntransfers, 
                                       vector<uint32_t>& pattern)
{
  const size_t patternSize(ntransfers+2);
  uint32_t     rdblock[patternSize];
  uint32_t     rdbkTransfers;
  size_t       transferred;
  
  // Write  and read back the number of transfers to use
  int status = m_pInterface->vmeWrite32(countAddr, a32amod, ntransfers);
  EQMSG("Setup vmeWrite32 return status", 0, status);
  status = m_pInterface->vmeRead32(countAddr, a32amod, &rdbkTransfers);
  EQMSG("Setup vmeRead32 return status", 0, status);
  EQMSG("Setup read-back verification", ntransfers, rdbkTransfers);

  // set up the list for the variable block read
  CVMUSBReadoutList list;
  list.addBlockCountRead32(countAddr,0xffffff,a32amod);
  list.addMaskedCountBlockRead32(readAddr,blkamod);

  // Execute the list
  status = m_pInterface->executeList(list,rdblock,sizeof(rdblock),&transferred);

  EQMSG("Block count read return status", 0, status);
  EQMSG("Transfer count", size_t(ntransfers+1), transferred/sizeof(uint32_t));
  for(int i=0; i<ntransfers; ++i) {
    EQMSG("Data comparison", pattern[i], rdblock[i+1]);
  }

}

/*! writeCyclicPattern
*
*  For a range of addresses write a steadily increasing value into each address.
*
*   \param baseAddr the address in which to start writing
*   \param startVal the starting value for the writes 
*   \param nTransfers the number of memory segments to write 
*
*   \return an image of the memory
*/
vector<uint32_t> vmeTests::writeCyclicPattern(uint32_t baseAddr, uint32_t startVal, size_t nTransfers)
{
  vector<uint32_t>  pattern(nTransfers);

  for (int i = 0; i<nTransfers; i++) {
    pattern[i] = startVal;
    m_pInterface->vmeWrite32(baseAddr+i*sizeof(uint32_t), a32amod, pattern[i]);
    ++startVal;
  }

  return pattern;
}
