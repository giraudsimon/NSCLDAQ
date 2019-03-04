
#include "RingItemComparisons.h"

#include <algorithm>

/**! Comparison of _RingItem objects
  *
  */
bool operator==(const _RingItem& lhs, const _RingItem& rhs)
{
    size_t size0 = lhs.s_header.s_size;
    size_t size1 = rhs.s_header.s_size;
    
    if (size0 != size1) {
        return false;
    }

    const char* iter0 = reinterpret_cast<const char*>(&lhs);
    const char* iter1 = reinterpret_cast<const char*>(&rhs);

    return std::equal(iter0, iter0+size0, iter1);
    
}

