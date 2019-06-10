
#include <stdint.h>
#include <iostream>
#include "CConfiguration.h"
#include "CamacSlotLimits.h"

template<class Controller, class RdoList>
CLeCroy2551<Controller,RdoList>::CLeCroy2551() 
  :
    m_pConfig(0)
{
}


template<class Controller, class RdoList>
CLeCroy2551<Controller,RdoList>::CLeCroy2551(const CLeCroy2551& rhs)
    : CReadoutHardwareT<Controller,RdoList>(rhs)
{
//    if (rhs.m_pConfig) {
//        m_pConfig = new CReadoutModule(*(rhs.m_pConfig));
//    }

}

template<class Controller, class RdoList>
CLeCroy2551<Controller,RdoList>& 
CLeCroy2551<Controller,RdoList>::operator=(const CLeCroy2551& rhs)
{
    return *this;
}

template<class Controller, class RdoList>
CLeCroy2551<Controller,RdoList>::~CLeCroy2551()
{
//    if(m_pConfig) delete m_pConfig;
}


template<class Controller, class RdoList>
void  CLeCroy2551<Controller,RdoList>::onAttach(CReadoutModule& config)
{
//    std::cout << "Attaching CLeCroy2551" << std::endl;
    m_pConfig = &config;

    m_pConfig->addParameter("-slot",
                            XXUSB::CConfigurableObject::isInteger,
                            &SlotLimits,
                            "1");
}

template<class Controller, class RdoList>
void CLeCroy2551<Controller,RdoList>::Initialize(Controller& controller)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 

    uint16_t qx=0;
    controller.simpleControl(slot,0,9,qx);

//    std::cout << "qx = " << qx << std::endl;
    if (qx==0) {
        throw "CLeCroy2551::Initialize(Controller&) Q==0 after simpleWrite16";
    }
}

template<class Controller, class RdoList>
void CLeCroy2551<Controller,RdoList>::addReadoutList(RdoList& list)
{
    addReadAll(list);
// I could not get this to work properly so I am disabling it.
//    addClearAll(list);
}

////////////////
template<class Controller, class RdoList>
void CLeCroy2551<Controller,RdoList>::addClearAll(RdoList& list)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    list.addControl(slot,0,9);
}
//////////////

template<class Controller, class RdoList>
void CLeCroy2551<Controller,RdoList>::addRead(RdoList& list, int chan)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    if (chan>11 || chan<0) {
        throw CInvalidA(chan);
    }
    list.addRead24(slot,chan,0);
}

template<class Controller, class RdoList>
void CLeCroy2551<Controller,RdoList>::addReadAll(RdoList& list)
{
    for (int chan = 0; chan<12; ++chan) {
        addRead(list,chan);
    }     
}

//extern template class CLeCroy2551<CCBD8210CrateController,CCBD8210ReadoutList>;
//template class CLecroy2551<CCCUSB,CCCUSBReadoutList> CamacLeCroy2551;

