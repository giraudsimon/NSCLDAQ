
#ifndef CCNAF_H
#define CCNAF_H

#include "CNAF.h"

class CCNAF {
    private:
        int m_c;
        CNAF m_naf;

    private:
        CCNAF();

    public:
        CCNAF(int c, int n, int a, int f);
        CCNAF(int c, const CNAF& naf);

        CCNAF(const CCNAF& rhs);

        CCNAF& operator=(const CCNAF& rhs);

        int c() const { return m_c;}
        int n() const { return m_naf.n();}
        int a() const { return m_naf.a();}
        int f() const { return m_naf.f();}

        CNAF naf() const { return m_naf;}
};


#endif
