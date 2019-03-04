
#include "CSourceCounterFilter.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <FragmentIndex.h>

using namespace std;

  CSourceCounterFilter::CSourceCounterFilter(uint32_t defaultId, std::string outputFile)
: m_counters(), m_defaultId(defaultId), m_outputFile(outputFile), m_builtData(true)
{
  setupCounters(m_defaultId);
}

CSourceCounterFilter::~CSourceCounterFilter() 
{
}

// The default handlers
CRingItem* CSourceCounterFilter::handleRingItem(CRingItem* pItem) 
{
  incrementCounter(pItem);
  return pItem;
}

CRingItem* CSourceCounterFilter::handleStateChangeItem(CRingStateChangeItem* pItem) 
{
  incrementCounter(pItem);
  return static_cast<CRingItem*>(pItem);
}

CRingItem* CSourceCounterFilter::handleScalerItem(CRingScalerItem* pItem) 
{
  incrementCounter(pItem);
  return static_cast<CRingItem*>(pItem);
}

CRingItem* CSourceCounterFilter::handleTextItem(CRingTextItem* pItem) 
{
  incrementCounter(pItem);
  return static_cast<CRingItem*>(pItem);
}

CRingItem* CSourceCounterFilter::handlePhysicsEventItem(CPhysicsEventItem* pItem) 
{
  if (m_builtData) {
    uint16_t* pBody = reinterpret_cast<uint16_t*>(pItem->getBodyPointer());
    FragmentIndex index(pBody);
    auto iter = index.begin();
    auto iter_end = index.end();
    while (iter != iter_end) {
      std::unique_ptr<CRingItem> pSubItem(CRingItemFactory::createRingItem(iter->s_itemhdr));
      incrementCounter(iter->s_sourceId, pSubItem->type());
      ++iter;
    }
  } else {
    incrementCounter(pItem);
  }
  return static_cast<CRingItem*>(pItem);
}

  CRingItem* 
CSourceCounterFilter::handlePhysicsEventCountItem(CRingPhysicsEventCountItem* pItem) 
{
  incrementCounter(pItem);
  return static_cast<CRingItem*>(pItem);
}

CRingItem* CSourceCounterFilter::handleFragmentItem(CRingFragmentItem* pItem)
{
  incrementCounter(pItem);
  return static_cast<CRingItem*>(pItem);
}


bool CSourceCounterFilter::counterExists(uint32_t type) 
{
  map<uint32_t,std::map<uint32_t,uint32_t> >::iterator it;
  it = m_counters.find(type);
  return ( it!=m_counters.end() );

}

void CSourceCounterFilter::printCounters(std::ostream& stream) const
{
  map<uint32_t,map<uint32_t,uint32_t> >::const_iterator it,itend;
  map<uint32_t,uint32_t>::const_iterator idit,iditend;
  it = m_counters.begin();
  itend = m_counters.end();

  stream << "set sourceMap {";
  while (it != itend) {
    idit = it->second.begin();
    iditend = it->second.end();
    stream << it->first << " {";
    while (idit!=iditend) {
      stream << translate(idit->first) 
           << " " << idit->second 
           << " ";
      ++idit;
    }
    stream << "} ";
    ++it;
  }
  stream << "}";
}


string CSourceCounterFilter::translate(uint32_t type) const
{
  map<uint32_t,string> namemap;
  namemap[BEGIN_RUN]            = "BEGIN_RUN";
  namemap[END_RUN]              = "END_RUN";
  namemap[PAUSE_RUN]            = "PAUSE_RUN";
  namemap[RESUME_RUN]           = "RESUME_RUN";
  namemap[PACKET_TYPES]         = "PACKET_TYPES";
  namemap[MONITORED_VARIABLES]  = "MONITORED_VARIABLES";
  namemap[RING_FORMAT]          = "RING_FORMAT";
  namemap[PERIODIC_SCALERS]     = "PERIODIC_SCALERS";
  namemap[PHYSICS_EVENT]        = "PHYSICS_EVENT";
  namemap[PHYSICS_EVENT_COUNT]  = "PHYSICS_EVENT_COUNT";
  namemap[EVB_FRAGMENT]         = "EVB_FRAGMENT";
  namemap[EVB_UNKNOWN_PAYLOAD]  = "EVB_UNKNOWN_PAYLOAD";
  namemap[EVB_GLOM_INFO]        = "EVB_GLOM_INFO";

  map<uint32_t,string>::const_iterator it;
  it = namemap.find(type);
  if ( it!=namemap.end() ) {
     return it->second;
  } else {
    stringstream name;
    name << "User type #" << type;
    return name.str();
  }


}

void CSourceCounterFilter::finalize() 
{
  std::ofstream dump_file(m_outputFile.c_str());
  printCounters(dump_file);
}


void CSourceCounterFilter::incrementCounter(CRingItem* pItem) 
{

  if ( pItem->hasBodyHeader() ) {
    incrementCounter(pItem->getSourceId(), pItem->type());
  } else {
    // this is setup in the constructor
    incrementCounter(m_defaultId, pItem->type());
  }

}

void CSourceCounterFilter::incrementCounter(uint32_t id, uint32_t type) 
{
    if (!counterExists(id)) {
      setupCounters(id);
    }
    m_counters[id][type] += 1;
}

void CSourceCounterFilter::setupCounters(uint32_t id) 
{
  m_counters[id][BEGIN_RUN]           = 0;
  m_counters[id][END_RUN]             = 0;
  m_counters[id][PAUSE_RUN]           = 0;
  m_counters[id][RESUME_RUN]          = 0;

  m_counters[id][PACKET_TYPES]        = 0;
  m_counters[id][MONITORED_VARIABLES] = 0;
  m_counters[id][RING_FORMAT]         = 0;

  m_counters[id][PERIODIC_SCALERS]    = 0;

  m_counters[id][PHYSICS_EVENT]       = 0;
  m_counters[id][PHYSICS_EVENT_COUNT] = 0;

  m_counters[id][EVB_FRAGMENT]        = 0;
  m_counters[id][EVB_UNKNOWN_PAYLOAD] = 0;
  m_counters[id][EVB_GLOM_INFO]       = 0;
}
