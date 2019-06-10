
#ifndef CTESTFILTER_H
#define CTESTFILTER_H

#include <CFilter.h>

class CRingItem;


class CTestFilter : public CFilter {
  private:
    int m_nProcessed;

  public:
    bool m_initCalled;
    bool m_finalCalled;

  public:
    CTestFilter();
    CTestFilter* clone() const { return new CTestFilter(*this);}

    virtual CRingItem* handleStateChangeItem(CRingStateChangeItem*);

    virtual CRingItem* handleScalerItem(CRingScalerItem* );

    virtual CRingItem* handleTextItem(CRingTextItem*);

    virtual CRingItem* handlePhysicsEventItem(CPhysicsEventItem* );

    virtual CRingItem*
      handlePhysicsEventCountItem(CRingPhysicsEventCountItem*);

    virtual CRingItem* handleFragmentItem(CRingFragmentItem*);

    virtual CRingItem* handleRingItem(CRingItem*);

    int getNProcessed() const { return m_nProcessed;}

    void initialize() { m_initCalled = true; }
    void finalize() { m_finalCalled = true; }
};

#endif
