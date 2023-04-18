

#include <cppunit/extensions/HelperMacros.h>
#include "AmplitudeProcessorTest.h"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( AmplitudeProcessorTest );


void AmplitudeProcessorTest::setUp()
{

    // empty trace
    null_tr = TraceS(); 

    // very small simple trace 
    std::vector<uint16_t> vec(2);
    vec[0] = 1;
    vec[1] = 2;
    simple_tr = TraceS(vec); 

    // simple trace with a single maximum at the center
    std::vector<uint16_t> vec1(10);
    vec1[0] = 1;
    vec1[1] = 2;
    vec1[2] = 4;
    vec1[3] = 8;
    vec1[4] = 16;
    vec1[5] = 15;
    vec1[6] = 7;
    vec1[7] = 3;
    vec1[8] = 1;
    vec1[9] = 0;
    simple_tr1 = TraceS(vec1); 

    // complicated trace that has 3 local maxima
    std::vector<uint16_t> vec2(10);
    vec2[0] = 1;
    vec2[1] = 51;
    vec2[2] = 4;
    vec2[3] = 8;
    vec2[4] = 16;
    vec2[5] = 15;
    vec2[6] = 7;
    vec2[7] = 3;
    vec2[8] = 42;
    vec2[9] = 0;
    simple_tr2 = TraceS(vec2); 
}

void AmplitudeProcessorTest::tearDown()
{

}

void AmplitudeProcessorTest::testFindAmp()
{
    TrIterS begin = simple_tr1.begin();    
    // better yet, use some different values
    unsigned int bl_offset = 1;
    unsigned int bl_range = 3;
    unsigned int pf_offset = 0;
    unsigned int pf_range = 4;

    TrIterS blbeg = begin; std::advance(blbeg,bl_offset);
    TrIterS blend = begin; std::advance(blend,bl_offset+bl_range);

    TrIterS pfbeg = begin; std::advance(pfbeg,bl_offset+bl_range+pf_offset);
    TrIterS pfend = begin; std::advance(pfend,bl_offset+bl_range+pf_offset+pf_range);
   
    AmplitudeProcResult res = ComputeAmplitude(TrRange<uint16_t>(blbeg,blend), TrRange<uint16_t>(pfbeg,pfend));
    
    // bl_mean  = 14.0/3
    // bl_stdev = sqrt( (2-4.667)^2 + (4.0-4.667)^2 + (8.0-4.667)^2 ) / sqrt(3-1)
    //          = sqrt( (2.667)^2 + (0.667)^2 + (3.333)^2 )/sqrt(2)
    //          = sqrt(  64/9 + 4/9 + 100/9 )/sqrt(2)
    //          = sqrt( 168/9 ) = sqrt(168.)/3 /sqrt(2)
    //          = sqrt( 168/(9*2) )
    CPPUNIT_ASSERT( (16-(2.+4.+8.)/3) == res.amp );
    
    CPPUNIT_ASSERT( ::fabs((::sqrt(168./18.0)-res.uamp ) ) < 1e-10); 
    
    CPPUNIT_ASSERT( (2.+4.+8.)/3 == res.baseline);
    CPPUNIT_ASSERT(::fabs((::sqrt(168./18.0)-res.ubaseline ) ) < 1e-10 );
    

}
