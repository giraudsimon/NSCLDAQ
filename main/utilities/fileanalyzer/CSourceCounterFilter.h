
#ifndef CSOURCECOUNTERFILTER_H
#define CSOURCECOUNTERFILTER_H

#include <CFilter.h>
#include <DataFormat.h>
#include <CRingItem.h>
#include <CRingStateChangeItem.h>
#include <CPhysicsEventItem.h>
#include <CRingTextItem.h>
#include <CRingItemFactory.h>
#include <CRingPhysicsEventCountItem.h>
#include <CRingFragmentItem.h>
#include <FragmentIndex.h>

#include <stdint.h>
#include <map>
#include <string>
#include <iosfwd>
#include <algorithm>
#include <memory>

class CSourceCounterFilter : public CFilter
{
  private:
    std::map<uint32_t, std::map<uint32_t, uint32_t> > m_counters;
    uint32_t     m_defaultId;
    std::string  m_outputFile;
    bool         m_builtData;

  public:

    CSourceCounterFilter(uint32_t defaultId, std::string outputFile);
    virtual ~CSourceCounterFilter();

    CSourceCounterFilter* clone() const { return new CSourceCounterFilter(*this);}

    void setBuiltData(bool val) { m_builtData = val;}

    // The default handlers
    virtual CRingItem* handleRingItem(CRingItem* pItem);
    virtual CRingItem* handleStateChangeItem(CRingStateChangeItem* pItem);
    virtual CRingItem* handleScalerItem(CRingScalerItem* pItem); 
    virtual CRingItem* handleTextItem(CRingTextItem* pItem); 
    virtual CRingItem* handlePhysicsEventItem(CPhysicsEventItem* pItem);
    virtual CRingItem* 
      handlePhysicsEventCountItem(CRingPhysicsEventCountItem* pItem);
    virtual CRingItem* handleFragmentItem(CRingFragmentItem* pItem);

    virtual void finalize();

  private:
    bool counterExists(uint32_t type);
    void setupCounters(uint32_t id);
    void incrementCounter(CRingItem* pItem);
    void incrementCounter(uint32_t id, uint32_t type);

    void printCounters(std::ostream& stream) const;
    std::string translate(uint32_t type) const;
  

};

#endif


