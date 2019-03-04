

#ifndef CCAMACBRANCHDRIVER_H
#define CCAMACBRANCHDRIVER_H

#include <stdint.h>
class CCrateController;
class CCamacReadoutList;

// I want to get rid of the next line but we are stick with it for the moment

class CCamacBranchDriver 
{
    public:
        // maybe these should be templated instead of pure virtual
        virtual CCrateController* createCrateController(int branch, int crate) = 0;
        virtual CCamacReadoutList* createReadoutList(int branch, int crate) = 0;

        virtual void initializeBranch(CCrateController& ctlr, int b) =0;
        virtual void initializeCrate(CCrateController& ctlr, int b, int c) =0;

        virtual void addRead16(CCamacReadoutList& list, int b, int c, 
                int n, int a, int f) = 0;

        virtual void addRead24(CCamacReadoutList& list, int b, int c, 
                int n, int a, int f) = 0;

        virtual void addWrite16(CCamacReadoutList& list, int b, int c, 
                int n, int a, int f, uint16_t data) = 0;

        virtual void addWrite24(CCamacReadoutList& list, int b, int c, 
                int n, int a, int f, uint32_t data) = 0; 
};


#endif
