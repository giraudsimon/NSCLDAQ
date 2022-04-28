#include "CNAF.h"


CNAF::CNAF(int n, int a, int f)
  : m_n(n), m_a(a), m_f(f)
{}

CNAF::CNAF(const CNAF& rhs)
: m_n(rhs.m_n),
    m_a(rhs.m_a),
    m_f(rhs.m_f)
{}

CNAF& CNAF::operator=(const CNAF& rhs)
{
    if (this != & rhs) {
        m_n = rhs.m_n;
        m_a = rhs.m_a;
        m_f = rhs.m_f;
    }
    return *this;
}
