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

#include "CConfigurableObject.h" /// Dirty dirty dirty!!! but required for 7.4
#include "CSIS3300Module.h"
#include <sis3300.h>
#include <CIntConfigParam.h>
#include <CIntArrayParam.h>
#include <CBoolConfigParam.h>
#include <stdint.h>

#include "PacketUtils.h"
#include "CEnumParameter.h"

static const size_t LARGEST_SIZE(4*128*1024+1024); // largest buffer + slop.

using namespace std;

/*!
   Constructor for the CSIS3300Module class.  The constructor must
   initialize members and set up the parameters required.  Note that
   cached parameter values don't require initialization as the Initilialize()
   member will get called prior to use and will save those values.
   \param name : const string& [in]
       Name of the module being created.
   \param interpreter : CTCLInterpreter& [modified]
       The interpreter on which this module is registered as a command.
*/

CSIS3300Module::CSIS3300Module(const string&    name,
			       CTCLInterpreter& interpreter) :
  CDigitizerModule(name, interpreter),
  m_pModule(0)
{
  // First set up the parameter types that are supported by the
  // base framework (all but enum params which we'll do last since
  // they're a bit weird.
  //

  AddIntParam("base", 0);
  AddIntParam("crate", 0);
  AddIntParam("startdelay", 0);
  AddIntParam("stopdelay", 0);
  AddBoolParam("gate", false);
  AddBoolParam("wrap", false);
  AddIntArrayParam("thresholds", 8, 0);
  AddBoolParam("lt", false);
  AddBoolParam("freerunning", false);
  AddIntParam("usegroups",    0xf);
  AddIntParam("id",          0x3300);
  AddBoolParam("subpacket",   true);

  // Set up the enum params...

  vector<CEnumParameter::enumeratorValue> clockValues;
  clockValues.push_back(CEnumParameter::enumeratorValue("100Mhz", 0));
  clockValues.push_back(CEnumParameter::enumeratorValue("50Mhz",  1));
  clockValues.push_back(CEnumParameter::enumeratorValue("25Mhz",  2));
  clockValues.push_back(CEnumParameter::enumeratorValue("12.5Mhz", 3));
  clockValues.push_back(CEnumParameter::enumeratorValue("6.25Mhz", 4));
  clockValues.push_back(CEnumParameter::enumeratorValue("3.125Mhz", 5));
  clockValues.push_back(CEnumParameter::enumeratorValue("frontpanel", 6));
  clockValues.push_back(CEnumParameter::enumeratorValue("p2", 7));
  CEnumParameter* clock = new CEnumParameter("clock", clockValues, "100Mhz");
  AddConfigParam(clock);

  vector<CEnumParameter::enumeratorValue> sampleSizeValues;
  sampleSizeValues.push_back(CEnumParameter::enumeratorValue("128K", 0));
  sampleSizeValues.push_back(CEnumParameter::enumeratorValue("16K",  1));
  sampleSizeValues.push_back(CEnumParameter::enumeratorValue("4K",   2));
  sampleSizeValues.push_back(CEnumParameter::enumeratorValue("2K",   3));
  sampleSizeValues.push_back(CEnumParameter::enumeratorValue("1K",   4));
  sampleSizeValues.push_back(CEnumParameter::enumeratorValue("512",  5));
  sampleSizeValues.push_back(CEnumParameter::enumeratorValue("256",  6));
  sampleSizeValues.push_back(CEnumParameter::enumeratorValue("128",  7));
  CEnumParameter* sampleSize = new CEnumParameter("samplesize",
						  sampleSizeValues,
						  "16K");
  AddConfigParam(sampleSize);

  // Set up parameter constraints.

  CIntConfigParam* pParam;

  // Crate must be in [0-7] for now:

  ParameterIterator p = Find("crate");
  pParam = dynamic_cast<CIntConfigParam*>(*p);
  pParam->setRange(0,7);

  // usegroups must be in the range [0, 0xf].

  p = Find("usegroups");
  pParam = dynamic_cast<CIntConfigParam*>(*p);
  pParam->setRange(0, 0xf);

}


CSIS3300Module::~CSIS3300Module()
{
  delete m_pModule;
}
              
