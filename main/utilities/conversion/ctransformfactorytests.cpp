/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>
#include "Asserts.h"

#define private public
#define protected public
#include <CTransformFactory.h>
#undef protected
#undef private

#include <stdexcept>

using namespace std;
using namespace DAQ::Transform;

// Forawd declarations for the dummy transform...
namespace DAQ {
  namespace V10 {
    class CRingItem;
  }

  namespace V11 {
    class CRingItem;
  }
}

//
struct DumbyTransform {
  using InitialType = DAQ::V10::CRingItem;
  using FinalType   = DAQ::V11::CRingItem;
  FinalType operator()(InitialType& item) { return FinalType(0); }
};


//
class CTransformFactoryTests : public CppUnit::TestFixture
{
private:
  CTransformFactory factory;

public:
    CPPUNIT_TEST_SUITE(CTransformFactoryTests);
    CPPUNIT_TEST(create_0);
    CPPUNIT_TEST(create_1);
    CPPUNIT_TEST(create_2);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
      factory = CTransformFactory();

    }
  void tearDown() {
  }
protected:
void create_0() {
  factory.setCreator( 10, 11, ( new CGenericCreator<DumbyTransform>()));
  CPPUNIT_ASSERT_NO_THROW_MESSAGE(
      "Create called with valid vsn specifiers should not throw",
      factory.create(10, 11)
  );

}

void create_1() {
  factory.setCreator( 10, 11, ( new CGenericCreator<DumbyTransform>()));
  CPPUNIT_ASSERT_MESSAGE(
      "Create called with valid vsn specifiers should return non-null",
      nullptr != factory.create(10, 11)
  );

}
void create_2() {
  CPPUNIT_ASSERT_THROW_MESSAGE(
      "Create called with invalid vsn specifiers should throw",
      factory.create(10, 11) , std::out_of_range
  );

}

}; // end of non-incr scaler tests

CPPUNIT_TEST_SUITE_REGISTRATION(CTransformFactoryTests);

