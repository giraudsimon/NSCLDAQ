// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "io.h"
#include <sys/uio.h>
#include <vector>
#include <string.h>
#include <unistd.h>
#include "Asserts.h"

// The function we're testing is not exported via the header as it's
// intended to be private...though this trick allows us to see it.

namespace io {
extern struct iovec*
updateIov(struct iovec* iov, int& nItems, ssize_t nBytes);
}

// The code below provides a mock of writev for
// higher level tests of writeDataV and writeDataVUnlimited.
// The vector below describes the number of bytes each call
// should 'write'.  The results go into a vector of write blocks.
//  If there are additional bytes after the last writev, the final
//  writev 'writes' the rest of them.
//

static std::vector<ssize_t> writevsizes;
static size_t wvsizeIndex(0);
struct blockDescriptor {
  size_t nBytes;
  char*  pData;
};
static std::vector<blockDescriptor> writtenBlocks;

static void freeWrittenData() {
  for (int i =0; i < writtenBlocks.size(); i++) {
    delete []writtenBlocks[i].pData;
  }
  writtenBlocks.clear();
}

static size_t sizeData(const struct iovec* iovs, int iovcnt)
{
  size_t result(0);
  for (int i = 0; i < iovcnt; i++) {
    result += iovs[i].iov_len;
  }
  return result;
}

static void
gather(char* pDest, const struct iovec* iovs, int iovcnt, size_t nBytes)
{
  while (nBytes) {
    if (iovs <= 0) {
      CPPUNIT_FAIL("Tried to writev more data than described");
    }
    if (nBytes >= iovs->iov_len) {   // Copy the whole block.
      memcpy(pDest, iovs->iov_base, iovs->iov_len);
      nBytes -= iovs->iov_len;
      pDest += iovs->iov_len;
      iovs++;
      iovcnt--;
    } else {                         // done copy partial.
      memcpy(pDest, iovs->iov_base, nBytes);
      nBytes = 0;
    }
  }
}

extern "C" {
  ssize_t writev(int fd, const struct iovec* iovs, int iovcnt) {
    size_t nBytes;
    if (wvsizeIndex < writevsizes.size()) {
      nBytes = writevsizes[wvsizeIndex++];
    } else {
      nBytes = sizeData(iovs, iovcnt);
    }
    char* pDest = new char[nBytes];
    gather(pDest, iovs, iovcnt, nBytes);
    blockDescriptor writedata = {nBytes, pDest};
    writtenBlocks.push_back(writedata);
    
    return nBytes;
  }
}

class iovtest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(iovtest);
  CPPUNIT_TEST(full_1);
  CPPUNIT_TEST(full_2);
  
  CPPUNIT_TEST(partial_1);
  CPPUNIT_TEST(partial_2);
  CPPUNIT_TEST(partial_3);
  
  CPPUNIT_TEST(writev_1);
  CPPUNIT_TEST(writev_2);
  CPPUNIT_TEST(writev_3);
  CPPUNIT_TEST(writev_4);
  CPPUNIT_TEST(writevU_1);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    freeWrittenData();
    writevsizes.clear();
    wvsizeIndex = 0;
  }
  void tearDown() {
  }
protected:
  void full_1();
  void full_2();
  
  void partial_1();
  void partial_2();
  void partial_3();
  
  void writev_1();
  void writev_2();
  void writev_3();
  void writev_4();
  void writevU_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(iovtest);

void iovtest::full_1() {
  struct iovec iov {
    (void*)(nullptr), 100
  };
  int n = 1;
  io::updateIov(&iov, n, 100);
  
  EQ(0, n);
}

void iovtest::full_2() {
  struct iovec iov[2] {
    {(void*)(nullptr), 50},
    {(void*)(nullptr), 50}
  };
  int n = 2;
  io::updateIov(iov, n, 100);
  EQ(0, n);
}

void iovtest::partial_1() { // one vector element partially done.
  struct iovec iov {
    0, 100
  };
  int n = 1;
  io::updateIov(&iov, n, 50);
  EQ(1, n);
  EQ(size_t(50), iov.iov_len);
  EQ((void*)(50), iov.iov_base);
  
}
void iovtest::partial_2() {  // Two vector elements, the first is fully done:
  struct iovec iov[2]{
    {0, 100},
    {0, 200}
  };
  int n = 2;
  struct iovec* next = io::updateIov(iov, n, 100);
  EQ(1, n);
  EQ(&(iov[1]), next);
  EQ(size_t(200), next->iov_len);
  
}
void iovtest::partial_3() { // Two vector elements, first and part of second done.
  struct iovec iov[2]{
    {0, 100},
    {0, 200}
  };
  int n = 2;
  struct iovec* next = io::updateIov(iov, n, 200);  
  
  EQ(1, n);
  EQ(&(iov[1]), next);
  EQ(size_t(100), next->iov_len);
}


