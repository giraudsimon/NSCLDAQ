

#include <cppunit/extensions/HelperMacros.h>
#include "RiseTimeProcessorTest.h"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( RiseTimeProcessorTest );


void RiseTimeProcessorTest::setUp()
{

//    processor0 = new RiseTimeProcessor(bl_offset, bl_range, pf_offset, pf_range); 

    // empty trace
    null_tr = TraceS(); 

    // very small simple trace 
    std::vector<uint16_t> vec(200);
    for (unsigned int i=0; i<50; ++i)   vec[i] = 0;
    for (unsigned int i=50; i<150; ++i) vec[i] = i-50;
    for (unsigned int i=150; i<200; ++i) vec[i] = 100;
    simple_tr = TraceS(vec); 

}

void RiseTimeProcessorTest::tearDown()
{
}


void RiseTimeProcessorTest::testProcess()
{
    unsigned int bl_offset = 1;
    unsigned int bl_range = 45;
//    unsigned int pf_offset = 2;
    unsigned int pf_range = 120;
    double threshold = 2000.2;
    
    RiseTimeProcResult<uint16_t> res 
              = ComputeRiseTime(simple_tr.begin()+bl_offset,bl_range,pf_range);
    CPPUNIT_ASSERT( 0 == res.baseline );
    CPPUNIT_ASSERT( 100 == res.amp);    
    
    CPPUNIT_ASSERT( 10 == res.t10_amp ); 
    CPPUNIT_ASSERT( simple_tr.begin()+60 == res.t10_index);

    CPPUNIT_ASSERT( 90 == res.t90_amp ); 
    CPPUNIT_ASSERT( simple_tr.begin()+140 == res.t90_index);
    
}

void RiseTimeProcessorTest::testT90AndT10Same()
{
    // very small simple trace 
    std::vector<uint16_t> vec(200);
    for (unsigned int i=0; i<50; ++i)   vec[i] = 0;
    for (unsigned int i=50; i<150; ++i) vec[i] = 100-(i-50);
    for (unsigned int i=150; i<200; ++i) vec[i] = 0;
    TraceS negslope_tr(vec); 

    unsigned int bl_offset = 1;
    unsigned int bl_range = 45;
    unsigned int pf_range = 120;
    double threshold = 2000.2;
    
    RiseTimeProcResult<uint16_t> res;
    CPPUNIT_ASSERT_THROW(res=ComputeRiseTime(negslope_tr.begin()+bl_offset,
                bl_range,pf_range), InvalidResultException);
}
