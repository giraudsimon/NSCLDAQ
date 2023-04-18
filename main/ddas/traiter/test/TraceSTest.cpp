

#include <cppunit/extensions/HelperMacros.h>
#include "TraceSTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( TraceSTest );


void TraceSTest::setUp()
{
    // trace fixtrue
    trace = new TraceT<uint16_t>;

    vec.resize(3);
    // vector of infor
    vec[0]=0;
    vec[1]=1;
    vec[2]=2;
}

void TraceSTest::tearDown()
{
    delete trace;
    vec.clear();
}

void TraceSTest::testLength()
{
     CPPUNIT_ASSERT(0 == trace->GetLength());
}

void TraceSTest::testArrayAccess()
{
    // fill the trace with some data  
    TraceT<uint16_t> tr(vec);

    CPPUNIT_ASSERT(double(1) == tr[1]);
}


void TraceSTest::testConstructor1()
{
    TraceT<uint16_t>* mtr = new TraceT<uint16_t>(vec);
    TraceT<uint16_t>& tr = *mtr;

    // Check that the trace has the same length
    // and the same data members
    CPPUNIT_ASSERT(3 == tr.GetLength());
    
    CPPUNIT_ASSERT(vec[0] == tr[0]);
    CPPUNIT_ASSERT(vec[1] == tr[1]);
    CPPUNIT_ASSERT(vec[2] == tr[2]);

    delete mtr;
}
