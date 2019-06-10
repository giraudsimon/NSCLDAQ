

#ifndef STATECHANGEHANDLER_H
#define STATECHANGEHANDLER_H

#include <stdint.h>
#include <map>

class CRingItem;
class CRingStateChangeItem;

class COneShotHandler 
{
  private:
  unsigned int m_nExpectedSources;
  std::map<uint32_t,uint32_t> m_stateCounts;
  uint32_t m_cachedRunNo;
  bool     m_complete;

  public:
    COneShotHandler(unsigned int ntrans);

    void initialize(CRingStateChangeItem* item);

    void update(CRingItem* item);

    bool waitingForBegin() const;
    bool complete() const;

    void reset();

    uint32_t getCount(uint32_t key) const;

  private:
    bool validType(uint32_t type) const;
    void clearCounts();
    
    void updateState(CRingStateChangeItem* item);
};
#endif