// Testing the actual writeDataV:

// One block completely written:

void iovtest::writev_1()
{
  char data[100];
  for (int i =0; i < 100; i++) {
    data[i] = i;
  }
  iovec v = {data, 100};
  
  io::writeDataV(0, &v, 1);
  EQ(size_t(1), writtenBlocks.size());
  EQ(size_t(100), writtenBlocks[0].nBytes);
  EQ(0, memcmp(data, writtenBlocks[0].pData, 100));
}
// One block written:
//   Four iovectors.  we write 2 blocks
//   - first one writes the midway through the second iov,
//   - second one writes the rest/

void iovtest::writev_2()
{
 char data[100];
  for (int i =0; i < 100; i++) {
    data[i] = i;
  }
  
  iovec v[4] = {
    {data, 25},
    {data+25, 25},
    {data+50, 25},
    {data+75, 25}
  };
  
  writevsizes.push_back(32);
  io::writeDataV(0, v, 4);
  
  EQ(size_t(2), writtenBlocks.size());
  EQ(size_t(32), writtenBlocks[0].nBytes);
  EQ(size_t(100-32), writtenBlocks[1].nBytes);
  EQ(0, memcmp(data, writtenBlocks[0].pData, 32));
  EQ(0, memcmp(data+32, writtenBlocks[1].pData, 100-32));
}
void iovtest::writev_3()
{
  // a few io vectors but we're only allowing one byte at a time.
  
  char data[100];
  for (int i =0; i < 100; i++) {
    data[i] = i;
  }
  
  iovec v[4] = {
    {data, 25},
    {data+25, 25},
    {data+50, 25},
    {data+75, 25}
  };
  
  for (int i =0; i < 100; i++) {
    writevsizes.push_back(1);
  }
  io::writeDataV(0, v, 4);
  
  EQ(size_t(100), writtenBlocks.size());
  for (int i =0; i < 100; i++) {
    EQ(size_t(1), writtenBlocks[i].nBytes);
    EQ(data[i], *writtenBlocks[i].pData);
  }
}
void iovtest::writev_4()
{
  // write 3 bytes at a time -- that'll require an odd one t finish.
  // and the write size isn't a multiple of the iov size.
  
  char data[100];
  for (int i =0; i < 100; i++) {
    data[i] = i;
  }
  // relatively prime to 3
  
  iovec v[4] = {
    {data, 23},
    {data+23, 27},
    {data+50, 27},
    {data+77, 23}
  };
  int nBlocks = 0;
  int total   = 100;
  while (total > 3) {
    nBlocks++;
    total -= 3;
    writevsizes.push_back(3);
  }
  if (total){
    nBlocks++;                 // Final partial block.
    writevsizes.push_back(total);
  }
  
  io::writeDataV(0, v, 4);
  
  EQ(size_t(nBlocks), writtenBlocks.size());
  char* p = data;
  for (int i =0; i < nBlocks; i++) {
    blockDescriptor b = writtenBlocks[i];
    EQ(size_t(writevsizes[i]), b.nBytes);
    EQ(0, memcmp(p, b.pData, b.nBytes));
    p += b.nBytes;
  }
}
// writeDataVUnlimited -  Check that this works properly.
//

void iovtest::writevU_1()
{
  long minimum = sysconf(_SC_IOV_MAX) + 1;  //minimum # iovs.
  int  nInts   = minimum*4 + 3;                 // beat on it a bit harder.
  int data[nInts];
  for (int i =0; i < nInts; i++) data[i] = i;
  
  iovec v[nInts];
  int* p = data;
  for (int i =0; i < nInts; i++) {
    v[i].iov_base = p++;
    v[i].iov_len  = sizeof(int);
  }
  
  io::writeDataVUnlimited(0, v, nInts);
  
  // Should be 5 blocks  of data (I think).
  
  EQ(size_t(5), writtenBlocks.size());
  char* pB = reinterpret_cast<char*>(data);
  for (int i =0; i < writtenBlocks.size(); i++) {
    EQ(0, memcmp(pB, writtenBlocks[i].pData, writtenBlocks[i].nBytes));
    pB += writtenBlocks[i].nBytes;
  }
  
}