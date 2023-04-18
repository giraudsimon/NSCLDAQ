
#include <cppunit/extensions/HelperMacros.h>
#include "AveragingRangeIteratorTest.h"
#include "Trace.h"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( AveragingRangeIteratorTest );


void AveragingRangeIteratorTest::setUp()
{
    null_tr = TraceS();


    std::vector<int16_t> vec(2);
    vec[0] = 1;
    vec[1] = 2;

    // create a trivial trace
    simple_tr = TraceS(vec);


    std::vector<int16_t> vec2(10);
    for (int i=0; i<vec2.size(); ++i) {
    if (i<5)    
        vec2[i] = 1;
    else
        vec2[i] = 2;
    }
    step_tr = TraceS(vec2);

    std::vector<int16_t> vec3(100);
    for (int i=0; i<vec3.size(); ++i) {
        vec3[i] = i;
    }
    line_tr = TraceS(vec3);

}

void AveragingRangeIteratorTest::tearDown()
{
}

void AveragingRangeIteratorTest::testConstructor()
{
    AveragingRangeIterator it ( simple_tr, 0,10);

    CPPUNIT_ASSERT( 0 == it.CurrentIndex() );
    CPPUNIT_ASSERT( false == it.IsDone() );
}


void AveragingRangeIteratorTest::testIsDoneOnNullTrace()
{
    AveragingRangeIterator it ( null_tr, 0, 4);

    CPPUNIT_ASSERT( 0 == it.CurrentIndex() );
    CPPUNIT_ASSERT( true == it.IsDone() );
}

void AveragingRangeIteratorTest::testMoveToInBounds()
{
    AveragingRangeIterator it ( step_tr, 0, 4 );

    it.MoveTo(3);
    CPPUNIT_ASSERT( 3 == it.CurrentIndex() );
    CPPUNIT_ASSERT( false == it.IsDone() );
}

void AveragingRangeIteratorTest::testMoveToOutOfBounds()
{
    AveragingRangeIterator it ( step_tr, 0, 4);

    it.MoveTo(201);
    CPPUNIT_ASSERT( 10 == it.CurrentIndex() );
    CPPUNIT_ASSERT( true == it.IsDone() );

    it.MoveTo(-1);
    CPPUNIT_ASSERT( 10 == it.CurrentIndex() );
    CPPUNIT_ASSERT( true == it.IsDone() );
}

void AveragingRangeIteratorTest::testIncrementInBounds()
{

    AveragingRangeIterator it( step_tr, 0, 4);
    
    CPPUNIT_ASSERT( 0 == it.CurrentIndex() );

    it.Increment();

    CPPUNIT_ASSERT( 1 == it.CurrentIndex() );
   
    
}

void AveragingRangeIteratorTest::testIncrementOutOfBounds()
{

    AveragingRangeIterator it( step_tr, 0, 4);
    
    it.MoveTo(3);
    CPPUNIT_ASSERT( 3 == it.CurrentIndex() );
    CPPUNIT_ASSERT( false == it.IsDone() );

    it.Increment();

    CPPUNIT_ASSERT( 10 == it.CurrentIndex() );
    CPPUNIT_ASSERT( true == it.IsDone() );
   
    it.Increment();

    CPPUNIT_ASSERT( 10 == it.CurrentIndex() );
    CPPUNIT_ASSERT( true == it.IsDone() );
    
}

void AveragingRangeIteratorTest::testSimpleAveragingInRange()
{

    AveragingRangeIterator it( line_tr, 10, 90, 2);

    it.MoveTo(10);
    CPPUNIT_ASSERT( 10.0 == it.CurrentValue() );
    CPPUNIT_ASSERT( 10 == it.CurrentIndex() );

    it.MoveTo(30);
    CPPUNIT_ASSERT( 30.0 == it.CurrentValue() );
    CPPUNIT_ASSERT( 30 == it.CurrentIndex() );

    it.MoveTo(50);
    CPPUNIT_ASSERT( 50.0 == it.CurrentValue() );
    CPPUNIT_ASSERT( 50 == it.CurrentIndex() );
    
    it.MoveTo(70);
    CPPUNIT_ASSERT( 70.0 == it.CurrentValue() );
    CPPUNIT_ASSERT( 70 == it.CurrentIndex() );

    it.MoveTo(89);
    CPPUNIT_ASSERT( 89.0 == it.CurrentValue() );
    CPPUNIT_ASSERT( 89 == it.CurrentIndex() );

}

void AveragingRangeIteratorTest::testAveragingOutOfRange()
{

    AveragingRangeIterator it( line_tr, 10, 90, 2);

    it.MoveTo(3);
    CPPUNIT_ASSERT( 0 == it.CurrentValue() );
    CPPUNIT_ASSERT( 100 == it.CurrentIndex() );

    it.MoveTo(90);
    CPPUNIT_ASSERT( 0 == it.CurrentValue() );
    CPPUNIT_ASSERT( 100 == it.CurrentIndex() );


}

void AveragingRangeIteratorTest::testHardBoundary()
{

    AveragingRangeIterator it( line_tr, 0, 90, 2);

    it.First();
    CPPUNIT_ASSERT( 2.0 == it.CurrentValue() );
    CPPUNIT_ASSERT( 2 == it.CurrentIndex() );

    it.MoveTo(99);
    CPPUNIT_ASSERT( 0.0 == it.CurrentValue() );
    CPPUNIT_ASSERT( 100 == it.CurrentIndex() );

}

void AveragingRangeIteratorTest::testFirst()
{

    AveragingRangeIterator it(line_tr,0,90,3);

    it.First();
    CPPUNIT_ASSERT( 3 == it.CurrentIndex() );
}

void AveragingRangeIteratorTest::testAveraging()
{

    AveragingRangeIterator it(step_tr,0,10,2);

    it.First();
    CPPUNIT_ASSERT( 2 == it.CurrentIndex() );
    CPPUNIT_ASSERT( 1.0 == it.CurrentValue() );

    it.Increment();
    CPPUNIT_ASSERT( 3 == it.CurrentIndex() );
    CPPUNIT_ASSERT( (4.0+2.0)/5 == it.CurrentValue() );

    it.Increment();
    CPPUNIT_ASSERT( 4 == it.CurrentIndex() );
    CPPUNIT_ASSERT( (3.0+4.0)/5 == it.CurrentValue() );

    it.Increment();
    CPPUNIT_ASSERT( 5 == it.CurrentIndex() );
    CPPUNIT_ASSERT( (2.0+6.0)/5 == it.CurrentValue() );

    it.Increment();
    CPPUNIT_ASSERT( 6 == it.CurrentIndex() );
    CPPUNIT_ASSERT( (1.0+8.0)/5 == it.CurrentValue() );
}


void AveragingRangeIteratorTest::testRangeLongerThanTrace()
{

    AveragingRangeIterator it(step_tr,0,10,100);

    CPPUNIT_ASSERT( 10== it.CurrentIndex() );
    CPPUNIT_ASSERT( 0== it.CurrentValue() );
     

}
