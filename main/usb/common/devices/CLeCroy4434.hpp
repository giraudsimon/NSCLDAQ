
#include <stdint.h>
#include <iostream>
#include "CConfiguration.h"
#include "CamacSlotLimits.h"

// Defines some useful constants for the command register
static const uint16_t ECLSCL_TEST (0x8000);
static const uint16_t ECLSCL_BD   (0x2000);
static const uint16_t ECLSCL_RD   (0x0080);
static const uint16_t ECLSCL_CL   (0x0040);
static const uint16_t ECLSCL_LD   (0x0020);

static const uint16_t ECLSCL_NPOS (8); ///!< offset for channel specification

template<class Controller, class RdoList>
CLeCroy4434<Controller,RdoList>::CLeCroy4434() 
  :
    m_pConfig(0)
{
}


template<class Controller, class RdoList>
CLeCroy4434<Controller,RdoList>::CLeCroy4434(const CLeCroy4434& rhs)
    : CReadoutHardwareT<Controller,RdoList>(rhs)
{
//    if (rhs.m_pConfig) {
//        m_pConfig = new CReadoutModule(*(rhs.m_pConfig));
//    }

}

// Dont do anything.
template<class Controller, class RdoList>
CLeCroy4434<Controller,RdoList>& 
CLeCroy4434<Controller,RdoList>::operator=(const CLeCroy4434& rhs)
{
    return *this;
}

// do nothing b/c the m_pconfig should own this object. Deleting the
// parent will cause recursive deletes to be called. This would be bad!
template<class Controller, class RdoList>
CLeCroy4434<Controller,RdoList>::~CLeCroy4434()
{
//    if(m_pConfig) delete m_pConfig;
}


// store the location of the parent and add the parameter
template<class Controller, class RdoList>
void  CLeCroy4434<Controller,RdoList>::onAttach(CReadoutModule& config)
{
//    std::cout << "Attaching CLeCroy4434" << std::endl;
    m_pConfig = &config;
//    m_pConfig->addIntegerParameter("-slot",0);

    m_pConfig->addParameter("-slot",
                            XXUSB::CConfigurableObject::isInteger,
                            &SlotLimits,
                            "1");

  m_pConfig->addBooleanParameter("-incremental",false);
}

template<class Controller, class RdoList>
void CLeCroy4434<Controller,RdoList>::Initialize(Controller& controller)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
  
    uint16_t qx=0;
    // clear module & disable aux bus the module 
    // by writing to the command register
    controller.simpleWrite16(slot,0,16,ECLSCL_CL | ECLSCL_BD, qx);
    if ((qx&1)==0) {
        throw "CLeCroy4434::Initialize(Controller&) Q==0 after simpleWrite16";
    }
}

template<class Controller, class RdoList>
void CLeCroy4434<Controller,RdoList>::addReadoutList(RdoList& list)
{
    addReadAll(list);

    if (m_pConfig->getBoolParameter("-incremental")) {
        addClear(list);
    }
}


template<class Controller, class RdoList>
void CLeCroy4434<Controller,RdoList>::addClear(RdoList& list)
{
    // clear module and disable aux bus by writing to command reg
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    list.addWrite16(slot,0,16,ECLSCL_CL | ECLSCL_BD);
}

template<class Controller, class RdoList>
void CLeCroy4434<Controller,RdoList>::addReadAll(RdoList& list)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 
    uint16_t chanToStart = 31 << ECLSCL_NPOS;

    // Load the values into memory for readout and set 
    // chan 31 as first to read
//    list.addRead16(0,7,0); // 100 ns
    list.addWrite16(slot,0,16, ECLSCL_LD | chanToStart );

    // set delay in between repeaed commands
    // Read channels. The module decrements the channel being read
    // after each read (31 -> 30 -> 29 -> ... -> 0)
//    for (int chan=31; chan!=-1; --chan) {
//        list.addRead16(0,7,0); // 100 ns
//        list.addRead24(slot,0,2);
//    }     

      list.addRead16(0,0,0); // 100 ns
      list.addRead16(0,0,0); // 100 ns
      list.addRead16(0,0,0); // 100 ns
      list.addRead16(0,0,0); // 100 ns
      list.addRead16(0,0,0); // 100 ns
      list.addRead16(0,0,0); // 100 ns
      list.addRead16(0,7,0); // 700 ns
      list.addQStop24(slot,0,2,32,false);
}

template<class Controller, class RdoList>
void CLeCroy4434<Controller,RdoList>::addRead(RdoList& list, int a)
{
    int slot = m_pConfig->getIntegerParameter("-slot"); 

    // Load the values into memory for readout and set 
    // chan a as first to read
    list.addWrite16(slot,0,16, ECLSCL_LD | (a << ECLSCL_NPOS) );
    // read a single channel
    list.addRead24(slot,0,0);
}

