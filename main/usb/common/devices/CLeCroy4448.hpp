
#include <stdint.h>
#include <iostream>
#include <sstream>
#include "CConfiguration.h"
#include "CamacSlotLimits.h"


template<class Controller, class RdoList>
CLeCroy4448<Controller,RdoList>::CLeCroy4448() 
  :
    m_pConfig(0)
{
}


template<class Controller, class RdoList>
CLeCroy4448<Controller,RdoList>::CLeCroy4448(const CLeCroy4448& rhs)
    : CReadoutHardwareT<Controller,RdoList>(rhs)
{
//    if (rhs.m_pConfig) {
//        m_pConfig = new CReadoutModule(*(rhs.m_pConfig));
//    }

}

// Dont do anything.
template<class Controller, class RdoList>
CLeCroy4448<Controller,RdoList>& 
CLeCroy4448<Controller,RdoList>::operator=(const CLeCroy4448& rhs)
{
    return *this;
}

// do nothing b/c the m_pconfig should own this object. Deleting the
// parent will cause recursive deletes to be called. This would be bad!
template<class Controller, class RdoList>
CLeCroy4448<Controller,RdoList>::~CLeCroy4448()
{
//    if(m_pConfig) delete m_pConfig;
}


// store the location of the parent and add the parameter
template<class Controller, class RdoList>
void  CLeCroy4448<Controller,RdoList>::onAttach(CReadoutModule& config)
{
    m_pConfig = &config;

    m_pConfig->addParameter("-slot",
                            XXUSB::CConfigurableObject::isInteger,
                            &SlotLimits,
                            "1");

}

template<class Controller, class RdoList>
void CLeCroy4448<Controller,RdoList>::Initialize(Controller& controller)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
  
    // clear module & disable aux bus the module 
    // by writing to the command register
    Clear(controller);
}

template<class Controller, class RdoList>
void CLeCroy4448<Controller,RdoList>::addReadoutList(RdoList& list)
{
}



////////////////////////////////////////////////////////////////////////////
//
// User functions for manipulating the module and stack
//


template<class Controller, class RdoList>
void CLeCroy4448<Controller,RdoList>::Clear(Controller& controller)
{
    // clear module and disable aux bus by writing to command reg
  uint16_t qx=0;

  int slot = m_pConfig->getIntegerParameter("-slot"); 

  controller.simpleControl(slot,0,11,qx);
  if ((qx&1)==0) {
    throw "CLeCroy4448::Clear(Controller&) Q==0 after simpleWrite16";
  }
}



//
//
template<class Controller, class RdoList>
void CLeCroy4448<Controller,RdoList>::sClear(RdoList& list)
{
    // clear module and disable aux bus by writing to command reg
   uint16_t qx=0;
  int slot = m_pConfig->getIntegerParameter("-slot"); 
  list.addControl(slot,0,11);
}



//
//
template<class Controller, class RdoList>
void CLeCroy4448<Controller,RdoList>::sRead(RdoList& list, int a)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 

    if (a!=0 || a!=1 || a!=2) {
      std::ostringstream errmsg; 
      errmsg << "CLeCroy4448::sRead(RdoList&,int) invalid argument. ";
      errmsg << "Register argument must be either 0, 1, or 2 and user provided ";
      errmsg << a << ".";
      throw errmsg.str();
    }
    list.addRead16(slot, a, 0);
}

