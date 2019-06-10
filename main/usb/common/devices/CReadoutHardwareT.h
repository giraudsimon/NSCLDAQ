
#ifndef CREADOUTHARDWARET_H
#define CREADOUTHARDWARET_H

#include <CReadoutModule.h>

template <class Controller, class RdoList>
class CReadoutHardwareT
{
    public:
        virtual void onAttach(CReadoutModule& config)=0;
        virtual void Initialize(Controller& controller)=0;
        virtual void addReadoutList(RdoList& list)=0;
        virtual void onEndRun(Controller& list)=0;
};

#endif
