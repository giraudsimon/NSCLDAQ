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

#include <V8/CScalerBuffer.h>
#include <V10/CRingScalerItem.h>

#include <V8/CControlBuffer.h>
#include <V10/CRingStateChangeItem.h>

#include <V8/CPhysicsEventBuffer.h>
#include <V10/CPhysicsEventItem.h>

#include <V8/CTextBuffer.h>
#include <V10/CRingTextItem.h>

#include <V8/DataFormatV8.h>
#include <V10/DataFormatV10.h>

#include <V8/format_cast.h>
#include <DebugUtils.h>

#define private public
#define protected public
#include <CTransform8p0to10p0.h>
#undef protected
#undef private

#include <iterator>
#include <algorithm>
#include <ctime>
#include <numeric>


using namespace std;

using namespace DAQ;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class CTransform8p0to10p0Tests_Scaler : public CppUnit::TestFixture
{
private:
    V8::bheader m_header;
    Transform::CTransform8p0to10p0 m_transform;
    vector<uint32_t> m_scalers;
    V10::CRingScalerItem m_item;
    std::uint32_t m_offsetBegin;
    std::uint32_t m_offsetEnd;

public:
    CPPUNIT_TEST_SUITE(CTransform8p0to10p0Tests_Scaler);
    CPPUNIT_TEST(scaler_0);
    CPPUNIT_TEST(scaler_1);
    CPPUNIT_TEST(scaler_2);
    CPPUNIT_TEST(scaler_4);
    CPPUNIT_TEST(scaler_5);
    CPPUNIT_TEST_SUITE_END();

public:
    CTransform8p0to10p0Tests_Scaler() : m_header(), m_transform(),
      m_scalers(), m_item(V10::INCREMENTAL_SCALERS),
      m_offsetBegin(0), m_offsetEnd(0) {}

    void setUp() {
        m_transform = Transform::CTransform8p0to10p0();

        m_header.type = V8::SCALERBF;
        m_header.nevt = 16;
        m_header.nwds = 33;
        m_header.buffmt = V8::StandardVsn;
        m_header.ssignature = V8::BOM16;
        m_header.lsignature = V8::BOM32;

        m_scalers = vector<uint32_t>(16);
        iota(m_scalers.begin(), m_scalers.end(), 0);

        m_offsetBegin = 0x1234;
        m_offsetEnd = 0x5678;
        V8::CScalerBuffer scaler(m_header, m_offsetBegin, m_offsetEnd, m_scalers);

        m_item = m_transform( V8::format_cast<V8::CRawBuffer>(scaler) );

    }
  void tearDown() {
  }

protected:
  void scaler_0() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Type transforms to INCREMENTAL_SCALERS",
                                 V10::INCREMENTAL_SCALERS, m_item.type());
  }

  void scaler_1() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Interval offset begin value is preserved",
                                 m_offsetBegin, m_item.getStartTime());
  }

  void scaler_2() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Interval offset end value is preserved",
                                 m_offsetEnd, m_item.getEndTime());
  }

//  void scaler_3() {
//    CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp is what we demand to be returned",
//                                 DEFAULT_TSTAMP, m_item.getTimestamp());
//  }

  void scaler_4() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("scaler count must be the same as m_header.nevt",
                                 std::uint32_t(m_header.nevt), m_item.getScalerCount());
  }

  void scaler_5() {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("scaler values must be the same",
                                 m_scalers, m_item.getScalers());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform8p0to10p0Tests_Scaler);



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


    class CTransform8p0to10p0Tests_Text : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform8p0to10p0Tests_Text);
        CPPUNIT_TEST(Text_0);
        //CPPUNIT_TEST(Text_1);
        CPPUNIT_TEST(Text_2);
        CPPUNIT_TEST(Text_4);
        CPPUNIT_TEST(Text_5);
        CPPUNIT_TEST(Text_6);
        CPPUNIT_TEST(Text_7);
        CPPUNIT_TEST_SUITE_END();

    public:
        V8::bheader m_header;
        V10::CRingTextItem v10item;
        Transform::CTransform8p0to10p0 m_transform;
        std::vector<std::string> m_strings;

    public:
        // We need to define a default constructor b/c the CRingTextItem classes
        // do not define a default constructor.
        CTransform8p0to10p0Tests_Text() : v10item(V10::MONITORED_VARIABLES, {}),
            m_transform(),
            m_strings() {}

        void setUp()
        {
          m_transform = Transform::CTransform8p0to10p0();

          m_header.type = V8::PKTDOCBF;
          m_header.nevt = 7;
          m_header.nwds = 35; // i think this includes the size of the header?
          m_header.ssignature = V8::BOM16;
          m_header.lsignature = V8::BOM32;

          m_strings = {"why", "did", "the", "chicken", "cross", "the", "road?"};
          uint16_t nTotalWords = 1;
          for (auto & str : m_strings ) {
            nTotalWords += str.size() + 1;
          }

          V8::CTextBuffer text(m_header, m_strings);

          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(text) );
        }

        void tearDown() {

        }

        void Text_0()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Text item must have 0 as its time offset",
                                       uint32_t(0), v10item.getTimeOffset());
        }

