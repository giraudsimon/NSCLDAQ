/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2015.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Jeromy Tompkins
       NSCL
       Michigan State University
       East Lansing, MI 48824-1321
*/

#include <cppunit/Asserter.h>
#include <cppunit/extensions/HelperMacros.h>
#include "Asserts.h"

#include <V10/CRingScalerItem.h>
#include <V11/CRingScalerItem.h>

#include <V10/CRingStateChangeItem.h>
#include <V11/CRingStateChangeItem.h>

#include <V10/CPhysicsEventItem.h>
#include <V11/CPhysicsEventItem.h>

#include <V10/CRingPhysicsEventCountItem.h>
#include <V11/CRingPhysicsEventCountItem.h>

#include <V10/CRingTextItem.h>
#include <V11/CRingTextItem.h>

#include <V10/CRingTimestampedRunningScalerItem.h>

#include <V10/CRingFragmentItem.h>
#include <V11/CRingFragmentItem.h>

#include <V10/DataFormatV10.h>
#include <V11/DataFormatV11.h>

#define private public
#define protected public
#include <CTransform10p0to11p0.h>
#undef protected
#undef private

#include <chrono>
#include <iterator>
#include <algorithm>

using namespace std;
using namespace DAQ;
using namespace DAQ::Transform;

/*!
 * \brief The CTransform10p0to11p0Tests class
 *
 * Tests whether the transformation rules are held for, scalers, state changes, and
 * physics events when going between version 10 and 11
 */
class CTransform10p0to11p0Tests : public CppUnit::TestFixture
{
private:
    CTransform10p0to11p0 m_transform;

public:
    CPPUNIT_TEST_SUITE(CTransform10p0to11p0Tests);
    CPPUNIT_TEST(scaler_0);
    CPPUNIT_TEST(stateChange_0);
    CPPUNIT_TEST(physicsEvent_0);
    CPPUNIT_TEST(physicsEvent_1);
    CPPUNIT_TEST(physicsEvent_2);
    CPPUNIT_TEST(physicsEvent_3);

    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() {
        m_transform = CTransform10p0to11p0();
    }
  void tearDown() {
  }
protected:
  void scaler_0();
  void stateChange_0();

  void physicsEvent_0();
  void physicsEvent_1();
  void physicsEvent_2();
  void physicsEvent_3();

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to11p0Tests);

