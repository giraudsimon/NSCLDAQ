
#include <cppunit/extensions/HelperMacros.h>
#include "BasicRangeIteratorTest.h"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( BasicRangeIteratorTest );


void BasicRangeIteratorTest::setUp()
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

void BasicRangeIteratorTest::tearDown()
{
}

void BasicRangeIteratorTest::testConstructor()
{
    TrRangeS it (simple_tr.begin(), simple_tr.end());

    CPPUNIT_ASSERT( it.m_begin == simple_tr.begin().m_iter);
    CPPUNIT_ASSERT( it.m_end == simple_tr.end().m_iter);
}

void BasicRangeIteratorTest::testConstructor2()
{
    TrRangeS it (step_tr.begin(), 3);
    CPPUNIT_ASSERT( it.m_begin == step_tr.begin().m_iter);
    CPPUNIT_ASSERT( it.m_end == (step_tr.begin().m_iter+3));

    TrRangeS it2 (step_tr.end(), -4);
    CPPUNIT_ASSERT( it2.m_begin == (step_tr.fData.data()+6));
    CPPUNIT_ASSERT( it2.m_end == step_tr.end());
}

void BasicRangeIteratorTest::testCopyConstructor()
{
    TrRangeS it (step_tr.begin(), 3);
    TrRangeS it2 (it);
    CPPUNIT_ASSERT( it == it2 );
}

void BasicRangeIteratorTest::testAssignment()
{
    TrRangeS it (step_tr.begin(), 3);
    TrRangeS it2 = it;
    CPPUNIT_ASSERT( it == it2 );
}

void BasicRangeIteratorTest::testEquality()
{
    TrRangeS it (step_tr.begin(), 3);
    TrRangeS it2 = it;
    CPPUNIT_ASSERT( it.m_begin == it2.m_begin );
    CPPUNIT_ASSERT( it.m_end == it2.m_end );
}

void BasicRangeIteratorTest::testIsDoneOnNullTrace()
{
    TrRangeS it (null_tr.begin(), null_tr.end());
    TrRangeS it2 (null_tr.begin(), null_tr.end());

    CPPUNIT_ASSERT( !(it < it2) );
}

void BasicRangeIteratorTest::testMoveToInBounds()
{
    
    // Define a 4 element range
    TrRangeS it ( step_tr.begin(), 4);

//    it.MoveTo(3);
    ++it;
    ++it;
    ++it;
    TrRangeS endrange(step_tr.end(),-3);
    CPPUNIT_ASSERT( it < endrange );
}

void BasicRangeIteratorTest::testMoveToOutOfBounds()
{
    TrRangeS it ( step_tr.begin(), 4 );
    TrRangeS end ( step_tr.end(), -4 ); 

//    it.MoveTo(201);
    it = it + 201;
    
    CPPUNIT_ASSERT( !(it<end)  );
}

void BasicRangeIteratorTest::testIncrementInBounds()
{

    TrRangeS it (step_tr.begin(), 4);
    // verify that we start at the right spot 
    CPPUNIT_ASSERT(it.begin().m_iter == (step_tr.fData.data()) );
    CPPUNIT_ASSERT(it.end().m_iter == (step_tr.fData.data() +4) );
    
    TrRangeS it2 (step_tr.begin(), 4);
    CPPUNIT_ASSERT( it == it2 );

    ++it;
    it2++;

    CPPUNIT_ASSERT( it == it2 );
    CPPUNIT_ASSERT( it < TrRangeS(step_tr.end(),-4) ); 
}

void BasicRangeIteratorTest::testIncrementOutOfBounds()
{

    TrRangeS it( step_tr.begin()+5, 4);
//    it.MoveTo(3);
    CPPUNIT_ASSERT( it.begin().m_iter == (step_tr.fData.data()+5) );
    CPPUNIT_ASSERT( it < TrRangeS(step_tr.end(),-4) );

//    it.Increment();
    ++it;

    CPPUNIT_ASSERT( it.begin().m_iter == (step_tr.fData.data() +6) );
    CPPUNIT_ASSERT( it == TrRangeS(step_tr.end(),-4) );
   
    ++it;

    CPPUNIT_ASSERT( it.end().m_iter == (step_tr.fData.data() +11) );
    CPPUNIT_ASSERT( ! (it < TrRangeS(step_tr.end(),-4)) );
}

void BasicRangeIteratorTest::testRandomAccess()
{
    // Create distinct iterators
    uint16_t arr[] = {0, 1, 2, 3, 4};

    TrRangeS it(arr,1);
    TrRangeS it2(arr,1);

    it = it+3;
    it2 = 3+it2;
    CPPUNIT_ASSERT( it.begin().m_iter == &arr[3] );
    CPPUNIT_ASSERT( it2.begin().m_iter == &arr[3] );
    CPPUNIT_ASSERT( it.end().m_iter == &arr[4] );
    CPPUNIT_ASSERT( it2.end().m_iter == &arr[4] );
    CPPUNIT_ASSERT( it == it2 );
    it = it-2;
    it2 = -2+it2;

    CPPUNIT_ASSERT(it.begin().m_iter == &arr[1]);
    CPPUNIT_ASSERT(it2.begin().m_iter == &arr[1]);
    CPPUNIT_ASSERT(it.end().m_iter == &arr[2]);
    CPPUNIT_ASSERT(it2.end().m_iter == &arr[2]);
    CPPUNIT_ASSERT(it == it2);

    it = TrRangeS(&arr[0],&arr[1]);
    it2 = TrRangeS(&arr[0],&arr[1]);
    
    it+=2;
    ++it2; ++it2;
    CPPUNIT_ASSERT(it == it2);
    
    it-=1;
    it2--;
    CPPUNIT_ASSERT(it==it2);
}
