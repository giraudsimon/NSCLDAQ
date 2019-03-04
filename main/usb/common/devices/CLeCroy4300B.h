

#ifndef CLECROY4300B_H
#define CLECROY4300B_H

#include <stdint.h> 
#include "CReadoutHardwareT.h"

class CReadoutModule;
class CCBD8210CrateController; 
class CCBD8210ReadoutList; 

template<class Controller, class RdoList> class CLeCroy4300B;

template<class Controller, class RdoList>
class CLeCroy4300B : public CReadoutHardwareT<Controller, RdoList>
{

    private:
    CReadoutModule* m_pConfig;

    public:
    CLeCroy4300B();
    CLeCroy4300B(const CLeCroy4300B& rhs);
    CLeCroy4300B& operator=(const CLeCroy4300B& rhs);
    virtual ~CLeCroy4300B();


    void onAttach(CReadoutModule& config);
    void Initialize(Controller& config);
    void addReadoutList(RdoList& config);
  
    // no op
    void onEndRun(Controller& ) {}

    CLeCroy4300B* clone() const {
        return new CLeCroy4300B(*this);
    }


    private:
    void writePedestals(Controller& controller, const std::vector<int>& peds);
    void execClear(Controller& controller);
    void setCommandRegister(Controller& controller, uint16_t cmdreg);
    void configureCommandRegister(Controller& controller);
    void addClear(RdoList& list);
    void handleStatus(int status);
    void handleQ(uint16_t q);

};

#include "CLeCroy4300B.hpp"

#endif
