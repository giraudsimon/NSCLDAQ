
#include <cppunit/extensions/HelperMacros.h>
#include "CFDTest.h"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( CFDTest );


void CFDTest::setUp()
{
    null_tr = TraceS();

    std::vector<uint16_t> vec(2);
    vec[0] = 1;
    vec[1] = 2;

    // create a trivial trace
    simple_tr = TraceS(vec);

    std::vector<uint16_t> vec2(20);
    for (int i=0; i<vec2.size(); ++i) {
    if (i<5)    
        vec2[i] = i;
    else
        vec2[i] = 5;
    }
    step_tr = TraceS(vec2);

    std::vector<uint16_t> vec3(100);
    for (int i=0; i<vec3.size(); ++i) {
        vec3[i] = i;
    }
    line_tr = TraceS(vec3);

}

void CFDTest::tearDown()
{
}

void CFDTest::testValue()
{
    // Define the CFD to have two trap filters
    // with rise=2 and gap=1
    // Further consider a scale_factor = 0
    // and a delay of 2
    //  index 0 1 2 3 4 5 6 7 8 9
    //  value 0 1 2 3 4 5 5 5 5 5
    //dely_tf|---| |---|    
    //lead_tf    |---| |---|
    //        0 1 3 5 6 6 5 3 1 0 0 0 0 0 0
    //scale   0  1.5  3  2.5 0.5  0 0 0 0 0  
    //         0.5 2.5  3  1.5  0 
    // index=6  iter=0  5-3 = 2          
    // index=7  iter=1  3-3 = 0
    // index=8  iter=2  1-2.5 = -1.5
    // index=9  iter=3  0-1.5 = -1.5
    // index=10 iter=4  0-0.5 = -0.5
    // index=1  iter=5  0-0 = 0
    // so then this should find the zero at i=6
    // the fraction should be 0

    int rise_len = 2;
    int gap_len = 1;
    int delay = 2;
    int scale_factor = 0;
    CFDResult<uint16_t> res 
        = CFD(TrRange<uint16_t>(step_tr.begin(), step_tr.end()),
                rise_len, 
                gap_len, 
                delay,
                scale_factor);

    CPPUNIT_ASSERT(res.trig == step_tr.begin()+7);
    CPPUNIT_ASSERT( 0 == res.fraction);
}   
