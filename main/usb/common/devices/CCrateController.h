
#ifndef CCRATECONTROLLER_H
#define CCRATECONTROLLER_H

#include <stdint.h>
#include <stddef.h>
#include "CNAF.h"

class CCamacReadoutList;

class CCrateController 
{
    public:
    virtual int simpleWrite16(int n, int a, int f, uint16_t data, uint16_t& qx)=0;
    virtual int simpleWrite24(int n, int a, int f, uint32_t data, uint16_t& qx)=0;
    virtual int simpleRead16( int n, int a, int f, uint16_t& data, uint16_t& qx)=0;
    virtual int simpleRead24( int n, int a, int f, uint32_t& data, uint16_t& qx)=0;
    virtual int simpleControl(int n, int a, int f, uint16_t& qx)=0;

    int write16(const CNAF& cnaf, uint16_t data, uint16_t& qx);
    int write24(const CNAF& cnaf, uint32_t data, uint16_t& qx);
    int read16(const CNAF& cnaf, uint16_t& data, uint16_t& qx);
    int read24(const CNAF& cnaf, uint32_t& data, uint16_t& qx);
    int control(const CNAF& cnaf, uint16_t& qx);

    virtual CCamacReadoutList* createReadoutList() const =0;
    virtual int executeList(CCamacReadoutList& list, void* buffer, 
                                    size_t bufsize, size_t* bytes)=0;
}; 

#endif