// not sure how to test the next one b/c it depends on the output of the time() function.
        // I could play tricks with the include path, but that is non trivial.
//        void Text_1()
//        {
//          CPPUNIT_ASSERT_EQUAL_MESSAGE("Text item must have expected timestamp",
//                                       now, v10item.getTimestamp());
//        }

        void Text_2()
        {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Text item string count is same as header.nevt",
                                         uint32_t(m_header.nevt), v10item.getStringCount());
        }

        void Text_4()
        {
          auto strings = v10item.getStrings();
          for (std::size_t i=0; i<strings.size(); ++i) {
            CPPUNIT_ASSERT_EQUAL_MESSAGE("Text string must be the same",
                                         m_strings.at(i), strings.at(i));
          }
        }

        void Text_5()
        {

          CPPUNIT_ASSERT_EQUAL_MESSAGE("PKTDOCBF --> PACKET_TYPES",
                                       V10::PACKET_TYPES, v10item.type());
        }
        void Text_6()
        {
          m_header.type = V8::RUNVARBF;
          m_header.nevt = 16;
          m_header.nwds = 33; // i think this includes the size of the header?
          m_header.ssignature = V8::BOM16;
          m_header.lsignature = V8::BOM32;

          m_strings = {"why", "did", "the", "chicken", "cross", "the", "road?"};

          V8::CTextBuffer text(m_header, m_strings);

          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(text) );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("RUNVARBF --> MONITORED_VARIABLES",
                                  V10::MONITORED_VARIABLES, v10item.type());

        }

        void Text_7()
        {
          m_header.type = V8::STATEVARBF;
          m_header.nevt = 16;
          m_header.nwds = 33; // i think this includes the size of the header?
          m_header.ssignature = V8::BOM16;
          m_header.lsignature = V8::BOM32;

          m_strings = {"why", "did", "the", "chicken", "cross", "the", "road?"};

          V8::CTextBuffer text(m_header, m_strings);

          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(text) );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("STATEVARBF --> MONITORED_VARIABLES",
                                  V10::MONITORED_VARIABLES, v10item.type());
        }

    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform8p0to10p0Tests_Text);


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////


    class CTransform8p0to10p0Tests_PhysicsEvent : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform8p0to10p0Tests_PhysicsEvent);
        CPPUNIT_TEST(Event_0);
        CPPUNIT_TEST(Event_1);
        CPPUNIT_TEST(Event_2);
        CPPUNIT_TEST(Event_3);
        CPPUNIT_TEST_SUITE_END();

    public:
        V8::bheader m_header;
        V10::CPhysicsEventItem v10item;
        Transform::CTransform8p0to10p0 m_transform;
        Buffer::ByteBuffer m_body;

    public:
        // We need to define a default constructor b/c the CRingTextItem classes
        // do not define a default constructor.
        CTransform8p0to10p0Tests_PhysicsEvent()
          : v10item(8192),
            m_transform(),
            m_body() {}

        void setUp()
        {
          m_transform = Transform::CTransform8p0to10p0();

          m_header.type = V8::DATABF;
          m_header.buffmt = V8::StandardVsn;
          m_header.nevt = 1;
          m_header.nwds = 18; // i think this includes the size of the header?
          m_header.ssignature = V8::BOM16;
          m_header.lsignature = V8::BOM32;

          std::vector<std::uint16_t> body({2, 1234});

        m_body = Buffer::ByteBuffer();
          m_body << body;

          V8::CPhysicsEventBuffer event(m_header, body);

          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(event) );
        }

        void tearDown() {

        }

        void Event_0()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("DATABF --> PHYSICS_EVENT",
                                       V10::PHYSICS_EVENT, v10item.type());
        }

        void Event_1()
        {
          const char* pBody =reinterpret_cast<const char*>(v10item.getBodyPointer());

          // copy the body into something safer and easier to work with
          Buffer::ByteBuffer body(pBody, pBody+v10item.getBodySize());

          CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics event body remains the same",
                                       m_body, body);
        }

        void adaptSetupForMultipleEvents() {
          std::vector<std::uint16_t> body({2, 0x1234, 2, 0x5678, 2, 0x9abc});

          m_header.nevt = 3;
          m_header.nwds = 22;

          m_body = Buffer::ByteBuffer();
          m_body << body;

          V8::CPhysicsEventBuffer event(m_header, body);

          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(event) );
        }

        void Event_2()
        {
          adaptSetupForMultipleEvents();
          CPPUNIT_ASSERT_EQUAL_MESSAGE("2 more physics events are present",
                                       std::size_t(2), m_transform.getRemainingEvents().size());
        }


        void Event_3()
        {
          adaptSetupForMultipleEvents();

          v10item = m_transform.getRemainingEvents().front();

          const char* pBody = reinterpret_cast<const char*>(v10item.getBodyPointer());

          // copy the body into something safer and easier to work with
          Buffer::ByteBuffer bodyBytes(pBody, pBody+v10item.getBodySize());

          CPPUNIT_ASSERT_EQUAL_MESSAGE("Physics event body remains the same",
                                       Buffer::ByteBuffer({2, 0, 0x78, 0x56}), bodyBytes );
        }

    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform8p0to10p0Tests_PhysicsEvent);


    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    class CTransform8p0to10p0Tests_Control : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform8p0to10p0Tests_Control);
        CPPUNIT_TEST(Control_0);
        CPPUNIT_TEST(Control_1);
        CPPUNIT_TEST(Control_2);
        CPPUNIT_TEST(Control_3);
        CPPUNIT_TEST(Control_4);
        CPPUNIT_TEST(Control_5);
        CPPUNIT_TEST(Control_6);
        CPPUNIT_TEST(Control_7);
        CPPUNIT_TEST_SUITE_END();

    public:
        V8::bheader m_header;
        V10::CRingStateChangeItem v10item;
        Transform::CTransform8p0to10p0 m_transform;
        std::uint32_t m_offset;
        std::string m_title;
        DAQ::V8::bftime m_tstruct;

    public:
        // We need to define a default constructor b/c the CRingTextItem classes
        // do not define a default constructor.
        CTransform8p0to10p0Tests_Control()
          : v10item(V10::BEGIN_RUN),
            m_transform()
        {}

        void setUp()
        {
          m_transform = Transform::CTransform8p0to10p0();

          m_header.type = V8::BEGRUNBF;
          m_header.buffmt = V8::StandardVsn;
          m_header.nwds = 65;
          m_header.nevt = 0;
          m_header.run = 1357;
          m_header.ssignature = V8::BOM16;
          m_header.lsignature = V8::BOM32;

          m_title = "a title for you and me";
          m_title.resize(80, ' '); // needed b/c CControlBuffer stretches title to 80 chars
                                   // we must do the same if we are to use it to compare

          m_offset = 0x12345678;

          m_tstruct = {1, 2, 1971, 3, 4, 5, 6};

          V8::CControlBuffer ctlBuf(m_header, m_title, m_offset, m_tstruct);

          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(ctlBuf) );
        }

        void tearDown() {

        }

        void Control_0()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("BEGRUNBF --> BEGIN_RUN",
                                       V10::BEGIN_RUN, v10item.type());
        }

        void Control_1()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Title remains the same",
                                       m_title, v10item.getTitle());
        }

        void Control_2()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Offset remains the same",
                                       m_offset, v10item.getElapsedTime());
        }

        void Control_3()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Run number remains the same",
                                       static_cast<std::uint32_t>(m_header.run),
                                       v10item.getRunNumber());
        }

        void Control_4()
        {

          std::tm calTime;
          calTime.tm_mon  = m_tstruct.month;
          calTime.tm_mday = m_tstruct.day;
          calTime.tm_year = m_tstruct.year - 1900;
          calTime.tm_hour = m_tstruct.hours;
          calTime.tm_min  = m_tstruct.min;
          calTime.tm_sec  = m_tstruct.sec;
	  calTime.tm_isdst= -1;

          std::time_t time = std::mktime(&calTime);
          std::time_t time2 = std::mktime(&calTime);
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp converts as expected",
                                       time2, time);
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Timestamp converts as expected",
                                       time, v10item.getTimestamp());
        }

        void Control_5 () {
          m_header.type = V8::ENDRUNBF;

          V8::CControlBuffer ctlBuf(m_header, m_title, m_offset, m_tstruct);

          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(ctlBuf) );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("ENDRUNBF --> END_RUN",
                                       V10::END_RUN, v10item.type());

        }

        void Control_6 () {
          m_header.type = V8::PAUSEBF;

          V8::CControlBuffer ctlBuf(m_header, m_title, m_offset, m_tstruct);

          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(ctlBuf) );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("PAUSEBF --> PAUSE_RUN",
                                       V10::PAUSE_RUN, v10item.type());

        }

        void Control_7 () {
          m_header.type = V8::RESUMEBF;

          V8::CControlBuffer ctlBuf(m_header, m_title, m_offset, m_tstruct);

          v10item = m_transform( V8::format_cast<V8::CRawBuffer>(ctlBuf) );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("RESUMEBF --> RESUME_RUN",
                                       V10::RESUME_RUN, v10item.type());

        }

    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform8p0to10p0Tests_Control);


