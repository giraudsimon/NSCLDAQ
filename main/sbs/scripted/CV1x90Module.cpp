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

#include "CV1x90Module.h"
#include <CCAENV1x90.h>
#include "TCLInterpreter.h"
#include "CIntConfigParam.h"
#include "CBoolConfigParam.h"
#include "CIntArrayParam.h"
#include <assert.h>
#include <iostream>

#include <stdint.h>


using namespace std;


/*!
   Constructs a module. The main thing is to register all of the
   configuration key words we're going to support.
   \param  name - with which to register the object command.
   \param  interp - Interpreter on wich the command should be registered.
*/
CV1x90Module::CV1x90Module(string           name,
			   CTCLInterpreter& interp) :
  CDigitizerModule(name, interp),
  m_pModule(0)
{
  // Set up the configuration parameters:

  AddIntParam(string("base"), 0);
  AddIntParam(string("vsn"), 0);
  AddIntParam(string("crate"), 0);

  // The terminate enum.
  
  vector<pair<string, int> > terminationValue;
  terminationValue.push_back(pair<string, int>("switch", 0));
  terminationValue.push_back(pair<string, int>("on", 1));
  terminationValue.push_back(pair<string, int>("off", 2));
  AddEnumParam(string("termination"), terminationValue, "on");

  AddBoolParam(string("tagtriggertime"), true);

  // What does the ECL Programmable output represent?

  vector<pair<string, int> > progoutValues;
  progoutValues.push_back(pair<string, int>("dataready",   CCAENV1x90::DATA_READY));
  progoutValues.push_back(pair<string, int>("full",        CCAENV1x90::FULL));
  progoutValues.push_back(pair<string, int>("almostfull",  CCAENV1x90::ALMOST_FULL));
  progoutValues.push_back(pair<string, int>("error",       CCAENV1x90::ERROR));
  AddEnumParam(string("eclprogout"), progoutValues, "dataready");


  // How does the trigger work (continous storage or trigger matching)

  vector<pair<string, int> > triggerValues;
  triggerValues.push_back(pair<string, int>("continuous", 0));
  triggerValues.push_back(pair<string, int>("matching",   1));
  AddEnumParam(string("triggermode"), triggerValues, "matching");

  // Various timing widths etc.

  AddIntParam(string("windowwidth"), 0);
  AddIntParam(string("offset"), 0);
  AddIntParam(string("extramargin"), 0);
  AddIntParam(string("rejectmargin"), 0);
  
  // 

  AddBoolParam(string("subtracttriggertime"), true);

  // The edge detection mode:

  vector<pair<string, int> > edgeValues;
  edgeValues.push_back(pair<string, int>("pair",    CCAENV1x90::EdgeMode_Pair));
  edgeValues.push_back(pair<string, int>("leading", CCAENV1x90::EdgeMode_Leading));
  edgeValues.push_back(pair<string, int>("trailing",CCAENV1x90::EdgeMode_Trailing));
  edgeValues.push_back(pair<string, int>("both",    CCAENV1x90::EdgeMode_Both));
  AddEnumParam(string("edgedetectmode"), edgeValues, "leading");

  AddBoolParam(string("encapsulatetdc"), true);

  // Maximum hits... they would be numbers but for the fact that there are only a limited set
  // and "unlimited" is allowed.

  vector<pair<string, int> > maxHitValues;
  maxHitValues.push_back(pair<string, int>("1",      CCAENV1x90::HITS_1));
  maxHitValues.push_back(pair<string, int>("2",      CCAENV1x90::HITS_2));
  maxHitValues.push_back(pair<string, int>("4",      CCAENV1x90::HITS_4));
  maxHitValues.push_back(pair<string, int>("8",      CCAENV1x90::HITS_8));
  maxHitValues.push_back(pair<string, int>("16",     CCAENV1x90::HITS_16));
  maxHitValues.push_back(pair<string, int>("32",     CCAENV1x90::HITS_32));
  maxHitValues.push_back(pair<string, int>("64",     CCAENV1x90::HITS_64));
  maxHitValues.push_back(pair<string, int>("128",    CCAENV1x90::HITS_128));
  maxHitValues.push_back(pair<string, int>("unlimited", CCAENV1x90::HITS_UNLIMITED));
  AddEnumParam(string("maxhits"), maxHitValues, "unlimited");

  // Least significant bit resolution:

  vector<pair<string, int> > lsbValues;
  lsbValues.push_back(pair<string, int>("25ps",      CCAENV1x90::Res_25ps));
  lsbValues.push_back(pair<string, int>("100ps",     CCAENV1x90::Res_100ps));
  lsbValues.push_back(pair<string, int>("200ps",     CCAENV1x90::Res_200ps));
  lsbValues.push_back(pair<string, int>("800ps",     CCAENV1x90::Res_800ps));
  AddEnumParam(string("individuallsb"), lsbValues, "100ps");

  vector<pair<string, int> > pairleValues;
  pairleValues.push_back(pair<string, int>("100ps",   CCAENV1x90::LE_100ps));
  pairleValues.push_back(pair<string, int>("200ps",   CCAENV1x90::LE_200ps));
  pairleValues.push_back(pair<string, int>("400ps",   CCAENV1x90::LE_400ps));
  pairleValues.push_back(pair<string, int>("800ps",   CCAENV1x90::LE_800ps));
  pairleValues.push_back(pair<string, int>("1600ps",  CCAENV1x90::LE_1600ps));
  pairleValues.push_back(pair<string, int>("3120ps",  CCAENV1x90::LE_3120ps));
  pairleValues.push_back(pair<string, int>("6250ps",  CCAENV1x90::LE_6250ps));
  pairleValues.push_back(pair<string, int>("12500ps", CCAENV1x90::LE_12500ps));
  AddEnumParam(string("pairleresolution"), pairleValues, "100ps");

  vector<pair<string, int> > pairwidValues;
  pairwidValues.push_back(pair<string, int>("100ps",       CCAENV1x90::PW_100ps));
  pairwidValues.push_back(pair<string, int>("200ps",       CCAENV1x90::PW_200ps));
  pairwidValues.push_back(pair<string, int>("400ps",       CCAENV1x90::PW_400ps));
  pairwidValues.push_back(pair<string, int>("800ps",       CCAENV1x90::PW_800ps));
  pairwidValues.push_back(pair<string, int>("1600ps",       CCAENV1x90::PW_1600ps));
  pairwidValues.push_back(pair<string, int>("3200ps",       CCAENV1x90::PW_3200ps));
  pairwidValues.push_back(pair<string, int>("6250ps",       CCAENV1x90::PW_6250ps));
  pairwidValues.push_back(pair<string, int>("12500ps",      CCAENV1x90::PW_12500ps));
  pairwidValues.push_back(pair<string, int>("25ns",       CCAENV1x90::PW_25ns));
  pairwidValues.push_back(pair<string, int>("50ns",       CCAENV1x90::PW_50ns));
  pairwidValues.push_back(pair<string, int>("100ns",      CCAENV1x90::PW_100ns));
  pairwidValues.push_back(pair<string, int>("200ns",      CCAENV1x90::PW_200ns));
  pairwidValues.push_back(pair<string, int>("400ns",      CCAENV1x90::PW_400ns));
  pairwidValues.push_back(pair<string, int>("800ns",      CCAENV1x90::PW_800ns));
  AddEnumParam("pairwidthresolution", pairwidValues, "100ps");

  vector<pair<string, int> > deadValues;
  deadValues.push_back(pair<string, int>("5ns",           CCAENV1x90::DT_5ns));
  deadValues.push_back(pair<string, int>("10ns",          CCAENV1x90::DT_10ns));
  deadValues.push_back(pair<string, int>("30ns",          CCAENV1x90::DT_30ns));
  deadValues.push_back(pair<string, int>("100ns",         CCAENV1x90::DT_100ns));
  AddEnumParam("deadtime", deadValues, "5ns");
}
/*!
  Now we initialize the module.  This is all driven from our configuration.
*/
void
CV1x90Module::Initialize()
{
  // First we need to destroy any existing module and get enough stuff from the
  // configuration to let us make a new one:
  //
  delete m_pModule;
  int  base = getIntegerValue("base");
  int  crate= getIntegerValue("crate");
  int  vsn  = getIntegerValue("vsn");

  m_pModule = new CCAENV1x90(vsn, crate, base);

  // initialize the module as per the configuration:

  m_pModule->Reset();
  m_pModule->Clear();

  switch (getEnumValue("termination")) {
  case 0:
    m_pModule->TerminateWithSwitch();
    break;
  case 1:
    m_pModule->Terminate();
    break;
  case 2:
    m_pModule->Unterminate();
    break;
  }
  if (getBoolValue("tagtriggertime")) {
    m_pModule->EnableTDCEncapsulation();
  }
  else {
    m_pModule->DisableTDCEncapsulation();
  }

  // Set the ECL Progout pin meaning:

  m_pModule->DefineECLOutput((CCAENV1x90::ECLOutputSelect)getEnumValue("eclprogout"));

  // Trigger mode:

  if (getEnumValue("triggermode") == 0) {
    m_pModule->ContinuousStorageMode();
  }
  else {
    m_pModule->TriggerMatchMode();
    
    // These timing parameters are only neeed in trigger matching mode:

    m_pModule->SetWindowWidth(getIntegerValue("windowwidth"));
    m_pModule->SetWindowOffset(getIntegerValue("offset"));
    m_pModule->SetExtraSearchMargin(getIntegerValue("extramargin"));
    m_pModule->SetRejectMargin(getIntegerValue("rejectmargin"));

    if (getBoolValue("subtracttriggertime")) {
      m_pModule->EnableTriggerTimeSubtraction();
    }
    else {
      m_pModule->DisableTriggerTimeSubtraction();
    }
  }
  if (getBoolValue("encapsulatetdc")) {
    m_pModule->EnableTDCEncapsulation();
  }
  else {
    m_pModule->DisableTDCEncapsulation();
  }

  // Determine how the signals are interpreterd, and  the various resolution parameters.

  m_pModule->SetEdgeDetectMode((CCAENV1x90::EdgeMode)getEnumValue("edgedetectmode"));
  m_pModule->SetMaxHitsPerEvent((CCAENV1x90::HitMax)getEnumValue("maxhits"));
  m_pModule->SetIndividualLSB((CCAENV1x90::Resolution)getEnumValue("inidividuallsb"));
  m_pModule->SetPairResolutions((CCAENV1x90::LEResolution)getEnumValue("parileresolution"),
				(CCAENV1x90::PWResolution)getEnumValue("pairwidthresolution"));
  m_pModule->SetDoubleHitResolution((CCAENV1x90::DeadTime)getEnumValue("deadtime"));

}

/*!
  Prepare for readout:
*/
void
CV1x90Module::Prepare()
{
  m_pModule->EnableAllChannels();
  m_pModule->EventReset();
}
/*!
  Clear the module post read.. not required.
*/

void
CV1x90Module::Clear()
{}

/*!
   
  Read the module into an ordinary buffer.  
*/

int
CV1x90Module::Read(void* pBuffer)
{
  if(m_pModule->DataReady()) {
    unsigned int nLongs = m_pModule->ReadData(pBuffer, 1024);
    return (nLongs/(sizeof(uint32_t)/sizeof(uint16_t)));
  }
  else {
    return 0;
  }
}


/*!
  Destructor must eliminat the module.

*/

CV1x90Module::~CV1x90Module() 
{
  delete m_pModule;
}


string
CV1x90Module::getType()  const
{
  return string("caenv1190/caenv1290");
}
