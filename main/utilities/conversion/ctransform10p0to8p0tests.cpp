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
#include <V10/CRingTimestampedRunningScalerItem.h>

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
#include <CTransform10p0to8p0.h>
#undef protected
#undef private

#include <iterator>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <cassert>
#include <numeric>

using namespace std;

using namespace DAQ;


/*!
 * \brief The CTransform10p0to8p0Tests_Scaler class
 *
 * Tests that scaler transforms obey the rules.
 */
class CTransform10p0to8p0Tests_Scaler : public CppUnit::TestFixture
{
public:
    CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_Scaler);
    CPPUNIT_TEST(Scaler_0);
    CPPUNIT_TEST(Scaler_1);
    CPPUNIT_TEST(Scaler_2);
    CPPUNIT_TEST(Scaler_4);
    CPPUNIT_TEST(Scaler_5);
    CPPUNIT_TEST(Scaler_6);
    CPPUNIT_TEST(Scaler_7);
    CPPUNIT_TEST(Scaler_8);
    CPPUNIT_TEST(Scaler_9);
    CPPUNIT_TEST(Scaler_10);
    CPPUNIT_TEST(Scaler_11);
    CPPUNIT_TEST(Scaler_12);
    CPPUNIT_TEST_SUITE_END();

protected:
    Transform::CTransform10p0to8p0 m_transform;
    vector<uint32_t> m_scalers;
    V8::CRawBuffer v8item;
    std::uint32_t m_offsetBegin;
    std::uint32_t m_offsetEnd;
    std::time_t m_tstamp;
    std::tm     m_calTime;


public:
    CTransform10p0to8p0Tests_Scaler() :
      m_transform(),
      m_scalers(), v8item(),
      m_offsetBegin(0),
      m_offsetEnd(0),
      m_tstamp(0),
      m_calTime() {}

    virtual void setUp() {
        m_transform = Transform::CTransform10p0to8p0();
        m_transform.setCurrentRunNumber(234);

        m_scalers = vector<uint32_t>(16);
        iota(m_scalers.begin(), m_scalers.end(), 0);

        m_offsetBegin = 0x1234;
        m_offsetEnd   = 0x5678;

        m_tstamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm* pTime = std::localtime(&m_tstamp);

        if (pTime != nullptr) {
          m_calTime = *pTime;
        }

        V10::CRingScalerItem scaler(m_offsetBegin, m_offsetEnd, m_tstamp, m_scalers);

        v8item = m_transform( scaler );

    }
  virtual void tearDown() {
  }

  void Scaler_0()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("INCREMENTAL_SCALERS --> SCALERBF",
                                 std::uint16_t(V8::SCALERBF), v8item.getHeader().type);
  }

  void Scaler_1()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Checksum is set to 0",
                                 std::uint16_t(0), v8item.getHeader().cks);
  }

  void Scaler_2()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Run number is set to the value known by the transform",
                                 std::uint16_t(234), v8item.getHeader().run);
  }

  void Scaler_4()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("nevt will be set to scaler count",
                                 std::uint16_t(m_scalers.size()), v8item.getHeader().nevt);
  }

  void Scaler_5()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("processor number set to 0",
                                 std::uint16_t(0), v8item.getHeader().cpu);

  }
  void Scaler_6()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("nbit set to 0",
                                 std::uint16_t(0), v8item.getHeader().nbit);
  }

  void Scaler_7()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("buffmt set to 5",
                                 std::uint16_t(5), v8item.getHeader().buffmt);
  }

  void Scaler_8()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("ssignature set to 0x0102",
                                 std::uint16_t(0x0102),
                                 v8item.getHeader().ssignature);
  }

  void Scaler_9()
  {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("lsignature set to 0x0102",
                                 std::uint32_t(0x01020304),
                                 v8item.getHeader().lsignature);
  }

  void Scaler_10()
  {
    auto sclBuf = V8::format_cast<V8::CScalerBuffer>(v8item);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("interval start offset remains the same",
                                 std::uint32_t(0x1234), sclBuf.getOffsetBegin());
  }

  void Scaler_11()
  {
    auto sclBuf = V8::format_cast<V8::CScalerBuffer>(v8item);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("interval end offset remains the same",
                                 std::uint32_t(0x5678), sclBuf.getOffsetEnd());
  }

  void Scaler_12()
  {
    auto sclBuf = V8::format_cast<V8::CScalerBuffer>(v8item);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("scalers should remain the same",
                                 m_scalers, sclBuf.getScalers());
  }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_Scaler);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/*!
 * \brief The CTransform10p0to8p0Tests_NonIncrScaler class
 *
 * Test that non-incremental scalers transform according to the rules
 */
