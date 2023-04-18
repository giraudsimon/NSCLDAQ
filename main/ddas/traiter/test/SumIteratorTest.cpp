
#include <cppunit/extensions/HelperMacros.h>
#include "SumIteratorTest.h"
#include "Trace.hpp"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( SumIteratorTest );


void SumIteratorTest::setUp()
{
    null_tr = TraceS();

    std::vector<uint16_t> vec(2);
    vec[0] = 1;
    vec[1] = 2;

    // create a trivial trace
    simple_tr = TraceS(vec);


    std::vector<uint16_t> vec2(10);
    for (int i=0; i<vec2.size(); ++i) {
    if (i<5)    
        vec2[i] = 1;
    else
        vec2[i] = 2;
    }
    step_tr = TraceS(vec2);

    std::vector<uint16_t> vec3(100);
    for (int i=0; i<vec3.size(); ++i) {
        vec3[i] = i;
    }
    line_tr = TraceS(vec3);

}

void SumIteratorTest::tearDown()
{
}

void SumIteratorTest::testConstructor()
{
    TrRangeS range(step_tr.begin(),10);
    SumIterS it(range);

    CPPUNIT_ASSERT( it.m_range == range );
    CPPUNIT_ASSERT( 15 == it.m_sum );
}


void SumIteratorTest::testIsDoneOnNullTrace()
{
    SumIterS it ( TrRangeS(null_tr.begin(), null_tr.end()) );

    CPPUNIT_ASSERT( !(it<SumIterS( TrRangeS(null_tr.begin(), null_tr.end()))) );
    CPPUNIT_ASSERT( it.m_sum == 0 );
}

void SumIteratorTest::testIncrement()
{
    SumIterS it ( TrRangeS(step_tr.begin(), 4) );

//    it.MoveTo(3);
    ++it;
    ++it;
    ++it;
    CPPUNIT_ASSERT( it.m_range.begin().m_iter == (step_tr.fData.data()+3));
    CPPUNIT_ASSERT( it.m_range.end().m_iter == (step_tr.fData.data()+7));
    CPPUNIT_ASSERT( 6 == it.m_sum );
    CPPUNIT_ASSERT( 6 == *it );
}