void CTransform10p0to11p0Tests::scaler_0()
{
    using namespace chrono;


    std::time_t time_now = system_clock::to_time_t( system_clock::now() );

    V10::CRingScalerItem v10item(0, 1, time_now, {0, 1, 2, 3});

    V11::CRingScalerItem v11item = m_transform(v10item);

    V11::CRingScalerItem expected(0, 1, time_now, {0, 1, 2, 3}, true, 1);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler item should have no body header",
                                 false, v11item.hasBodyHeader() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE("INCREMENTAL_SCALERS should become PERIODIC_SCALERS",
                                 V11::PERIODIC_SCALERS, v11item.type() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler item start times should be the same",
                                 v10item.getStartTime(), v11item.getStartTime() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler item stop times should be the same",
                                 v10item.getEndTime(), v11item.getEndTime() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler item timestamp should be the same",
                                 v10item.getTimestamp(), v11item.getTimestamp() );

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scaler count should remain the same",
                                 v10item.getScalerCount(), v11item.getScalerCount() );

    CPPUNIT_ASSERT_MESSAGE("Scaler values should remain the same",
                                 v10item.getScalers() == v11item.getScalers() );

}

    void CTransform10p0to11p0Tests::stateChange_0()
    {
        using namespace chrono;


        std::time_t time_now = system_clock::to_time_t( system_clock::now() );

        V10::CRingStateChangeItem v10item(V10::BEGIN_RUN, 3,
                                                10203, time_now, "test");

        V11::CRingStateChangeItem v11item = m_transform(v10item);

        V11::CRingStateChangeItem expected(V10::BEGIN_RUN, 3,
                                                 10203, time_now, "test");

        CPPUNIT_ASSERT_EQUAL_MESSAGE("State change item should have no body header",
                                     false, v11item.hasBodyHeader() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("State change item type should remain the same",
                                     V11::BEGIN_RUN, v11item.type() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("State change item run number should be the same",
                                     v10item.getRunNumber(), v11item.getRunNumber() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("State change item time offset should be the same",
                                     v10item.getElapsedTime(), v11item.getElapsedTime() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("State change item timestamp should remain the same",
                                     v10item.getTimestamp(), v11item.getTimestamp() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("State change item offset divisor defaults to 1",
                                     1u, v11item.getOffsetDivisor() );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("State change title remains the same",
                                     v10item.getTitle(), v11item.getTitle() );
    }

    void CTransform10p0to11p0Tests::physicsEvent_0()
    {
        vector<uint16_t> data = {0, 1, 2, 3, 4};

        V10::CPhysicsEventItem v10item(V10::PHYSICS_EVENT);
        v10item.fillBody(data);

        V11::CPhysicsEventItem v11item = m_transform(v10item);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics event should have no body header",
                                     false, v11item.hasBodyHeader() );

    }

    void CTransform10p0to11p0Tests::physicsEvent_1()
    {
        vector<uint16_t> data = {0, 1, 2, 3, 4};

        V10::CPhysicsEventItem v10item(V10::PHYSICS_EVENT);
        v10item.fillBody(data);

        V11::CPhysicsEventItem v11item = m_transform(v10item);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics event should have no body header",
                                     V11::PHYSICS_EVENT, v11item.type() );

    }

    void CTransform10p0to11p0Tests::physicsEvent_2()
    {
        vector<uint16_t> data = {0, 1, 2, 3, 4};

        V10::CPhysicsEventItem v10item(V10::PHYSICS_EVENT);
        v10item.fillBody(data);

        V11::CPhysicsEventItem v11item = m_transform(v10item);

        uint16_t* pBody = reinterpret_cast<uint16_t*>(v11item.getBodyPointer());

        for (size_t i=0; i<data.size(); i++) {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics event body remains the same",
                                         data.at(i), *pBody++);
        }

    }

    void CTransform10p0to11p0Tests::physicsEvent_3()
    {
        vector<uint16_t> data = {0, 1, 2, 3, 4};

        V10::CPhysicsEventItem v10item(V10::PHYSICS_EVENT);
        v10item.fillBody(data);

        V11::CPhysicsEventItem v11item = m_transform(v10item);

        CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics event body should have same size",
                                     v10item.getBodySize(), v11item.getBodySize());

    }

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    /*!
     * \brief The CTransform10p0to11p0Tests_PhysicsCount class
     *
     * Tests that transformation rules are obeyed for physics event count
     *  items when going from 10 -> 11.
     */
    class CTransform10p0to11p0Tests_PhysicsCount : public CppUnit::TestFixture
    {

        CPPUNIT_TEST_SUITE(CTransform10p0to11p0Tests_PhysicsCount);
        CPPUNIT_TEST(physicsCount_0);
        CPPUNIT_TEST(physicsCount_1);
        CPPUNIT_TEST(physicsCount_2);
        CPPUNIT_TEST(physicsCount_3);
        CPPUNIT_TEST(physicsCount_4);
        CPPUNIT_TEST_SUITE_END();

    public:
        V10::CRingPhysicsEventCountItem v10item;
        V11::CRingPhysicsEventCountItem v11item;
        CTransform10p0to11p0 m_transform;
        std::time_t time_now;

    public:
        void setUp()
        {
            using namespace std::chrono;

            // force this be different than now
            time_now = system_clock::to_time_t( system_clock::now() ) + 1;

            m_transform = CTransform10p0to11p0();
            v10item     = V10::CRingPhysicsEventCountItem(123, 345, time_now);
            v11item     = m_transform(v10item);
        }

        void tearDown() {

        }

        void physicsCount_0()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics count should have no body header",
                                         false, v11item.hasBodyHeader());

        }

        void physicsCount_1()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics count offset should remain the same",
                                         uint32_t(345), v11item.getTimeOffset());
        }

        void physicsCount_2()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics count offset divisor defaults to 1",
                                         uint32_t(1), v11item.getTimeDivisor());
        }

        void physicsCount_3()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics count timestamp should remain the same",
                                         time_now, v11item.getTimestamp());
        }
        void physicsCount_4()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics count event count should remain the same",
                                         uint64_t(123), v11item.getEventCount());
        }
    };

    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to11p0Tests_PhysicsCount);


    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    /*!
     * \brief The CTransform10p0to11p0Tests_Text class
     *
     * Tests that the transformation rules for CRingTextItems are obeyed.
     */
    class CTransform10p0to11p0Tests_Text : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform10p0to11p0Tests_Text);
        CPPUNIT_TEST(Text_0);
        CPPUNIT_TEST(Text_1);
        CPPUNIT_TEST(Text_2);
        CPPUNIT_TEST(Text_3);
        CPPUNIT_TEST(Text_4);
        CPPUNIT_TEST(Text_5);
        CPPUNIT_TEST(Text_6);
        CPPUNIT_TEST(Text_7);
        CPPUNIT_TEST(Text_8);
        CPPUNIT_TEST_SUITE_END();

    public:
        V10::CRingTextItem v10item;
        V11::CRingTextItem v11item;
        CTransform10p0to11p0 m_transform;
        std::time_t time_now;
        std::vector<std::string> strings;

    public:
        // We need to define a default constructor b/c the CRingTextItem classes
        // do not define a default constructor.
        CTransform10p0to11p0Tests_Text() : v10item(V10::MONITORED_VARIABLES, {}),
            v11item(V11::MONITORED_VARIABLES, {}),
            m_transform(),
            time_now(),
            strings() {}

        void setUp()
        {
            using namespace std::chrono;

            // force this be different than now
            time_now = system_clock::to_time_t( system_clock::now() ) + 1;

            strings = std::vector<std::string>({"the", "test", "array"});
            v10item     = V10::CRingTextItem(V10::MONITORED_VARIABLES,
                                                   strings,
                                                   345,
                                                   time_now);
            v11item     = m_transform(v10item);

            m_transform = CTransform10p0to11p0();
        }

        void tearDown() {

        }

        void Text_0()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Text item should have no body header",
                                         false, v11item.hasBodyHeader());

        }

        void Text_1()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Text time offset should remain the same",
                                         uint32_t(345), v11item.getTimeOffset());
        }

        void Text_2()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Text item timestamp should remain the same",
                                         time_now, v11item.getTimestamp());
        }
        void Text_3()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Text item string count should remain the same",
                                         uint32_t(3), v11item.getStringCount());
        }
        void Text_4()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Text item time divisor should remain the same",
                                         uint32_t(1), v11item.getTimeDivisor());
        }

        void Text_5()
        {
            CPPUNIT_ASSERT_MESSAGE("Text item strings should remain the same",
                                    strings == v11item.getStrings());
        }
        void Text_6()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                        "String count must remain the same",
                        uint32_t(strings.size()), v11item.getStringCount());
        }

        void Text_7()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                        "V10::MONITORED_VARIABLES --> V11::MONITORED_VARIABLES",
                        V11::MONITORED_VARIABLES, v11item.type());
        }

        void Text_8()
        {
            v10item = V10::CRingTextItem(V10::PACKET_TYPES, {});
            v11item = m_transform(v10item);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                        "V10::PACKET_TYPES --> V11::PACKET_TYPES",
                        V11::PACKET_TYPES, v11item.type());
        }

    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to11p0Tests_Text);

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////


    /*!
     * \brief The CTransform10p0to11p0Tests_NonIncrSclr class
     *
     * Tests that rules of transformation are obeyed for
     * CRingTimestampedRunningScalers (i.e. TIMESTAMPED_NONINCR_SCALERS)
     */
    class CTransform10p0to11p0Tests_NonIncrSclr : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform10p0to11p0Tests_NonIncrSclr);
        CPPUNIT_TEST(NonIncrSclr_0);
        CPPUNIT_TEST(NonIncrSclr_1);
        CPPUNIT_TEST(NonIncrSclr_2);
        CPPUNIT_TEST(NonIncrSclr_3);
        CPPUNIT_TEST(NonIncrSclr_4);
        CPPUNIT_TEST(NonIncrSclr_5);
        CPPUNIT_TEST(NonIncrSclr_6);
        CPPUNIT_TEST(NonIncrSclr_7);
        CPPUNIT_TEST(NonIncrSclr_8);
        CPPUNIT_TEST_SUITE_END();

    public:
        V10::CRingTimestampedRunningScalerItem v10item;
        V11::CRingScalerItem v11item;
        CTransform10p0to11p0 m_transform;
        std::time_t time_now;
        std::vector<uint32_t> values;

    public:
        CTransform10p0to11p0Tests_NonIncrSclr()
            : v10item(0,0,0,0,0, {1}),
              v11item(12),
              time_now(),
              values({}) {}

        void setUp()
        {
            using namespace std::chrono;
            using NonIncrSclr = V10::CRingTimestampedRunningScalerItem;

            // force this be different than now by 1
            time_now = system_clock::to_time_t( system_clock::now() ) + 1;
            values   = vector<uint32_t>({0, 1, 2, 3});

            m_transform = CTransform10p0to11p0();

            v10item     = NonIncrSclr(123456, 12, 34, 56, time_now, values);

            // do the transform
            v11item     = m_transform(v10item);
        }

        void tearDown() {

        }

        void NonIncrSclr_0()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("NonIncrSclr item should have no body header",
                                         false, v11item.hasBodyHeader());

        }

        void NonIncrSclr_1()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("NonIncrSclr start time should remain the same",
                                         uint32_t(12), v11item.getStartTime());
        }

        void NonIncrSclr_2()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("NonIncrSclr end time should remain the same",
                                         uint32_t(34), v11item.getEndTime());
        }
        void NonIncrSclr_3()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("NonIncrSclr interval divisor should become 1",
                                         uint32_t(1), v11item.getTimeDivisor());
        }

        void NonIncrSclr_4()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("NonIncrSclr item timestamp should remain the same",
                                         time_now, v11item.getTimestamp());
        }
        void NonIncrSclr_5()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                        "NonIncrSclr Scaler count should remain unchanged",
                        uint32_t(values.size()), v11item.getScalerCount());
        }

        void NonIncrSclr_6()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
               "V10::TIMESTAMPED_NONINCR_SCALERS --> V11::PERIODIC_SCALERS",
                        V11::PERIODIC_SCALERS, v11item.type());
        }
        void NonIncrSclr_7()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
               "NonIncrSclr is incremental should become 0",
                        false, v11item.isIncremental());
        }

        void NonIncrSclr_8()
        {
            CPPUNIT_ASSERT_MESSAGE(
               "NonIncrSclr scaler values should remain the same",
                        values == v11item.getScalers());
        }

        // the final stipulation is that the event timestamp of the v10 item is
        // discarded. This happens naturally because there is no place for the
        // data element in the v11 scaler item. We can't really test this.
    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to11p0Tests_NonIncrSclr);

    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////

    /*!
     * \brief The CTransform10p0to11p0Tests_Fragment class
     *
     * Tests that rules are obeyed for fragment types.
     */
    class CTransform10p0to11p0Tests_Fragment : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform10p0to11p0Tests_Fragment);
        CPPUNIT_TEST(Fragment_0);
        CPPUNIT_TEST(Fragment_1);
        CPPUNIT_TEST(Fragment_2);
        CPPUNIT_TEST(Fragment_3);
        CPPUNIT_TEST(Fragment_4);
        CPPUNIT_TEST(Fragment_5);
        CPPUNIT_TEST(Fragment_6);
        CPPUNIT_TEST(Fragment_7);
        CPPUNIT_TEST(Fragment_8);
        CPPUNIT_TEST_SUITE_END();

    public:
        V10::CRingFragmentItem v10item;
        V11::CRingFragmentItem v11item;
        CTransform10p0to11p0 m_transform;
        std::time_t time_now;
        std::vector<uint8_t> values;

    public:
        CTransform10p0to11p0Tests_Fragment()
            : v10item(0,0,0,0,0),
              v11item(0,0,0,0,0),
              time_now(),
              values({}) {}

        void setUp()
        {
            using namespace std::chrono;

            // force this be different than now by 1
            time_now = system_clock::to_time_t( system_clock::now() ) + 1;
            values   = vector<uint8_t>({0, 1, 2, 3});

            m_transform = CTransform10p0to11p0();

            v10item     = V10::CRingFragmentItem(1234567, // timestamp
                                                       3,  // source id
                                                       values.size(),  // payload size
                                                       values.data(), // payload data
                                                       10); // barrier

            // do the transform
            v11item     = m_transform(v10item);
        }

        void tearDown() {

        }

        void Fragment_0()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Fragment item should have a body header",
                                         true, v11item.hasBodyHeader());

        }

        void Fragment_1()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Fragment timestamp should remain the same",
                                         uint64_t(1234567), v11item.getEventTimestamp());
        }

        void Fragment_2()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Fragment timestamp should remain same",
                                         uint64_t(1234567), v11item.timestamp());
        }
        void Fragment_3()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Fragment source id should reamin same",
                                         uint32_t(3), v11item.getSourceId());
        }

        void Fragment_4()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Fragment source id should reamin same",
                                         uint32_t(3), v11item.source());
        }
        void Fragment_5()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                        "Fragment barrier type should remain unchanged",
                        uint32_t(10), v11item.getBarrierType());
        }

        void Fragment_6()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
                        "Fragment barrier type should remain unchanged",
                        uint32_t(10), v11item.barrierType());
        }
        void Fragment_7()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE(
               "Fragment payload size should remain the same",
                        size_t(values.size()), v11item.payloadSize());
        }

        void Fragment_8()
        {
            uint8_t* pBody = reinterpret_cast<uint8_t*>(v11item.payloadPointer());
            CPPUNIT_ASSERT_MESSAGE(
               "Fragment payload data should remain the same",
                        equal(begin(values), end(values), pBody) );
        }

        // the final stipulation is that the event timestamp of the v10 item is
        // discarded. This happens naturally because there is no place for the
        // data element in the v11 scaler item. We can't really test this.
    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to11p0Tests_Fragment);
