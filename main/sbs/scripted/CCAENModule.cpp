/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#include <config.h>

#include "CCAENModule.h"
#include "CAENcard.h"
#include "TCLInterpreter.h"
#include "CIntConfigParam.h"
#include "CBoolConfigParam.h"
#include "CIntArrayParam.h"
#include <assert.h>
#include <iostream>

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


// static const int CHANNELS(32);
static const int WAITLOOPS(20);
/*!
  Constructs a CAEN module object.  The constructor
creates the common set of configuration options that
are recognized by all types of modules.

\param rName (const string& [in]):
   Command name that will be associated with this module.
   note that CCAENModules never get built by themselves,
   only their concrete model specific sub-clases.

\param rInterpeter (CTCLInterpreter& [in]):
   The interpreter on which this object's manipulating
   command will be registered.
\param nChannels 
    Number of channels this module supports (this is to deal with
    some of the stranger dual range modules that CAEN is recently 
    putting out.
*/
CCAENModule::CCAENModule(const std::string & rName,
			CTCLInterpreter& rInterpreter, int nChannels) :
  CDigitizerModule(rName, rInterpreter),
  m_pCAENcard(0),
  m_fMultiEvent(false),
  m_nChannels(nChannels)
{
  // Setup our configuration parameters:
  
  AddIntParam(std::string("crate"), 0);
  AddIntParam(std::string("slot"), -1);   // Force legal config.
  AddIntArrayParam(std::string("threshold"), m_nChannels, 0);
  AddBoolParam(std::string("keepunder"), false);
  AddBoolParam(std::string("keepoverflow"), false);
  AddBoolParam(std::string("card"), true);
  AddBoolParam(std::string("geo"),  true);    // Geographical addressing.
  AddIntArrayParam(std::string("enable"), m_nChannels, 1);
  AddIntParam(std::string("base"), 0);
  AddBoolParam(std::string("multievent"), false);
  AddIntParam(std::string("fastclearwindow"), 0);
  AddIntParam(std::string("waitloops"), WAITLOOPS);

  // This parameter is added to allow us to source in 
  // SpecTcl configuration scripts too!!

  AddStringArrayParam(std::string("parameters"), 32);
  
  // Now set the appropriate ranges for the integer parameters.
  // This is a two step process for each parameter (or array).
  // - use Find to locate the parameter and 
  // - Invoke the found object's setRange member.
  // 
  // Since we have just entered these objects in the 
  // parser's configuration list, failing to find them
  // is considered a fatal error signalled via a failed
  // assert.
  
  ParameterIterator i;
  CIntConfigParam*  pInt;
  CIntArrayParam*   pArray;

  // waitloops in the range 1-1000 (that's big, 1000, I really
  // want to make sure no joker sets it to 0 or negative.
  //

  i = Find("waitloops");
  assert(i != end());
  pInt = (CIntConfigParam*)*i;
  pInt->setRange(1, 1000);

  // Crate numbers are in the range [0-7]:

  i = Find(std::string("crate"));
  assert(i != end());
  pInt = (CIntConfigParam*)*i;
  pInt->setRange(0,7);

  // Slot numbers range from 2 to 21 since
  // since slot 1 is a system controller.
  
  i    = Find(std::string("slot"));
  assert(i != end());
  pInt = (CIntConfigParam*)*i;
  pInt->setRange(1, 21);
   
  // Thresholds have the range 0 - 4000mV.
  
  i      = Find(std::string("threshold"));
  assert(i != end());
  pArray = (CIntArrayParam*)*i;
  pArray->setRange(0, 4000);
  
  // Ranges have values 0 or 1 (really flags).
  
  i      = Find(std::string("enable"));
  assert(i != end());
  pArray = (CIntArrayParam*)*i;
  pArray->setRange(0,1);

  // Fast clear window runs from 0, 2000000:

  i = Find(std::string("fastclearwindow"));
  assert(i != end());
  pInt = (CIntConfigParam*)*i;
  pInt->setRange(0, 2000000);
  
}
/*!
   Initializes a module.  This is the function that will actually
   create the underlying module driver object.   The parameters
   are processed and used to configure it.
*/
void
CCAENModule::Initialize()
{
  delete m_pCAENcard;    // Kill off the old card.
  
  // Figure out the card slot so that we can construct
  // the new card.
  
  ParameterIterator     i;
  CIntConfigParam*      pInt;
  CIntArrayParam*       pArray;
  CBoolConfigParam* pBool;
  
  i = Find("crate");
  assert(i != end());
  pInt = (CIntConfigParam*)*i;
  int nCrate = pInt->getOptionValue();


  bool fGeo;

  i   = Find("slot");
  assert(i != end());
  pInt= (CIntConfigParam*)*i;
  int nSlot = pInt->getOptionValue();
  
  // Could be set via base address with slot programmed:
  
  i = Find("base");
  assert(i != end());
  pInt = (CIntConfigParam*)*i;
  int nBase = pInt->getOptionValue();

  // Figure out which to use for the config:

  i = Find("geo");
  assert(i != end());
  pBool = (CBoolConfigParam*)*i;
  fGeo   = pBool->getOptionValue();

  m_pCAENcard = new CAENcard(nSlot, nCrate, fGeo, nBase);
  
  // Now we can configure the new card according to the
  // remaining parameters:
  
  // Set the thresholds: 1mv == 1 chan so no conversion
  // is required;
  
    i   = Find("threshold");
    assert(i != end());
    pArray = (CIntArrayParam*)*i;
    for(int c = 0; c < m_nChannels; c++) {
      m_pCAENcard->setThreshold(c, (*pArray)[c]);
    }
  
  // Determine which parameters will be excluded.
  
    i   = Find("keepunder");
    assert(i != end());
    pBool= (CBoolConfigParam*)*i;
    if(pBool->getOptionValue()) {
      m_pCAENcard->keepUnderThresholdData();
    }
    else {
      m_pCAENcard->discardUnderThresholdData();
    }
    i   = Find("keepoverflow");
    assert(i != end());
    pBool = (CBoolConfigParam*)*i;
    if(pBool->getOptionValue()) {
      m_pCAENcard->keepOverflowData();
    }
    else {
      m_pCAENcard->discardOverflowData();
    }

  // Figure out if the card is enabled, and which
  // individual channels are enabled.
  
    i = Find("card");
    assert(i != end());
    pBool = (CBoolConfigParam*)*i;
    if(pBool->getOptionValue()) {
      m_pCAENcard->cardOn();
    }
    else {
      m_pCAENcard->cardOff();
    }
    i = Find("enable");
    assert(i != end());
    pArray = (CIntArrayParam*)*i;
    for(int c = 0; c < m_nChannels; c++) {
      if((*pArray)[c]) {
        m_pCAENcard->channelOn(c);
      }
      else {
        m_pCAENcard->channelOff(c);
      }
    }
    // Multievent mode?

    i = Find("multievent");
    assert(i != end());
    pBool = (CBoolConfigParam*)*i;
    m_fMultiEvent = pBool->getOptionValue();


    // Fast clear window:
    
    i = Find("fastclearwindow");
    assert(i != end());
    pInt = (CIntConfigParam*)*i;
    float ns = (float)(pInt->getOptionValue());
    int   registervalue = (int)((ns/31.25) + 0.5);
    m_pCAENcard->setFastClearWindow(registervalue);

    // How long should we wait for data ready:

    i = Find("waitloops");
    assert(i != end());
    pInt = (CIntConfigParam*)*i;
    m_nLoopTime = pInt->getOptionValue();

    // Finally set the begin run state:
    
    m_pCAENcard->resetEventCounter();
    m_pCAENcard->clearData();
}
/*!
  Called on a per event basis to prepare the card to accept the next
  trigger.  For the CAEN modules, this is a no-op, once initialized,
  they're ready to go.

*/
void
CCAENModule::Prepare()
{
  if(!m_fMultiEvent) 
    m_pCAENcard->clearData();
}

/*!
   Called to read an event into a nondaq buffer.
   this can be used for testing, but is really there
   to make the infrastructure of scaler and event reads
   identical.
   \return Number of words read.
*/
int
CCAENModule::Read(void* pBuffer)
{
  int result(0);
  if(m_pCAENcard) {
    if(m_pCAENcard->dataPresent())
      result =  (m_pCAENcard->readEvent(pBuffer)/sizeof(short));
       
  }
  else {
    result = 0;			// Nonexistent cards give no data.
  }
  return result;
}
/*!
   Called to clear any pending data from the module.
*/
void
CCAENModule::Clear()
{
  if(m_pCAENcard) m_pCAENcard->clearData();
}


CCAENModule::~CCAENModule() {
  delete m_pCAENcard;
}
