
#ifndef CNAF_H
#define CNAF_H

class CNAF {
    private:
        int m_n;
        int m_a;
        int m_f;

    private:
        CNAF();

    public:
        CNAF(int n, int a, int f);

        CNAF(const CNAF& rhs);

        CNAF& operator=(const CNAF& rhs);

        int n() const { return m_n;}
        int a() const { return m_a;}
        int f() const { return m_f;}
};


#endif
