

#include <cppunit/extensions/HelperMacros.h>
#include "BaseLineProcessorTest.h"
#include <cmath>
#include <iterator>

CPPUNIT_TEST_SUITE_REGISTRATION( BaseLineProcessorTest );


void BaseLineProcessorTest::setUp()
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
}

void BaseLineProcessorTest::tearDown()
{
}

void BaseLineProcessorTest::testProcessReturnTypeIsBaseLineProcResult()
{
    BaseLineProcResult res = TrAnal::ComputeBaseLine(simple_tr.begin(), simple_tr.end());
    
    CPPUNIT_ASSERT( typeid(res)==typeid(BaseLineProcResult) );
}

void BaseLineProcessorTest::testProcess()
{
    
    // create a processor to process the entire trace
    // (i.e. 2 points)
    TrIterator<uint16_t> begin=simple_tr.begin();
    TrIterator<uint16_t> end = begin; std::advance(end,2);

    BaseLineProcResult res = TrAnal::ComputeBaseLine(begin,end);
    
    CPPUNIT_ASSERT(1.5 == res.mean);

    // std dev = sqrt( { (1.0 - 1.5)^2 + (2.0 - 1.5)^2 ) } /(2-1) )
    //         = sqrt( { (0.5)^2 + (0.5)^2 } / (2-1) )
    //         = sqrt( { 0.25 + 0.25 } / (2-1) )
    //         = sqrt( 0.5 / 1 )
    CPPUNIT_ASSERT(::sqrt(0.5) == res.stdev);

}

void BaseLineProcessorTest::testRange()
{
    TrIterator<uint16_t> begin=step_tr.begin();

    // set the end 4 elements beyond the begin
    // the range is thus 4 elements [0,3)
    TrIterator<uint16_t> end=begin; std::advance(end,4);

    BaseLineProcResult res = TrAnal::ComputeBaseLine(begin,end);
    CPPUNIT_ASSERT(1.0 == res.mean );

    // move the end by 2 to be 6 elements beyond the begin
    // the range is thus 6 elements [0,6)
    std::advance(end,2);
    res = TrAnal::ComputeBaseLine(begin,end);
    CPPUNIT_ASSERT( (5*1.0 + 2.0)/6 == res.mean);

}

void BaseLineProcessorTest::testOffset()
{

    // Process three times and check that the result is sensible
    // The offset will change each time

    TrIterator<uint16_t> begin=step_tr.begin();
    TrIterator<uint16_t> end=begin; std::advance(end,4);
//    blp->SetRange(4);
    BaseLineProcResult res = TrAnal::ComputeBaseLine(begin,end);

    CPPUNIT_ASSERT(1.0 == res.mean );
    CPPUNIT_ASSERT(0.0 == res.stdev );

    std::advance(begin,5);
    end=begin; std::advance(end,4);
//    blp->SetOffset(5);
    res = TrAnal::ComputeBaseLine(begin,end);
    CPPUNIT_ASSERT( 2.0 == res.mean);
    CPPUNIT_ASSERT( 0.0 == res.stdev);

    begin = step_tr.begin();
    std::advance(begin,3);
    end=begin; std::advance(end,4);
//    blp->SetOffset(3);
    res = TrAnal::ComputeBaseLine(begin,end);

    CPPUNIT_ASSERT( 1.5 == res.mean);

    // sqrt((1-1.5)^2 + (1-1.5)^2 + (2-1.5)^2 + (2-1.5)^2) / sqrt(4-1)
    // = sqrt( 0.25 + 0.25 + 0.25 + 0.25) / sqrt(4-1)
    // = 1/sqrt(3) 
    CPPUNIT_ASSERT( ::sqrt(1.0/3.0) == res.stdev);

}
