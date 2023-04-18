

#include <cppunit/extensions/HelperMacros.h>
#include "PeakFindProcessorTest.h"
#include <cmath>
#include <iterator>

CPPUNIT_TEST_SUITE_REGISTRATION( PeakFindProcessorTest );


void PeakFindProcessorTest::setUp()
{
    null_tr = TraceS(); 
    
    std::vector<uint16_t> vec(2);
    vec[0] = 1;
    vec[1] = 2;
    simple_tr = TraceS(vec); 

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

void PeakFindProcessorTest::tearDown()
{
}

void PeakFindProcessorTest::testProcessReturnType()
{
    PeakFindProcResult<uint16_t> res1 = TrAnal::FindPeak(simple_tr1.begin(), 
            simple_tr1.end());
    
    CPPUNIT_ASSERT( typeid(res1)==typeid(PeakFindProcResult<uint16_t>) );
}

void PeakFindProcessorTest::testFindMax()
{
    PeakFindProcResult<uint16_t> res = TrAnal::FindPeak(simple_tr1.begin(), 
            simple_tr1.end());

    CPPUNIT_ASSERT( 16 == res.max );
    CPPUNIT_ASSERT( 0  == res.umax );
    CPPUNIT_ASSERT( 4  == std::distance(simple_tr1.begin(), res.index) );
    
}

//void PeakFindProcessorTest::testFailOffsetOutOfBounds()
//{
//    // processor1 begins its algorithm at the 5th point
//    // simple_tr has 2 points
//    // This should fail and throw an exception 
//
//    PeakFindProcResult<uint16_t> res;
//    CPPUNIT_ASSERT_THROW( res=processor1->Process(simple_tr) ,  
//                            AlgorithmOutOfBoundsException );
//
//    
//}

void PeakFindProcessorTest::testSetOffset()
{
    TrIterator<uint16_t> begin = simple_tr2.begin();
    TrIterator<uint16_t> end = simple_tr2.end(); 
    PeakFindProcResult<uint16_t> res0 = TrAnal::FindPeak(begin,end);
    CPPUNIT_ASSERT( 51 == res0.max );
    CPPUNIT_ASSERT( 1  == std::distance(begin,res0.index) );

//    processor0->SetOffset(2);
    std::advance(begin,2);
    PeakFindProcResult<uint16_t> res = TrAnal::FindPeak(begin,simple_tr2.end());
    
    CPPUNIT_ASSERT( 42 == res.max );
    CPPUNIT_ASSERT( 8  == std::distance(simple_tr2.begin(), res.index) );

    
}

void PeakFindProcessorTest::testLocalMax()
{
    TrIterator<uint16_t> begin=simple_tr2.begin();
    std::advance(begin,5);
    PeakFindProcResult<uint16_t> res0 = TrAnal::FindPeak(begin, simple_tr2.end());
    CPPUNIT_ASSERT( 42 == res0.max);
    CPPUNIT_ASSERT( 8  == std::distance(simple_tr2.begin(),res0.index) );

//    processor1->SetSearchRange(3);
    TrIterator<uint16_t> end=begin;
    std::advance(end,3);
    res0 = TrAnal::FindPeak(begin,end);

    CPPUNIT_ASSERT( 15 == res0.max );
    CPPUNIT_ASSERT( 5  == std::distance(simple_tr2.begin(), res0.index) );

 //   processor1->SetSearchRange(5);
    end = begin;
    std::advance(end,5);
    res0 = TrAnal::FindPeak(begin,end);

    CPPUNIT_ASSERT( 42 == res0.max );
    CPPUNIT_ASSERT( 8 == std::distance(simple_tr2.begin(), res0.index) );
}


//void PeakFindProcessorTest::testAveragePeak()
//{
//    // this processor1 is offset by 5 so it starts at index 5
//    // simple_tr2 has 10 elements
//    PeakFindProcResult<uint16_t> res0 = processor1->Process(simple_tr2);
//    CPPUNIT_ASSERT( 42 == res0.max);
//    CPPUNIT_ASSERT( 8  == res0.index );
//
//    processor1->SetSearchRange(3);
//    res0 = processor1->Process(simple_tr2);
//
//    CPPUNIT_ASSERT( 15 == res0.max );
//    CPPUNIT_ASSERT( 5  == res0.index );
//
////    vec2[0] = 1;      
////    vec2[1] = 51;     18.667
////    vec2[2] = 4;      21
////    vec2[3] = 8;      9.333
////    vec2[4] = 16;     13
////    vec2[5] = 15;     12.667
////    vec2[6] = 7;      8.333
////    vec2[7] = 3;      17.33
////    vec2[8] = 42;     15
////    vec2[9] = 0;  
//
//    processor1->SetAveragingRange(1);
//    res0 = processor1->Process(simple_tr2);
//
//    CPPUNIT_ASSERT( (7.0+3.0+42.0)/3.0 == res0.max );
//    CPPUNIT_ASSERT( 7 == res0.index );
//
//}

//
//void PeakFindProcessorTest::testProcess()
//{
//    std::vector<int16_t> vec(2);
//    vec[0] = 1;
//    vec[1] = 2;
//
//    // create a trivial trace
//    TraceT<int16_t> trace(vec);
//    
//    // create a processor to process the entire trace
//    // (i.e. 2 points)
//    unsigned int npoints = 2;
//    PeakFindProcessor blp(npoints);
//    BaseLineProcResult<uint16_t>* res = blp.Process(trace);
//    
//    CPPUNIT_ASSERT(1.5 == res->mean);
//
//    // std dev = sqrt( (1.0 - 1.5)^2 + (2.0 - 1.5)^2 )
//    //         = sqrt( (0.5)^2 + (0.5)^2 )
//    //         = sqrt( 0.25 + 0.25 )
//    //         = sqrt( 0.5 )
//    CPPUNIT_ASSERT(::sqrt(0.5) == res->stdev);
//
//    delete res;
//}
//
//
