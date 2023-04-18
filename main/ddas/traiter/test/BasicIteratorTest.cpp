
#include <cppunit/extensions/HelperMacros.h>
#include "BasicIteratorTest.h"
#include <cmath>

CPPUNIT_TEST_SUITE_REGISTRATION( BasicIteratorTest );


void BasicIteratorTest::setUp()
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

    std::vector<uint16_t> vec3(10);
    for (int i=0; i<vec3.size(); ++i) {
        vec3[i] = i;
    }
    linear_tr = TraceS(vec3);
}

void BasicIteratorTest::tearDown()
{
}

void BasicIteratorTest::testConstructor()
{
    TrIterS it = simple_tr.begin() ;

    CPPUNIT_ASSERT( it == simple_tr.begin() );
    CPPUNIT_ASSERT( 1 == *it );
}

void BasicIteratorTest::testCopyConstructor()
{
    TrIterS it = simple_tr.begin() ;

    TrIterS it2(it);

    CPPUNIT_ASSERT( it.m_iter == it2.m_iter );
}

void BasicIteratorTest::testAssignment()
{
    // Create distinct iterators
    TrIterS it = simple_tr.begin() ;
    TrIterS it2 = simple_tr.end();
    //ensure that they are different
    CPPUNIT_ASSERT( it.m_iter != it2.m_iter );

    // set them equal
    it2 = it;

    // verify that they are equal
    CPPUNIT_ASSERT( it == it2 );
}

void BasicIteratorTest::testEquality()
{
    // Create distinct iterators
    TrIterS it((uint16_t*)0x0);
    TrIterS it2((uint16_t*) 0x10);
    TrIterS it3((uint16_t*)0x10);

    // test not equal 
    CPPUNIT_ASSERT( it != it2 );
    // test equality
    CPPUNIT_ASSERT( it2 == it3 );
}

void BasicIteratorTest::testComparisons()
{
    // Create distinct iterators
    TrIterS it( (uint16_t*)0x0);
    TrIterS it2((uint16_t*)0x10);
    TrIterS it3((uint16_t*)0x10);

    // test less than 
    CPPUNIT_ASSERT( it < it2 );

    // test less than equal 
    CPPUNIT_ASSERT( it <= it2 );
    CPPUNIT_ASSERT( it2 <= it3 );

    // test greater than
    CPPUNIT_ASSERT( it2 > it );
    CPPUNIT_ASSERT( it2 >= it3 );
}

void BasicIteratorTest::testIncrements()
{
    // Create distinct iterators
    uint16_t arr[] = {0, 1, 2, 3, 4};

    TrIterS it(arr);
    TrIterS it2(arr);

    CPPUNIT_ASSERT(it.m_iter = &arr[0]);
    CPPUNIT_ASSERT(it2.m_iter = &arr[0]);

    it++;
    ++it2;

    CPPUNIT_ASSERT(it.m_iter = &arr[1]);
    CPPUNIT_ASSERT(it2.m_iter = &arr[1]);
    CPPUNIT_ASSERT(it == it2);

}

void BasicIteratorTest::testRandomAccess()
{
    // Create distinct iterators
    uint16_t arr[] = {0, 1, 2, 3, 4};

    TrIterS it(arr);
    TrIterS it2(arr);

    it = it+3;
    it2 = 3+it2;
    CPPUNIT_ASSERT( it.m_iter = &arr[3] );
    CPPUNIT_ASSERT( it2.m_iter = &arr[3] );
    CPPUNIT_ASSERT( it == it2 );
    it = it-2;
    it2 = -2+it2;

    CPPUNIT_ASSERT(it.m_iter = &arr[1]);
    CPPUNIT_ASSERT(it2.m_iter = &arr[1]);
    CPPUNIT_ASSERT(it == it2);

    it = arr;
    it2 = arr;
    
    it+=2;
    ++it2; ++it2;
    CPPUNIT_ASSERT(it == it2);
    
    it-=1;
    it2--;
    CPPUNIT_ASSERT(it==it2);
}

void BasicIteratorTest::testIsDoneOnNullTrace()
{
    TrIterS it = null_tr.begin();

    CPPUNIT_ASSERT( it == null_tr.begin() );
    CPPUNIT_ASSERT( it == null_tr.end() );
}

void BasicIteratorTest::testMoveToInBounds()
{
    TrIterS it = linear_tr.begin() ;

//    it.MoveTo(3);
    ++it; // it->1
    ++it; // it->2
    ++it; // it->3
    CPPUNIT_ASSERT( 3 == *it );
    CPPUNIT_ASSERT( it < linear_tr.end() );
}

void BasicIteratorTest::testMoveToOutOfBounds()
{
    TrIterS it = step_tr.begin();

    for (unsigned int i=0; i<201; ++i) ++it;

//    it.MoveTo(201);
//    CPPUNIT_ASSERT( 10 == it.CurrentIndex() );
    CPPUNIT_ASSERT( it >= step_tr.end() );
//    CPPUNIT_ASSERT( true == it.IsDone() );

//    it.MoveTo(-1);
//    CPPUNIT_ASSERT( 10 == it.CurrentIndex() );
//    CPPUNIT_ASSERT( true == it.IsDone() );
}

void BasicIteratorTest::testIncrementInBounds()
{

    TrIterS it = step_tr.begin();
    
    CPPUNIT_ASSERT( it == step_tr.begin() );

//    it.Increment();
    ++it;

    CPPUNIT_ASSERT( it.m_iter == (step_tr.begin().m_iter +1) );
   
    
}

void BasicIteratorTest::testIncrementOutOfBounds()
{

    TrIterS it = step_tr.begin();
    TrIterS it2= step_tr.begin();

    const uint16_t* ptr = step_tr.begin().m_iter;
    
//    it.MoveTo(9);
    std::advance(it,9);
    for (uint16_t i=0; i<9; ++i) it2++;
    
    CPPUNIT_ASSERT( it.m_iter == (ptr+9) );
    CPPUNIT_ASSERT( it == it2 );
    CPPUNIT_ASSERT( it < step_tr.end() );

//    it.Increment();
    ++it;
    it2++;

    CPPUNIT_ASSERT( it.m_iter == (ptr+10) );
    CPPUNIT_ASSERT( it == it2 );
    CPPUNIT_ASSERT( it >= step_tr.end() );
   
//    it.Increment();
    ++it;
    it2++;

    CPPUNIT_ASSERT( it.m_iter == (ptr+11) );
    CPPUNIT_ASSERT( it == it2 );
    CPPUNIT_ASSERT( it >= step_tr.end() );
    
}

void BasicIteratorTest::testIteratorSubtraction()
{
    uint16_t arr[] = {0,1,2,3,4};
    TrIterS i0(arr);
    TrIterS i1(&arr[3]);


    CPPUNIT_ASSERT(3 == (i1-i0));
    CPPUNIT_ASSERT( -3 == (i0-i1));

}
