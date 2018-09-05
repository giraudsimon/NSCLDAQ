// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "fragment.h"

namespace EVB {
extern void resetFragmentPool();            // Frees fragments in pools.
}

class Fragalloctest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Fragalloctest);
  CPPUNIT_TEST(headerpool);
  
  CPPUNIT_TEST(bodypool_1);
  CPPUNIT_TEST(bodypool_2);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
    EVB::resetFragmentPool();
  }
  void tearDown() {
  }
protected:
  void headerpool();
  
  void bodypool_1();
  void bodypool_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Fragalloctest);

// Allocating a header,releasing it and allocating again should give the
// same header
void Fragalloctest::headerpool() {
  EVB::FragmentHeader hdr = {0, 0, 100, 0};
  EVB::pFragment first = allocateFragment(&hdr);
  freeFragment(first);
  EVB::pFragment second = allocateFragment(&hdr);
  
  EQ(first, second);
  
  freeFragment(second);
}

// Allocating/freeing/allocating same granule set gives same results.

void Fragalloctest::bodypool_1()
{
  EVB::FragmentHeader hdr = {0, 0, 100, 0};
  EVB::pFragment frag = allocateFragment(&hdr);
  void* pBody1 = frag->s_pBody;
  
  freeFragment(frag);
  
  frag = allocateFragment(&hdr);
  EQ(pBody1, frag->s_pBody);
  
  freeFragment(frag);
  
}
//  Same as bodypool_1, but the next allocation should come from a different
//  granule size and hence the pointers should not be the same for the
//  bodies but should be the same for the headers:

void Fragalloctest::bodypool_2()
{
  EVB::FragmentHeader hdr = {0,0, 10, 0};
  EVB::pFragment frag1 = allocateFragment(&hdr);
  void* pBody1 = frag1->s_pBody;
  freeFragment(frag1);
  
  hdr.s_size = 257;                 // DIfferent pool from 100.
  EVB::pFragment frag2 = allocateFragment(&hdr);
  
  EQ(frag1, frag2);
  ASSERT(pBody1 != frag2->s_pBody);
  
  freeFragment(frag2);
}