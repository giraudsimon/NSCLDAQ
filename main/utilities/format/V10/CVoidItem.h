
#ifndef NSCLDAQ10_CVOIDITEM_H
#define NSCLDAQ10_CVOIDITEM_H

namespace DAQ {
namespace V10 {

class CVoidItem : public CRingItem
{
  public:
    CVoidItem() : CRingItem(0) {}

    uint32_t type() const { return 0; }
};

} // end V10
} // end DAQ

#endif