class CTransform10p0to8p0Tests_NonIncrScaler : public CTransform10p0to8p0Tests_Scaler
{
public:
  CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_NonIncrScaler);
  CPPUNIT_TEST(Scaler_1);
  CPPUNIT_TEST(Scaler_2);
  CPPUNIT_TEST(Scaler_4);
  CPPUNIT_TEST(Scaler_5);
  CPPUNIT_TEST(Scaler_6);
  CPPUNIT_TEST(Scaler_7);
  CPPUNIT_TEST(Scaler_8);
  CPPUNIT_TEST(Scaler_9);
  CPPUNIT_TEST(Scaler_10);
  CPPUNIT_TEST(Scaler_11);
  CPPUNIT_TEST(Scaler_12);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {
    m_transform = Transform::CTransform10p0to8p0();
    m_transform.setCurrentRunNumber(234);

    m_scalers = vector<uint32_t>(16);
    iota(m_scalers.begin(), m_scalers.end(), 0);

    m_offsetBegin = 0x1234;
    m_offsetEnd   = 0x5678;

    m_tstamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm* pTime = std::localtime(&m_tstamp);

    if (pTime != nullptr) {
      m_calTime = *pTime;
    }

    V10::CRingTimestampedRunningScalerItem scaler(0x123456789ab, m_offsetBegin,m_offsetEnd,
                                                        1, m_tstamp, m_scalers);

    v8item = m_transform( scaler );
  }

  void tearDown() {}

  void Scaler_13 () {
    CPPUNIT_ASSERT_EQUAL_MESSAGE("TIMESTAMPED_NONINCR_SCALERS --> SCALERBF",
                                 std::uint16_t(V8::SCALERBF), v8item.getHeader().type);

  }
};
CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_NonIncrScaler);

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    /*!
     * \brief The CTransform10p0to8p0Tests_Text class
     *
     * Make sure V10::CRingTextItems transform according to the rules
     */
    class CTransform10p0to8p0Tests_Text : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_Text);
        CPPUNIT_TEST(Text_0);
        CPPUNIT_TEST(Text_1);
        CPPUNIT_TEST(Text_2);
        CPPUNIT_TEST(Text_4);
        CPPUNIT_TEST(Text_5);
        CPPUNIT_TEST(Text_6);
        CPPUNIT_TEST(Text_7);
        CPPUNIT_TEST(Text_8);
        CPPUNIT_TEST(Text_9);
        CPPUNIT_TEST(Text_10);
        CPPUNIT_TEST(Text_11);
        CPPUNIT_TEST_SUITE_END();

    public:
        V8::CRawBuffer                v8item;
        V10::CRingTextItem       v10item;
        Transform::CTransform10p0to8p0 m_transform;
        std::vector<std::string>       m_strings;
        std::uint32_t                  m_offsetTime;

    public:
        // We need to define a default constructor b/c the CRingTextItem classes
        // do not define a default constructor.
        CTransform10p0to8p0Tests_Text()
          : v8item(), v10item(V10::MONITORED_VARIABLES),
            m_transform(),
            m_strings() ,
            m_offsetTime() {}

        void setUp()
        {
          V8::gBufferSize = 70;
          m_transform = Transform::CTransform10p0to8p0();

          m_strings = {"why", "did", "the", "chicken", "cross", "the", "road?"};

          std::time_t m_tstamp = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

          v10item = V10::CRingTextItem(V10::MONITORED_VARIABLES,
                                        m_strings, m_offsetTime, m_tstamp);

          v8item = m_transform( v10item );
        }

        void tearDown() {
          V8::gBufferSize = 8192;
        }

        void Text_0()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("MONITORED_VARIABLES --> RUNVARBF",
                                       std::uint16_t(V8::RUNVARBF), v8item.getHeader().type);
        }

        void Text_1()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Checksum is set to 0",
                                       std::uint16_t(0), v8item.getHeader().cks);
        }

        void Text_2()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Run number is set to 0",
                                       std::uint16_t(0), v8item.getHeader().run);
        }

        void Text_4()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("nevt will be set to number of strings",
                                       std::uint16_t(m_strings.size()), v8item.getHeader().nevt);
        }

        void Text_5()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("processor number set to 0",
                                       std::uint16_t(0), v8item.getHeader().cpu);

        }
        void Text_6()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("nbit set to 0",
                                       std::uint16_t(0), v8item.getHeader().nbit);
        }

        void Text_7()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("buffmt set to 5",
                                       std::uint16_t(5), v8item.getHeader().buffmt);
        }

        void Text_8()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("ssignature set to 0x0102",
                                       std::uint16_t(0x0102),
                                       v8item.getHeader().ssignature);
        }

        void Text_9()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("lsignature set to 0x0102",
                                       std::uint32_t(0x01020304),
                                       v8item.getHeader().lsignature);
        }

        void Text_10()
        {
          V8::CTextBuffer textBuf = V8::format_cast<V8::CTextBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("Strings will be copied in unaltered",
                                       m_strings, textBuf.getStrings());
        }

        void Text_11()
        {
          std::time_t tstamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

          V10::CRingTextItem text(V10::PACKET_TYPES, m_strings, m_offsetTime, tstamp);

          m_transform = DAQ::Transform::CTransform10p0to8p0();
          v8item = m_transform( text );

          V8::CTextBuffer buffer = V8::format_cast<V8::CTextBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("PACKET_TYPES --> PKTDOCBF",
                                       V8::PKTDOCBF, buffer.type());
        }

    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_Text);

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    /*!
     * \brief The CTransform10p0to8p0Tests_MultiText class
     *
     * Makes sure that V10::CRingTextItems that overflow the V8 buffers are handled
     * properly
     */
    class CTransform10p0to8p0Tests_MultiText : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_MultiText);
        CPPUNIT_TEST(MultiText_0);
        CPPUNIT_TEST(MultiText_1);
        CPPUNIT_TEST(MultiText_2);
        CPPUNIT_TEST_SUITE_END();

    public:
        V8::CRawBuffer                 v8item;
        V10::CRingTextItem       v10item;
        Transform::CTransform10p0to8p0 m_transform;
        std::vector<std::string>       m_strings;
        std::uint32_t                  m_offsetTime;

    public:
        // We need to define a default constructor b/c the CRingTextItem classes
        // do not define a default constructor.
        CTransform10p0to8p0Tests_MultiText()
          : v8item(), v10item(V10::MONITORED_VARIABLES),
            m_transform(),
            m_strings() ,
            m_offsetTime() {}

        void setUp()
        {
          V8::gBufferSize = 43; // just big enough to fit two 3-letter words each buffer
          m_transform = Transform::CTransform10p0to8p0();

          m_strings = {"why", "did", "the","cat", "nap"};

          std::time_t tstamp = std::chrono::system_clock::to_time_t( std::chrono::system_clock::now() );

          v10item = V10::CRingTextItem(V10::MONITORED_VARIABLES,
                                        m_strings, m_offsetTime, tstamp);
        }

        void tearDown() {

        }

        void MultiText_0()
        {
          v8item = m_transform( v10item );

          CPPUNIT_ASSERT_EQUAL_MESSAGE("First component of CRingTextItem should be returned",
                                       std::uint16_t(V8::RUNVARBF), v8item.getHeader().type);
        }

        void MultiText_1()
        {
          v8item = m_transform( v10item );

          V8::CTextBuffer textBuf = V8::format_cast<V8::CTextBuffer>(v8item);
          CPPUNIT_ASSERT_EQUAL_MESSAGE("The returned buffer should contain first 2 words only",
                                       std::size_t(2), textBuf.getStrings().size());
        }

        void MultiText_2()
        {
          v8item = m_transform( v10item );

          V8::CTextBuffer textBuf = V8::format_cast<V8::CTextBuffer>(v8item);
          CPPUNIT_ASSERT_EQUAL_MESSAGE("After the first is returned, there should be two ready to be returned",
                                       std::size_t(2), m_transform.getStagedTextBuffers().size());
        }

    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_MultiText);

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    /*!
     * \brief The CTransform10p0to8p0Tests_PhysicsEvent class
     *
     * Ensures that V8::CPhysicsEventItems convert as we would expect them to
     */
    class CTransform10p0to8p0Tests_PhysicsEvent : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_PhysicsEvent);
        CPPUNIT_TEST(Event_0);
        CPPUNIT_TEST(Event_1);
        CPPUNIT_TEST(Event_2);
        CPPUNIT_TEST(Event_3);
        CPPUNIT_TEST(Event_4);
        CPPUNIT_TEST(Event_5);
        CPPUNIT_TEST(Event_6);
        CPPUNIT_TEST(Event_7);
        CPPUNIT_TEST(Event_8);
        CPPUNIT_TEST(Event_9);
        CPPUNIT_TEST(Event_10);
        CPPUNIT_TEST(Event_11);
        CPPUNIT_TEST_SUITE_END();

    public:
        V8::CRawBuffer        v8item;
        Transform::CTransform10p0to8p0 m_transform;
        Buffer::ByteBuffer             m_bodyData;

    public:
        // We need to define a default constructor b/c the CRingTextItem classes
        // do not define a default constructor.
        CTransform10p0to8p0Tests_PhysicsEvent()
          : v8item(),
            m_transform(),
            m_bodyData() {}

        void setUp()
        {

          V8::gBufferSize = 40; // select a size that will automatically flush
          m_transform = Transform::CTransform10p0to8p0();

          V10::CPhysicsEventItem event(V10::PHYSICS_EVENT, 8192);
          m_bodyData << std::vector<std::uint16_t>({4, 0, 1, 2});

          event.fillBody(m_bodyData);


          v8item = m_transform( event );

        }

        void tearDown() {
          V8::gBufferSize = 8192;
        }

        void Event_0()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("PHYSICS_EVENT --> DATABF",
                                       V8::DATABF,
                                       V8::BufferTypes(v8item.getHeader().type));
        }

        void Event_1()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Checksum is set to 0",
                                       std::uint16_t(0), v8item.getHeader().cks);
        }

        void Event_2()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Run number is set to 0",
                                       std::uint16_t(0), v8item.getHeader().run);
        }

        void Event_3()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("nevt will be set to 1",
                                       std::uint16_t(1), v8item.getHeader().nevt);
        }

        void Event_4()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("processor number set to 0",
                                       std::uint16_t(0), v8item.getHeader().cpu);

        }
        void Event_5()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("nbit set to 0",
                                       std::uint16_t(0), v8item.getHeader().nbit);
        }

        void Event_6()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("buffmt set to 5",
                                       std::uint16_t(5), v8item.getHeader().buffmt);
        }

        void Event_7()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("nlam set to 0",
                                       std::uint16_t(0), v8item.getHeader().nlam);
        }

        void Event_8()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("ssignature set to 0x0102",
                                       std::uint16_t(0x0102),
                                       v8item.getHeader().ssignature);
        }

        void Event_9()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("lsignature set to 0x0102",
                                       std::uint32_t(0x01020304),
                                       v8item.getHeader().lsignature);
        }

        void Event_10()
        {
          V8::CPhysicsEventBuffer evtBuf = V8::format_cast<V8::CPhysicsEventBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("PhysicsEventBuffer contains only 1 event",
                                       std::size_t(1), evtBuf.size());
        }

        void Event_11()
        {
          V8::CPhysicsEventBuffer evtBuf = V8::format_cast<V8::CPhysicsEventBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("PhysicsEvent remains the same",
                                       m_bodyData, evtBuf.at(0)->getBuffer());
        }


    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_PhysicsEvent);

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    /*!
     * \brief The CTransform10p0to8p0Tests_MultiPhysicsEvent class
     *
     * Verify that the buffering capabibilities of the transform behave appropriately
     * for physics events.
     */
    class CTransform10p0to8p0Tests_MultiPhysicsEvent : public CppUnit::TestFixture
    {
    public:
    CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_MultiPhysicsEvent);
    CPPUNIT_TEST(MultiEvent_0);
    CPPUNIT_TEST(MultiEvent_1);
    CPPUNIT_TEST(MultiEvent_2);
    CPPUNIT_TEST(MultiEvent_3);
    CPPUNIT_TEST_SUITE_END();

    private:
      Buffer::ByteBuffer m_bodyData;
      V8::CRawBuffer     v8item;
      V10::CPhysicsEventItem v10item;
      Transform::CTransform10p0to8p0 m_transform;

    public:
      CTransform10p0to8p0Tests_MultiPhysicsEvent() : m_bodyData(), v8item(),
        v10item(V10::PHYSICS_EVENT), m_transform() {}

      void setUp() {
        V8::gBufferSize = 41; // select a size that will require two transforms to flush
        m_transform = Transform::CTransform10p0to8p0();

        v10item = V10::CPhysicsEventItem(V10::PHYSICS_EVENT, 8192);
        m_bodyData << std::vector<std::uint16_t>({4, 0, 1, 2});

        v10item.fillBody(m_bodyData);

      }

      void tearDown() {
        V8::gBufferSize = 8192;
      }

      void MultiEvent_0 () {
        v8item = m_transform( v10item );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Return VOID when space remains in buffer",
                                     std::uint16_t(V8::VOID), v8item.getHeader().type);
      }

      void MultiEvent_1 () {
        v8item = m_transform( v10item );
        v8item = m_transform( v10item );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Return DATABF when data needs to be flushed",
                                     std::uint16_t(V8::DATABF), v8item.getHeader().type);
      }

      void MultiEvent_2 () {
        v8item = m_transform( v10item );
        v8item = m_transform( v10item );

        V8::CPhysicsEventBuffer evtBuf = V8::format_cast<V8::CPhysicsEventBuffer>(v8item);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Return DATABF with expected number of physics events",
                                     std::size_t(1), evtBuf.size() );
      }

      void MultiEvent_3 () {
        v8item = m_transform( v10item );
        v8item = m_transform( v10item );

        CPPUNIT_ASSERT_EQUAL_MESSAGE("After flush, transform object should have started a new physics event",
                                     std::size_t(1), m_transform.getCurrentPhysicsBuffer().size() );
      }

    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_MultiPhysicsEvent);

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////


    /*!
     * \brief The CTransform10p0to8p0Tests_Control class
     *
     * Verifies that the V10::CRingStateChangeItem transforms appropriately
     */
    class CTransform10p0to8p0Tests_Control : public CppUnit::TestFixture
    {

        public:
        CPPUNIT_TEST_SUITE(CTransform10p0to8p0Tests_Control);
        CPPUNIT_TEST(Control_0);
        CPPUNIT_TEST(Control_1);
        CPPUNIT_TEST(Control_2);
        CPPUNIT_TEST(Control_3);
        CPPUNIT_TEST(Control_4);
        CPPUNIT_TEST(Control_5);
        CPPUNIT_TEST(Control_6);
        CPPUNIT_TEST(Control_7);
        CPPUNIT_TEST(Control_8);
        CPPUNIT_TEST(Control_9);
        CPPUNIT_TEST(Control_10);
        CPPUNIT_TEST(Control_11);
        CPPUNIT_TEST(Control_12);
        CPPUNIT_TEST(Control_13);
        CPPUNIT_TEST(Control_14);
        CPPUNIT_TEST(Control_15);
        CPPUNIT_TEST(Control_16);
        CPPUNIT_TEST(Control_17);
        CPPUNIT_TEST(Control_18);
        CPPUNIT_TEST(Control_19);
        CPPUNIT_TEST(Control_20);
        CPPUNIT_TEST(Control_21);
        CPPUNIT_TEST(Control_22);
        CPPUNIT_TEST(Control_23);
        CPPUNIT_TEST(Control_24);
        CPPUNIT_TEST_SUITE_END();

    public:
        V8::CRawBuffer v8item;
        Transform::CTransform10p0to8p0 m_transform;
        std::uint32_t m_offset;
        std::string m_title;
        std::uint16_t m_run;
        std::tm     m_calTime;
    public:
        // We need to define a default constructor b/c the CRingTextItem classes
        // do not define a default constructor.
        CTransform10p0to8p0Tests_Control()
          : v8item(),
            m_transform()
        {}

        void setUp()
        {
          m_transform = Transform::CTransform10p0to8p0();

          m_transform.setNTriggersProcessed(10);

          m_title = "a title for you and me";

          m_offset = 0x12345678;

          m_run = 1357;

          V10::CRingStateChangeItem v10item = setUpType(V10::END_RUN);

          v8item = m_transform( v10item );
        }

        V10::CRingStateChangeItem setUpType(std::uint16_t type)
        {
          using namespace std::chrono;
          std::time_t now = system_clock::to_time_t(system_clock::now());
          std::tm* pTime = std::localtime(&now);
          if (pTime != nullptr) {
            m_calTime = *pTime;
          }

          return V10::CRingStateChangeItem(type, m_run, m_offset, now, m_title);
        }

        void tearDown() {

        }

        void Control_0()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("END_RUN --> ENDRUNBF",
                                       std::uint16_t(V8::ENDRUNBF), v8item.getHeader().type);
        }

        void Control_1()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Checksum is set to 0",
                                       std::uint16_t(0), v8item.getHeader().cks);
        }

        void Control_2()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("Run number is mapped correctly",
                                       std::uint16_t(1357), v8item.getHeader().run);
        }

        void Control_3()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("nevt will be set to 0",
                                       std::uint16_t(0), v8item.getHeader().nevt);
        }

        void Control_4()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("processor number set to 0",
                                       std::uint16_t(0), v8item.getHeader().cpu);

        }
        void Control_5()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("nbit set to 0",
                                       std::uint16_t(0), v8item.getHeader().nbit);
        }

        void Control_6()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("buffmt set to 5",
                                       std::uint16_t(5), v8item.getHeader().buffmt);
        }

        void Control_7()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("nlam set to 0",
                                       std::uint16_t(0), v8item.getHeader().nlam);
        }

        void Control_8()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("ssignature set to 0x0102",
                                       std::uint16_t(0x0102),
                                       v8item.getHeader().ssignature);
        }

        void Control_9()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("lsignature set to 0x0102",
                                       std::uint32_t(0x01020304),
                                       v8item.getHeader().lsignature);
        }

        void Control_10()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("run number is stored by transformer",
                                       std::uint16_t(1357),
                                       m_transform.getCurrentRunNumber());
        }

        void Control_11()
        {
          CPPUNIT_ASSERT_EQUAL_MESSAGE("sequence number known by transformer is used",
                                       std::uint32_t(10),
                                       v8item.getHeader().seq);
        }

        void Control_12()
        {
          auto ctlBuf = V8::format_cast<V8::CControlBuffer>(v8item);

          m_title.resize(80, ' ');
          CPPUNIT_ASSERT_MESSAGE("title will transform the same for first 79 characters",
                                 std::equal(m_title.begin(), m_title.begin()+79, ctlBuf.getTitle().begin()));
        }

        void Control_13()
        {
          auto ctlBuf = V8::format_cast<V8::CControlBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("80th character of title will be a null terminator",
                                 '\0', ctlBuf.getTitle().at(79));
        }

        void Control_14()
        {
          auto ctlBuf = V8::format_cast<V8::CControlBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("Time offset will remain unchanged",
                                 m_offset, ctlBuf.getOffset());
        }

        void Control_15()
        {
          auto ctlBuf = V8::format_cast<V8::CControlBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("month will map according to std::localtime",
                                       std::uint16_t(m_calTime.tm_mon), ctlBuf.getTimeStruct().month);
        }

        void Control_16()
        {
          auto ctlBuf = V8::format_cast<V8::CControlBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("day will map according to std::localtime",
                                 std::uint16_t(m_calTime.tm_mday), ctlBuf.getTimeStruct().day);
        }

        void Control_17()
        {
          auto ctlBuf = V8::format_cast<V8::CControlBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("year will map according to std::localtime + 1900",
                                 std::uint16_t(m_calTime.tm_year+1900), ctlBuf.getTimeStruct().year);
          CPPUNIT_ASSERT_EQUAL_MESSAGE("year will map according to std::localtime + 1900",
                                 std::uint16_t(m_calTime.tm_year + 1900), ctlBuf.getTimeStruct().year);

        }

        void Control_18()
        {
          auto ctlBuf = V8::format_cast<V8::CControlBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("hours will map according to std::localtime",
                                 std::uint16_t(m_calTime.tm_hour), ctlBuf.getTimeStruct().hours);
        }

        void Control_19()
        {
          auto ctlBuf = V8::format_cast<V8::CControlBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("minutes will map according to std::localtime",
                                 std::uint16_t(m_calTime.tm_min), ctlBuf.getTimeStruct().min);
        }

        void Control_20()
        {
          auto ctlBuf = V8::format_cast<V8::CControlBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("seconds will map according to std::localtime",
                                 std::uint16_t(m_calTime.tm_sec), ctlBuf.getTimeStruct().sec);
        }

        void Control_21()
        {
          auto ctlBuf = V8::format_cast<V8::CControlBuffer>(v8item);

          CPPUNIT_ASSERT_EQUAL_MESSAGE("tenths will become 0",
                                 std::uint16_t(0), ctlBuf.getTimeStruct().tenths);
        }

        void Control_22()
        {
          auto v10item= setUpType(V10::BEGIN_RUN);

          v8item = m_transform( v10item );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("BEGIN_RUN --> BEGRUNBF",
                                 std::uint16_t(V8::BEGRUNBF), v8item.getHeader().type);
        }

        void Control_23()
        {
          auto v10item= setUpType(V10::PAUSE_RUN);

          v8item = m_transform( v10item );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("PAUSE_RUN --> PAUSEBF",
                                 std::uint16_t(V8::PAUSEBF), v8item.getHeader().type);
        }

        void Control_24()
        {
          auto v10item= setUpType(V10::RESUME_RUN);

          v8item = m_transform( v10item );
          CPPUNIT_ASSERT_EQUAL_MESSAGE("RESUME_RUN --> RESUMEBF",
                                 std::uint16_t(V8::RESUMEBF), v8item.getHeader().type);
        }
    };
    CPPUNIT_TEST_SUITE_REGISTRATION(CTransform10p0to8p0Tests_Control);



