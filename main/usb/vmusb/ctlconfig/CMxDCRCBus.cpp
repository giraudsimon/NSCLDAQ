
#include <CMxDCRCBus.h>
#include <MADC32Registers.h>

#include <CVMUSB.h>
#include <CControlModule.h>
#include <CVMUSBReadoutList.h>
#include <VMEAddressModifier.h>
#include <sstream>
#include <memory>
#include <limits>
#include <cstdio>
#include <iostream>
#include <unistd.h>

using namespace std;


/**
 *
 */
CMxDCRCBus::CMxDCRCBus () :
  CControlHardware(),
  m_maxPollAttempts(1000)
{}

/**
 *
 */
CMxDCRCBus::CMxDCRCBus (const CMxDCRCBus& rhs) :
  CControlHardware(rhs)
{
  copy(rhs);
}

CMxDCRCBus& CMxDCRCBus::operator=(const CMxDCRCBus& rhs) 
{
  if (this != &rhs) {
    copy(rhs);
    CControlHardware::operator=(rhs);
  }

  return *this;
}

CControlHardware* CMxDCRCBus::clone() const
{
  return (new CMxDCRCBus(*this));
}

/** Perform specific copy operations for the derived instance
 *
 */
void CMxDCRCBus::copy (const CMxDCRCBus& other) 
{
  m_maxPollAttempts = other.getPollTimeout();
}

/**
 *
 */
void CMxDCRCBus::onAttach(CControlModule& config) 
{
  m_pConfig = &config;
  config.addIntegerParameter("-base");
}

/**
 *
 */
void CMxDCRCBus::Initialize(CVMUSB& ctlr) 
{
  activate(ctlr);
}

/**
 *
 */
std::string CMxDCRCBus::Update(CVMUSB& ctlr) {
  return std::string("OK");
}

/**
 *
 */
std::string CMxDCRCBus::Set(CVMUSB& ctlr, std::string what, std::string value) 
{

  uint16_t dataToWrite = atoi(value.c_str());

  // Try to write until either the operation succeeds or we have exhausted
  // the allowed number of attempts
  int maxAttempts = 4, nAttempts=0;
  uint16_t response=0;
  for (; nAttempts<maxAttempts; ++nAttempts) {

    initiateWrite(ctlr, what, dataToWrite);
    response = pollForResponse(ctlr);

    // if we succeeded with no errors, then stop trying
    if (! responseIndicatesError(response)) {
      break;
    }

  } // end for loop
  
  // if we tried the maximum number of times without success then stop trying
  if (nAttempts==maxAttempts) {
    return convertResponseToErrorString(response);
  }

  // get the value read back from the device
  uint16_t dataRead = readResult(ctlr);

  if (dataRead != dataToWrite) {
    string errmsg("ERROR - CMxDCRCBus::Set - ");
    errmsg += "Failed to read back same value as was written.";
    return errmsg;
  }

  return string("OK");
}

/**
 *
 */
std::string CMxDCRCBus::Get(CVMUSB& ctlr, std::string what) 
{
  
  // Try up to 4 times to successfully complete a read transaction on RCbus
  uint16_t response=0;
  int maxAttempts=4, nAttempts=0;
  for (;nAttempts<maxAttempts; ++nAttempts) {

    initiateRead(ctlr, what);
    response = pollForResponse(ctlr);

    // if we succeeded with no errors, then stop trying
    if (! responseIndicatesError(response)) {
      break;
    }

  } // end for loop

  // If we failed 4 times, return ERROR and the reason why
  if (nAttempts==maxAttempts) {
    return convertResponseToErrorString(response);
  } 

  uint16_t dataRead = readResult(ctlr);


  // the sluggishness of initiating stringstream is acceptable for the moment.
  stringstream retstr;
  retstr << dataRead;
  return retstr.str();

}

/**
 *
 */
void CMxDCRCBus::activate(CVMUSB& ctlr) {

  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  
  unique_ptr<CVMUSBReadoutList> list(ctlr.createReadoutList());
  list->addWrite16(base+NIMBusyFunction, VMEAMod::a32UserData, 3);

  size_t nBytesRead = 0;
  uint32_t data[128];
  int status = ctlr.executeList(*list, data, sizeof(uint16_t), &nBytesRead);

  if (status<0) {
    stringstream errmsg;
    errmsg << "CMxDCRCBus::activate() - executeList returned status = ";
    errmsg << status;

    throw errmsg.str();
  }

}


/**
 *
 */
uint16_t CMxDCRCBus::pollForResponse(CVMUSB& ctlr)
{
    
  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  
  // Poll until the device tells us to stop.
  size_t nAttempts=0;
  uint16_t data = 0;
  int status = 0;
  do {
    // if we timeout, throw
    if (nAttempts >= m_maxPollAttempts) {
      std::string msg("CMxDCRCBus::pollForResponse "); 
      msg += "Timed out while awaiting response";
      throw msg;
    }

    status = ctlr.vmeRead16(base+RCStatus, VMEAMod::a32UserData, &data);

    if (status<0) {
      stringstream errmsg;
      errmsg << "CMxDCRCBus::pollForResponse() - executeList returned status = ";
      errmsg << status;

      throw errmsg.str(); // failure while communicating with VM-USB is worthy 
                          // of a thrown exception
    }
      
    ++nAttempts;
  } while ( (data & RCSTAT_MASK) != RCSTAT_ACTIVE );

  return data;
}


