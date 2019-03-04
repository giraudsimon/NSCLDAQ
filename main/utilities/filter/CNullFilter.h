

#ifndef CNULLFILTER_H
#define CNULLFILTER_H

#include <CFilter.h>

/**! A filter whose handlers always return NULL.
*
* This is really intended for use in testing. 
*/
class CNullFilter : public CFilter {
  public :
    CNullFilter* clone() const { return new CNullFilter(*this); }

    CRingItem* handleScalerItem(CRingScalerItem*) {
      return 0;
    }

    CRingItem* handleTextItem(CRingTextItem*) {
      return 0;
    }

    CRingItem* handleFragmentItem(CRingFragmentItem*) {
      return 0;
    }
    CRingItem* handlePhysicsEventCountItem(CRingPhysicsEventCountItem*) {
      return 0;
    }

    CRingItem* handlePhysicsEventItem(CPhysicsEventItem*) {
      return 0;
    }
    CRingItem* handleStateChangeItem(CRingStateChangeItem*) {
      return 0;
    }

    CRingItem* handleRingItem(CRingItem*) {
      return 0;
    }

};


#endif
