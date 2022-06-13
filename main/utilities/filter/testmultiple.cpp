/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  testmultiple
 *  @brief: test the test source and sink, using test src and sink and multiple inserts
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CFilterTestSource.h"
#include "CFilterTestSink.h"
#include <CDataFormatItem.h>
#include <CGlomParameters.h>
#include <CRingStateChangeItem.h>
#include <CAbnormalEndItem.h>
#include <CDataFormatItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingScalerItem.h>
#include <time.h>
#include <DataFormat.h>
#include "CFilterMain.h"
#include "CMediator.h"
#include "CFilter.h"


#include "CFilterTestSource.h"
#include "CFilterTestSink.h"
#include <CDataFormatItem.h>
#include <CGlomParameters.h>
#include <CRingStateChangeItem.h>
#include <CAbnormalEndItem.h>
#include <CDataFormatItem.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingScalerItem.h>
#include <time.h>
#include <DataFormat.h>
#include "CFilterMain.h"
#include "CMediator.h"
#include "CFilter.h"


class testmultiple : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(testmultiple);
    CPPUNIT_TEST(src_1);
    CPPUNIT_TEST(src_2);
    CPPUNIT_TEST(src_3);
    CPPUNIT_TEST(sink_1);
    CPPUNIT_TEST(sink_2);
    CPPUNIT_TEST(sink_3);
    
    CPPUNIT_TEST(abend_1);
    CPPUNIT_TEST(emptyrun_1);
    CPPUNIT_TEST(scaler_1);
    // We could do more but let's face it. We've demonstrated at this point
    // that I can ask the filter to output arbitrary ring items
    // in addition to what I return back to the filter...win
    // We can put daqdev/NSCLDAQ#804 to bed as solved.
    
    CPPUNIT_TEST(sink_1);
    CPPUNIT_TEST(sink_2);
    CPPUNIT_TEST(sink_3);
    
    CPPUNIT_TEST(abend_1);
    CPPUNIT_TEST(emptyrun_1);
    CPPUNIT_TEST(scaler_1);
    // We could do more but let's face it. We've demonstrated at this point
    // that I can ask the filter to output arbitrary ring items
    // in addition to what I return back to the filter...win
    // We can put daqdev/NSCLDAQ#804 to bed as solved.
    CPPUNIT_TEST_SUITE_END();

protected:
    void src_1();
    void src_2();
    void src_3();
    
    void sink_1();
    void sink_2();
    void sink_3();
    
    void abend_1();
    void emptyrun_1();
    void scaler_1();

private:
    CFilterTestSource* m_pSrc;
    CFilterTestSink*   m_pSink;
    CFilterMain*       m_pFilter;

public:
    void setUp() {
        m_pSrc = new CFilterTestSource;
        m_pSink= new CFilterTestSink;
        
        CFilterMain::m_testing = true;
        const char* filterargs[4] = {
            "filter", "--source=-", "--sink=-", "--oneshot"
        };
        m_pFilter = new CFilterMain(4, const_cast<char**>(filterargs));
        
    }
    void tearDown() {
        delete m_pSrc;
        delete m_pSink;
        delete m_pFilter;
    }


};

CPPUNIT_TEST_SUITE_REGISTRATION(testmultiple);


void testmultiple::src_1()
{
    // Empty data source is an immediate eof:
    
    EQ((CRingItem*)(nullptr), m_pSrc->getItem());
}
void testmultiple::src_2()
{
    // If a put a ring item in the first returned item should
    
    // be equal to the one I took out.  We'll us a ring format item.
    
    CDataFormatItem item;
    m_pSrc->addItem(&item);
    
    CRingItem* pItem = m_pSrc->getItem();
    ASSERT(item == (*pItem));
    
    delete pItem;
    
    EQ((CRingItem*)(nullptr), m_pSrc->getItem());  // No more.
}
void testmultiple::src_3()
{

    // If a put a few items in the src they come out in the right order:
    
    CDataFormatItem item1;
    CGlomParameters item2(100, true, CGlomParameters::first);
    CRingStateChangeItem item3(BEGIN_RUN, 1, 0, time(nullptr), "The title string");
    CRingStateChangeItem item4(END_RUN, 1, 100, time(nullptr), "The title string");
    std::vector<CRingItem*> items = {&item1, &item2, &item3, &item4};
    
    m_pSrc->addItem(&item1);
    m_pSrc->addItem(&item2);
    m_pSrc->addItem(&item3);
    m_pSrc->addItem(&item4);
    
    for (int i =0; i < items.size(); i++ ) {
        CRingItem* p = m_pSrc->getItem();
        ASSERT(p);
        ASSERT(*p == *items[i]);
        
        delete p;
    }
    ASSERT(!m_pSrc->getItem());

}