/*!
   Initialize:  We need to take the configuration parameter and turn them into
   a working module.  In addition the freerunning, usegroups,
   id, and subpacket values get cached so that we can restart data taking
   after an event very quickly.
*/
void
CSIS3300Module::Initialize()
{

  // First get enough information to constuct a new module:

  delete m_pModule;
  m_pModule = static_cast<CSIS3300*>(NULL);

  unsigned long base = getIntValue("base");
  int crate          = getIntValue("crate");

  m_pModule = new CSIS3300(base, crate);
  m_pModule->Reset();
  m_pModule->LightOn();

  // Now configure the module caching stuff we'll need later.
  
  int clockSource  = getEnumValue("clock");
  m_pModule->SetClock(static_cast<CSIS3300::ClockSource>(clockSource));

  m_samplesPerChannel = getEnumValue("samplesize");
  m_pModule->SetSampleSize(static_cast<CSIS3300::SampleSize>(m_samplesPerChannel));

  int startDelay = getIntValue("startdelay");
  m_pModule->SetStartDelay(startDelay ? true : false, startDelay);

  int stopDelay = getIntValue("stopdelay");
  m_pModule->SetStopDelay(stopDelay ? true : false, stopDelay);

  m_pModule->GateMode(getBoolValue("gate"));
  m_pModule->EnableWrap(getBoolValue("wrap"));
  
  CIntArrayParam* pArray;
  ParameterIterator p = Find("thresholds");
  pArray  = dynamic_cast<CIntArrayParam*>(*p);
  bool lt = getBoolValue("lt");
  bool lessflags[8];
  unsigned int thresholds[8];
  for(int i=0; i < 8; i++) {
    thresholds[i] = (*pArray)[i];
    lessflags[i]  = lt;
  }
  m_pModule->SetThresholds(lessflags, thresholds);

  m_freeRunning = getBoolValue("freerunning");

  m_groupsRead = getIntValue("usegroups");
  m_id         = getIntValue("id");
  m_subpackets = getBoolValue("subpacket");
 
  // If free running, we start sampling and external stops it.

}

/*!
   Prepare the module to take data:
*/
void
CSIS3300Module::Prepare()
{
  m_pModule->InitDaq();
  m_pModule->Arm1();
  m_pModule->Arm2();
  if (m_freeRunning) {
    m_pModule->StartSampling();
  }
  m_pModule->LightOff();
 
}
/*!
   Clear data
*/
void
CSIS3300Module::Clear()
{
  m_pModule->ClearDaq();
  if (m_freeRunning) {
    m_pModule->StartSampling();
  }
}

/*!  Return module type:
 */

string
CSIS3300Module::getType() const
{
  return string("sis3300");
}

/*!
   Read into spectrodaq buffer. 
*/
int
CSIS3300Module::Read(void* pBuffer)
{
  uint16_t* p = reinterpret_cast<uint16_t*>(pBuffer);
  if(!m_pModule->WaitUntilDone(100)) {
    return 0; // Nothing read in this case
  }

  // We always put a jumbo packet structure around this;


  uint16_t* start = p;
  p = PacketUtil::startPacket(p, m_id);
  

  // Now we're ready for the body.
  // Alas we must do each group separately without a loop since we have no
  // user callable function to read a single group.
  //

  if (m_groupsRead & 0x1) {
    uint16_t*  pkstart = p;
    int groupId              = 1;
    if(m_subpackets) {
      p = PacketUtil::startPacket(p, groupId);

    }
    p += m_pModule->ReadGroup1(p);

    if(m_subpackets) {
      PacketUtil::endPacket(pkstart, p);      


    }
  }
  if (m_groupsRead & 0x2) {
		
    uint16_t* pkstart = p;
    int groupId              = 2;
    if(m_subpackets) {

      p = PacketUtil::startPacket(p, groupId);

    }
    p += m_pModule->ReadGroup2(p);

    if(m_subpackets) {
      PacketUtil::endPacket(pkstart, p);
      
    }
  }
  if (m_groupsRead & 0x4) {
    uint16_t* pkstart = p;
    int groupId              = 3;
    if(m_subpackets) {
      p = PacketUtil::startPacket(p, groupId);
    }
    p += m_pModule->ReadGroup3(p);

    if(m_subpackets) {
      PacketUtil::endPacket(pkstart, p);

    }

  }
  if (m_groupsRead & 0x8) {
    uint16_t* pkstart = p;
    int groupId              = 4;
    if(m_subpackets) {
      p = PacketUtil::startPacket(p, groupId);

    }
    p += m_pModule->ReadGroup4(p);

    if(m_subpackets) {
      PacketUtil::endPacket(pkstart, p);
    }
  }

  // Close the packet:

	return PacketUtil::endPacket(start, p);
  

}

/*
   Utility function to get the value of an integer config parametr.
*/
int
CSIS3300Module::getIntValue(string name)
{
  ParameterIterator p = Find(name);
  CIntConfigParam*  pInt = dynamic_cast<CIntConfigParam*>(*p);
  return pInt->getOptionValue();
}
/*
   Utility function to get the value of a bool paramter.
*/
bool
CSIS3300Module::getBoolValue(string name)
{
  ParameterIterator p =  Find(name);
  CBoolConfigParam* pBool = dynamic_cast<CBoolConfigParam*>(*p);
  return pBool->getOptionValue();
}
/*
   Utility function to get the value of an enumerated config param.
*/
int
CSIS3300Module::getEnumValue(string name)
{
  ParameterIterator p = Find(name);
  CEnumParameter*   pEnum = dynamic_cast<CEnumParameter*>(*p);
  return pEnum->GetEnumValue();
}


////////////// horrible kludge:

SConfigurableObject::ParameterIterator
CSIS3300Module::AddConfigParam(CConfigurationParameter* param)
{
  return AddParameter(param);


}
