
#include <cppunit/extensions/HelperMacros.h>
#include "TrapFilterTest.h"
#include "Trace.hpp"
#include "SumIterator.hpp"
#include <iterator>
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( TrapFilterTest );


void TrapFilterTest::setUp()
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

    std::vector<uint16_t> vec4(20);
    for (int i=0; i<vec4.size(); ++i) {
    if (i<5)    
        vec4[i] = 10;
    else
        vec4[i] = 2;
    }
    step_trlong = TraceS(vec4);

}

void TrapFilterTest::tearDown()
{
}

void TrapFilterTest::testConstructor()
{

    TrFilterS filter(line_tr.begin(),5,1);

    TrRangeS lowrange (line_tr.begin(),5);

    SumIterator<uint16_t> lowsum (lowrange);

    TrRangeS leadrange = lowrange;
    SumIterator<uint16_t> leadsum (leadrange+=6);

    CPPUNIT_ASSERT( filter.m_trailsum.min_extent() == line_tr.begin() );
    CPPUNIT_ASSERT( filter.m_trailsum == lowsum );
    CPPUNIT_ASSERT( 10 == *(filter.m_trailsum) );

    CPPUNIT_ASSERT( filter.m_leadsum == leadsum );
    CPPUNIT_ASSERT( 40 == *(filter.m_leadsum) );

    CPPUNIT_ASSERT( 30 == *filter );

}

void TrapFilterTest::testIncrement()
{
    TrFilterS filter (step_tr.begin(), 2, 1) ;

    CPPUNIT_ASSERT( 0 == *filter );

    ++filter;
    CPPUNIT_ASSERT( 1 == *filter );

    ++filter;
    CPPUNIT_ASSERT( 2 == *filter );

    ++filter;
    CPPUNIT_ASSERT( 2 == *filter );

    ++filter;
    CPPUNIT_ASSERT( 1 == *filter );

}

void TrapFilterTest::testMinExtent()
{
    TrFilterS filter (step_tr.begin()+1, 2, 1) ;
    
    CPPUNIT_ASSERT( filter.min_extent() == step_tr.begin()+1);
    CPPUNIT_ASSERT( filter.min_extent() == filter.m_trailsum.m_range.begin());

    filter.m_trailsum.m_range.invert();
    CPPUNIT_ASSERT( filter.min_extent() == step_tr.begin()+1);
    CPPUNIT_ASSERT( filter.min_extent() == filter.m_trailsum.m_range.end());

//    --filter.m_leadsum;
//    --filter.m_leadsum;
//    --filter.m_leadsum;
//    --filter.m_leadsum;
    SumIterator<uint16_t> sum(TrRangeS(step_tr.begin(),2));
    filter.m_leadsum = sum;
   
    CPPUNIT_ASSERT( filter.min_extent() == step_tr.begin());
    CPPUNIT_ASSERT( filter.min_extent() == filter.m_leadsum.m_range.begin());

    filter.m_leadsum.m_range.invert();
    CPPUNIT_ASSERT( filter.min_extent() == step_tr.begin());
    CPPUNIT_ASSERT( filter.min_extent() == filter.m_leadsum.m_range.end());

}

void TrapFilterTest::testMaxExtent()
{
    TrFilterS filter (step_tr.begin()+1, 2, 1) ;
    
    CPPUNIT_ASSERT( filter.max_extent() == step_tr.begin()+6);
    CPPUNIT_ASSERT( filter.max_extent() == filter.m_leadsum.m_range.end());

    filter.m_leadsum.m_range.invert();
    CPPUNIT_ASSERT( filter.max_extent() == step_tr.begin()+6);
    CPPUNIT_ASSERT( filter.max_extent() == filter.m_leadsum.m_range.begin());

    ++filter.m_trailsum;
    ++filter.m_trailsum;
    ++filter.m_trailsum;
    ++filter.m_trailsum;
   
    CPPUNIT_ASSERT( filter.max_extent() == step_tr.begin()+7);
    CPPUNIT_ASSERT( filter.max_extent() == filter.m_trailsum.m_range.end());

    filter.m_trailsum.m_range.invert();
    CPPUNIT_ASSERT( filter.max_extent() == step_tr.begin()+7);
    CPPUNIT_ASSERT( filter.max_extent() == filter.m_trailsum.m_range.begin());

}

void TrapFilterTest::testWhile()
{
    TrFilterS filter (step_tr.begin(), 2, 1) ;
    
    CPPUNIT_ASSERT_NO_THROW( while ( filter < step_tr.end() ) ++filter;);
}

void TrapFilterTest::testNegative()
{

    // step_trlong is a trace with
    // index : 0  1  2  3  4  5  6  7  8  9 10 11 ...
    // value :10 10 10 10 10  2  2  2  2  2  2  2 ...
    TrFilterS filter (step_trlong.begin(), 2, 1) ;

    // index : 0  1  2  3  4  5  6  7  8  9 10 11 ...
    // value :10 10 10 10 10  2  2  2  2  2  2  2 ...
    //       |-----|  |-----|
    CPPUNIT_ASSERT( 0 == *filter );

    ++filter;
    
    // index : 0  1  2  3  4  5  6  7  8  9 10 11 ...
    // value :10 10 10 10 10  2  2  2  2  2  2  2 ...
    //          |-----|  |-----|
    CPPUNIT_ASSERT( -8.0 == *filter );

    ++filter;
    // index : 0  1  2  3  4  5  6  7  8  9 10 11 ...
    // value :10 10 10 10 10  2  2  2  2  2  2  2 ...
    //             |-----|  |-----|
    CPPUNIT_ASSERT( -16.0 == *filter );

    ++filter;
    // index : 0  1  2  3  4  5  6  7  8  9 10 11 ...
    // value :10 10 10 10 10  2  2  2  2  2  2  2 ...
    //                |-----|  |-----|
    CPPUNIT_ASSERT( -16.0 == *filter );

    ++filter;
    // index : 0  1  2  3  4  5  6  7  8  9 10 11 ...
    // value :10 10 10 10 10  2  2  2  2  2  2  2 ...
    //                   |-----|  |-----|
    CPPUNIT_ASSERT( -8.0 == *filter );

    ++filter;
    CPPUNIT_ASSERT( 0 == *filter );

}