/**
 *
 */
uint16_t CMxDCRCBus::readResult(CVMUSB& ctlr) 
{
  uint32_t base     = m_pConfig->getUnsignedParameter("-base");
  uint16_t dataRead = 0;

  int status = ctlr.vmeRead16(base+RCData, VMEAMod::a32UserData, &dataRead); 
  if (status<0) {
    stringstream errmsg;
    errmsg << "ERROR - ";
    errmsg << "CMxDCRCBus::readResult - failure executing list with ";
    errmsg << "status = " << status;
    throw errmsg.str(); // failure while communicating with VM-USB is worthy 
                        // of a thrown exception
  }

  return dataRead;
}

/**
 *
 */
std::pair<uint16_t,uint16_t> CMxDCRCBus::parseAddress(std::string what)
{
  uint16_t devNo=0;
  uint16_t addr=0;
  int ntokens = sscanf(what.c_str(), "d%hua%hu", &devNo, &addr);
  if (ntokens!=2) {
    throw string("CMxDCRCBus::parseAddress Failed to parse address string");
  }

  return make_pair(devNo, addr);
}


/**
 *
 */
bool CMxDCRCBus::responseIndicatesError(uint16_t datum)
{
  return (((datum & RCSTAT_ADDRCOLLISION) | (datum & RCSTAT_NORESPONSE))!=0);
}

/**
 *
 */
std::string CMxDCRCBus::convertResponseToErrorString(uint16_t datum) 
{

  string message;

  // at the moment there are only two error flags
  if (datum & RCSTAT_ADDRCOLLISION) {
    message = "ERROR - Address collision during last RC-bus operation";  
    message += " : code=";
    message += to_string(datum);
  } else if (datum & RCSTAT_NORESPONSE) {
    message = "ERROR - No response during last RC-bus operation"; 
    message += " : code=";
    message += to_string(datum);
  } else {
    message = "ERROR - Unknown error code returned from last RC-bus operation";
    message += " : code=";
    message += to_string(datum);
  }

  return message;
}


 
void CMxDCRCBus::addParameterWrite(CVMUSBReadoutList& list,
                                   std::pair<uint16_t,uint16_t> addresses,
                                   uint16_t value)
{
  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  list.addWrite16(base+RCModNum, VMEAMod::a32UserData, addresses.first);
  list.addWrite16(base+RCOpCode, VMEAMod::a32UserData, RCOP_WRITEDATA);
  list.addWrite16(base+RCAddr,   VMEAMod::a32UserData, addresses.second);
  list.addWrite16(base+RCData,   VMEAMod::a32UserData, value);
}

/**
 *
 */
void CMxDCRCBus::addParameterRead(CVMUSBReadoutList& list,
                                   std::pair<uint16_t,uint16_t> addresses)
{
  uint32_t base = m_pConfig->getUnsignedParameter("-base");
  list.addWrite16(base+RCModNum, VMEAMod::a32UserData, addresses.first);
  list.addWrite16(base+RCOpCode, VMEAMod::a32UserData, RCOP_READDATA);
  list.addWrite16(base+RCAddr,   VMEAMod::a32UserData, addresses.second);
  list.addWrite16(base+RCData,   VMEAMod::a32UserData, 0);
}


void CMxDCRCBus::initiateWrite(CVMUSB& ctlr, 
                               std::string what,
                               uint16_t value)
{
  // extract the device address and parameter address encoded in "what" argument
  pair<uint16_t,uint16_t> busAddress = parseAddress(what);


  // Create a list and append the ensemble of commands for a parameter write 
  // to it.
  unique_ptr<CVMUSBReadoutList> list(ctlr.createReadoutList());
  addParameterWrite(*list, busAddress, value);

  size_t nBytesRead = 0;
  uint32_t data[128];
  int status = ctlr.executeList(*list, data, sizeof(uint16_t), &nBytesRead);
  if (status<0) {
    stringstream errmsg;
    errmsg << "ERROR - ";
    errmsg << "CMxDCRCBus::Set - executeList returned status = ";
    errmsg << status;

    throw errmsg.str();
  }
}

void CMxDCRCBus::initiateRead(CVMUSB& ctlr, 
                               std::string what)
{
  // extract the device address and parameter address encoded in "what" argument
  pair<uint16_t,uint16_t> busAddress = parseAddress(what);

  // Create a list and append the ensemble of commands for a parameter write 
  // to it.
  unique_ptr<CVMUSBReadoutList> list(ctlr.createReadoutList());
  addParameterRead(*list, busAddress);

  size_t nBytesRead = 0;
  uint32_t data[128];
  int status = ctlr.executeList(*list, data, sizeof(uint16_t), &nBytesRead);
  if (status<0) {
    stringstream errmsg;
    errmsg << "ERROR - ";
    errmsg << "CMxDCRCBus::Get - executeList returned status = ";
    errmsg << status;

    throw errmsg.str();
  }
}
