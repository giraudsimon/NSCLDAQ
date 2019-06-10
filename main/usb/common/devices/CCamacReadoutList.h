
#ifndef CCAMACREADOUTLIST_H
#define CCAMACREADOUTLIST_H

#include <stdint.h>
class CCamacReadoutList
{
    public:
        virtual void addRead16(int n, int a, int f)=0;
        virtual void addRead24(int n, int a, int f)=0;

        virtual void addWrite16(int n, int a, int f, uint16_t data)=0;
        virtual void addWrite24(int n, int a, int f, uint32_t data)=0;

        virtual void addControl(int n, int a, int f)=0;

        virtual void clear()=0;
};

#endif