void testmultiple::sink_1()
{
    // If I don't put any thing in a sink is empty:
    
    ASSERT(m_pSink->m_sink.empty());
}
void testmultiple::sink_2()
{
    // If I put a single ring item a pointer to a copy of it will
    // be in the sink vector.
    
    CRingStateChangeItem item(BEGIN_RUN, 1, 0, time(nullptr), "The title string");
    m_pSink->putItem(item);
    
    EQ(size_t(1), m_pSink->m_sink.size());
    CRingItem* pSunk = m_pSink->m_sink[0];
    ASSERT(item == *pSunk);
}
void testmultiple::sink_3()
{
    // If I put several items copies of them will be in the sink in the right
    // order
    
    CDataFormatItem item1;
    CGlomParameters item2(100, true, CGlomParameters::first);
    CRingStateChangeItem item3(BEGIN_RUN, 1, 0, time(nullptr), "The title string");
    CRingStateChangeItem item4(END_RUN, 1, 100, time(nullptr), "The title string");
    std::vector<CRingItem*> items = {&item1, &item2, &item3, &item4};
    
    for (int i =0; i < items.size(); i++) {
        m_pSink->putItem(*items[i]);
    }
    
    EQ(items.size(), m_pSink->m_sink.size());
    for (int i =0; i < items.size(); i++) {
        ASSERT(*items[i] == *(m_pSink->m_sink[i]));
    }

}
void testmultiple::abend_1()
{
    // Running the filter with only an abnormal end in the source
    // results in an abnormal end in the sink.
 
    CMediator* pMed = m_pFilter->getMediator();
    CFilterTestSource* pSrc = dynamic_cast<CFilterTestSource*>(pMed->getDataSource());
    CFilterTestSink*   pSink = dynamic_cast<CFilterTestSink*>(pMed->getDataSink());
    
    /// Test that the testing flag made the right source/sink
    
    ASSERT(pSrc);
    ASSERT(pSink);
    
    CAbnormalEndItem abend;
    pSrc->addItem(&abend);
    
    (*m_pFilter)();
    
    EQ(size_t(1), pSink->m_sink.size());
    ASSERT(abend == *(pSink->m_sink[0]));
    
}

/**
 *  The next test and simple filter emits a format item prior to any
 *  begin run it produces.
 */
class CTestFilter1  : public CFilter
{
private:
    CFilterMain* m_pMain;
public:
    CTestFilter1(CFilterMain* pMain) : m_pMain(pMain) {}
    
    CRingItem* handleStateChangeItem(CRingStateChangeItem* pItem)
    {
        if (pItem->type() == BEGIN_RUN) {
            CDataFormatItem format;
            m_pMain->putRingItem(&format);
        }
        return pItem;
    }

    CRingItem* handleScalerItem(CRingScalerItem* pItem)
    {
        CRingPhysicsEventCountItem item(100, 10);
        m_pMain->putRingItem(&item);
        return pItem;
    }
    
    
    
    CFilter* clone() const {

        return new CTestFilter1(m_pMain);
    }
};

void testmultiple::emptyrun_1()
{
    CTestFilter1 filter(m_pFilter);
    m_pFilter->registerFilter(&filter);
    CRingStateChangeItem begin(BEGIN_RUN, 1, 0, time(nullptr), "The title string");
    CRingStateChangeItem end(END_RUN, 1, 100, time(nullptr), "The title string");
    CFilterTestSource* pSrc = dynamic_cast<CFilterTestSource*>(
        m_pFilter->getMediator()->getDataSource()
    );
    pSrc->addItem(&begin);
    pSrc->addItem(&end);
    
    (*m_pFilter)();
    
    // The sink should have a data format item begin and end.
    CFilterTestSink* pSink = dynamic_cast<CFilterTestSink*>(
        m_pFilter->getMediator()->getDataSink()
    );
    EQ(size_t(3), pSink->m_sink.size());
    
    CRingItem* fmt = pSink->m_sink[0];
    EQ(RING_FORMAT, fmt->type());
    
    ASSERT(begin == *(pSink->m_sink[1]));
    ASSERT(end == *(pSink->m_sink[2]));

}
void testmultiple::scaler_1()
{
    // Scaler item can emit physics event count too

    CTestFilter1 filter(m_pFilter);
    m_pFilter->registerFilter(&filter);
    CRingStateChangeItem begin(BEGIN_RUN, 1, 0, time(nullptr), "The title string");
    std::vector<uint32_t> scalers;
    for (int i =0; i <32; i++) {
        scalers.push_back(i);
    }
    CRingScalerItem scaler(0, 10, time(nullptr), scalers);
    CRingStateChangeItem end(END_RUN, 1, 100, time(nullptr), "The title string");
    
    CFilterTestSource* pSrc = dynamic_cast<CFilterTestSource*>(
        m_pFilter->getMediator()->getDataSource()
    );
    pSrc->addItem(&begin);
    pSrc->addItem(&scaler);
    pSrc->addItem(&end);
    
    (*m_pFilter)();
    
    // I shoulid have a format item, a begin run, an event count item,
    // a scaler item and an end run.
    
    CFilterTestSink* pSink = dynamic_cast<CFilterTestSink*>(
        m_pFilter->getMediator()->getDataSink()
    );
    EQ(size_t(5), pSink->m_sink.size());
    EQ(RING_FORMAT, pSink->m_sink[0]->type());
    EQ(BEGIN_RUN, pSink->m_sink[1]->type());
    EQ(PHYSICS_EVENT_COUNT, pSink->m_sink[2]->type());
    EQ(PERIODIC_SCALERS, pSink->m_sink[3]->type());
    EQ(END_RUN, pSink->m_sink[4]->type());
}
