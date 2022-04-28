
#ifndef CCBD8210READOUTLIST_H
#define CCBD8210READOUTLIST_H

#include <stdint.h>
#include "CCamacReadoutList.h"

class CVMUSBReadoutList;

class CCBD8210ReadoutList : public CCamacReadoutList 
{
    
    private:
    int m_branch;
    int m_crate;
    
    CVMUSBReadoutList* m_rdoList;
    bool m_isListOwner;

    public:
    CCBD8210ReadoutList();
    CCBD8210ReadoutList(int branch, int crate);

    void addRead16(int n, int a, int f);
    void addRead24(int n, int a, int f);

    void addWrite16(int n, int a, int f, uint16_t data);
    void addWrite24(int n, int a, int f, uint32_t data);

    void addControl(int n, int a, int f);

    void addQStop24(int n, int a, int f, int max_reps, bool lamwait=false); 

    public:
    int getCrateIndex() const {return m_crate;}
    int getBranchIndex() const {return m_branch;}

    void setCrateIndex(int c) { m_crate = c;}
    void setBranchIndex(int b) { m_branch = b;}

    // careful here because this remembers the last rdolist
    // if you return a func 
    CVMUSBReadoutList* getReadoutList() const { return m_rdoList;}

    // This does not own the readout list. It merely references it. 
    void setReadoutList(CVMUSBReadoutList& rdoList);

    void clear();
    size_t size() const;
};

#endif
