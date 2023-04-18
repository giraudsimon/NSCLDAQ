#include <cppunit/extensions/HelperMacros.h>
#include "ThresholdTest.h"
#include "SumIterator.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( ThresholdTest );

void ThresholdTest::setUp()
{
    null_tr = TraceS();

    std::vector<uint16_t> vec(100);
    for (unsigned int i=0; i<vec.size(); ++i) 
        vec[i] = i;

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

    std::vector<uint16_t> vec3(4);
    vec3[0] = 10;
    vec3[1] = 7;
    vec3[2] = 4;
    vec3[3] = 0;
    negslope_tr = TraceS(vec3);

}

void ThresholdTest::tearDown()
{
}

void ThresholdTest::testProcess() 
{
    TrIterator<uint16_t> begin=simple_tr.begin();
    std::advance(begin,1);

    TrIterator<uint16_t> end=simple_tr.end();
    TrIterator<uint16_t> res = Threshold(begin,end,23.5);
    
    
    CPPUNIT_ASSERT( 24 == std::distance(simple_tr.begin(),res) );

}

void ThresholdTest::testFirstValSatisfiesThresh()
{
    
    TrIterator<uint16_t> res = Threshold(negslope_tr.begin(),
            negslope_tr.end(),
            5.0);

    CPPUNIT_ASSERT(res == negslope_tr.begin() );
    
}

void ThresholdTest::testNoSatisfaction()
{
    TrIterator<uint16_t> begin=negslope_tr.begin();
    TrIterator<uint16_t> end=negslope_tr.end();
    TrIterator<uint16_t> res =Threshold(begin,end,20.0);
    CPPUNIT_ASSERT(end == res);
} 

void ThresholdTest::testAlgoIteratorProcess()
{
    SumIterator<uint16_t> sum(TrRange<uint16_t>(simple_tr.begin(),4));
    
    TrIterator<uint16_t> res = Threshold(sum,simple_tr.end(),50);
     
    CPPUNIT_ASSERT(res == (simple_tr.begin() + 11));


}

