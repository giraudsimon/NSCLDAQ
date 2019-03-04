
#include <CTestFilter.h>
#include <vector>
#include <string>

using namespace std;

CTestFilter::CTestFilter ()
  : CFilter(), m_nProcessed(0), m_initCalled(false), m_finalCalled(false) 
{}

CRingItem* CTestFilter::handleStateChangeItem(CRingStateChangeItem*) 
{
  ++m_nProcessed; 
  return new CRingStateChangeItem(BEGIN_RUN);
}

CRingItem* CTestFilter::handleScalerItem(CRingScalerItem* ) 
{ 
  ++m_nProcessed; 
  return new CRingScalerItem(200);
}

CRingItem* 
CTestFilter::handleTextItem(CRingTextItem*) 
{ 
  ++m_nProcessed; 
  std::vector<string> str_vec;
  str_vec.push_back("0000");
  str_vec.push_back("1111");
  str_vec.push_back("2222");
  return new CRingTextItem(PACKET_TYPES,str_vec);
}

CRingItem* 
CTestFilter::handlePhysicsEventItem(CPhysicsEventItem* ) 
{ 
  ++m_nProcessed; 
  return new CPhysicsEventItem(4096);
}

CRingItem*
CTestFilter::handlePhysicsEventCountItem(CRingPhysicsEventCountItem*) 
{ 
  ++m_nProcessed; 
  return new CRingPhysicsEventCountItem(static_cast<uint64_t>(4),
                                        static_cast<uint32_t>(1001));
}

CRingItem* CTestFilter::handleFragmentItem(CRingFragmentItem*)
{
  ++m_nProcessed; 
  return new CRingFragmentItem(static_cast<uint64_t>(10101),
                                static_cast<uint32_t>(1),
                                static_cast<uint32_t>(2),
                                reinterpret_cast<void*>(new char[2]),
                                static_cast<uint32_t>(3));
}

CRingItem* CTestFilter::handleRingItem(CRingItem*) 
{ 
  ++m_nProcessed; 
  return new CRingItem(100);
}

