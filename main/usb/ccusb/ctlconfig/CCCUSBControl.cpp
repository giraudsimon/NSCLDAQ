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
#include "CCCUSBControl.h"
#include "CControlModule.h"
#include "CCCUSB.h"
#include "CCCUSBReadoutList.h"  // for the AM codes.

#include <TCLInterpreter.h>
#include <TCLList.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <errno.h>

using namespace std;

//////////////////////////////////////////////////////////////
// Implementation of canonical operations.
//
/**
 * Construction is even easier than normal because we don't actually
 * need the configuration.
 * @param name - Name of the module supplied in the script.
 */
CCCUSBControl::CCCUSBControl() :
  CControlHardware()
{}
/**
 * Copy construction
 */
CCCUSBControl::CCCUSBControl(const CCCUSBControl& rhs) :
  CControlHardware(rhs)
  
{
}
/**
 * Destruction is literally a no-op.
 */
CCCUSBControl::~CCCUSBControl()
{
}
/**
 * Assignment is a clone operation:
 * @param rhs - The source of the assignment (this is the target).
 * @return CCCUSBControl&
 * @retval *this
 */
CCCUSBControl&
CCCUSBControl::operator=(const CCCUSBControl& rhs)
{
  if (this != &rhs) {
    CControlHardware::operator=(rhs);
  }
  return *this;
}
/**
 *  Identical configuration means identical so they are all equal since there's not really
 *  a configuration.
*/
int
CCCUSBControl::operator==(const CCCUSBControl& rhs) const
{
  return 1;
}
/**
 * So therefore none are ever unequal.
 */
int
CCCUSBControl::operator!=(const CCCUSBControl& rhs) const
{
  return 0;
} 

/**
 * Since there's not really a configuration, this is a no-op.
 */
void
CCCUSBControl::onAttach(CControlModule& configuration)
{
  m_pConfig = &configuration;
}

// Init and update do nothing as well:

void
CCCUSBControl::Initialize(CCCUSB& vme)
{}

string
CCCUSBControl::Update(CCCUSB& vme)
{
  return string("OK");
}
/**
 * Set - this is responsible for accepting a 
 *       CCUSB immediate list and executing it.
 * @param vme       - CCCUSB handle to the VMUSB device.  
 * @param parameter - 'list'
 * @param value     - TCL List form that describes the readout list:
 *                    Element 0:   Max size of input buffer we are allowed to receive.
 *                    Element 1:   Tcl list that contains the uint32_t's that make up the
 *                                 operation list.
 * @return string:
 *   OK {result}    Success {result} is the read data from the list execution bytewise.
 *   ERROR - reason   not successful.
 */
string
CCCUSBControl::Set(CCCUSB& vme, string parameter, string value)
{
  if (parameter != "list") {
    return string ("ERROR - Invalid parameter name, must be 'list'");
  }

  uint8_t* readdata(0);
  try {
    if (parameter != "list") {
      string msg = "Parameter name must be 'list' and was ";
      msg       += parameter;
      throw msg;
    }
    size_t           maxBuffer    = decodeInputSize(value);
    size_t           bytesread;
    readdata                      = new uint8_t[maxBuffer];
    vector<uint16_t> listContents = decodeList(value);
    CCCUSBReadoutList theList(listContents);
    int status = vme.executeList( theList,
                                  readdata,
                                  maxBuffer, &bytesread);
    if (status == 0) {
      string result =  marshallOutput(readdata, bytesread);
      delete []readdata;
      return result;
    }
    string msg = "ERROR - ";
    int ecode  = errno;
    if (status == -1) {
      msg += "usb_bulk_write failed: ";
    }
    else {
      msg += "usb_bulk_read failed: ";
    }
    char errorMessage[1000];
    strerror_r(ecode, errorMessage, sizeof(errorMessage)); // Safest to use in threaded code.
    msg += errorMessage;
    throw msg;
    
  }
  catch (string msg) {    // Deep calls throw a string error message:
    string error  = "ERROR - ";
    error        += msg;
    delete [] readdata;
    throw error;
  }
}
/**
 * Get is always an error.
 */
string
CCCUSBControl::Get(CCCUSB& vme, string parameter) 
{
  return "ERROR - Get not supported by VMUSBModule driver";
}
/**
 * Clone is a noop.
 */
CControlHardware*
CCCUSBControl::clone() const
{
  return (new CCCUSBControl);
}


////////////////////////////////////////////////////////////////////////////////////////

/**
 * decode the output buffer size being provided by the driver.  This will be a size_t
 * from the first element of the list.
 * @param value - the value handed to set.
 * @return size_t
 * @retval desired read buffer size.
 * @throw string if the list is not a valid Tcl list or if the
 *        first element doesn't decode as a valid size_t (uint32_t for now).
 */
size_t 
CCCUSBControl::decodeInputSize(std::string& list)
{
  CTCLInterpreter interp;
  CTCLList        tclList(&interp, list);

  StringArray      brokenDownList;
  tclList.Split(brokenDownList);
  if (brokenDownList.size() != 2) {
    throw "Incorrect list size in Set parameter value must have 2 elements";
  }
  char*         endPointer;
  const char*   pSize = brokenDownList[0].c_str();
  uint16_t result = strtoul(pSize, &endPointer, 0);
  if (endPointer == pSize) {
    throw "First element of the Set parameter must be a valid buffer size and is not";
  }
  return (size_t) result;
  
  
}
/**
 * Decode the second element of the value list...that represents the CC-USB list
 * @param list   - reference to the data list.
 * @return vector<uint16_t>
 * @return the CC-USB list desired.
 * @throw string - if the second element is not a list and the second element's list elements are not
 *                 all uint16_t's.
 */
vector<uint16_t>
CCCUSBControl::decodeList(string& list)
{
  CTCLInterpreter interp;
  CTCLList        outerList(&interp, list);
  StringArray     outerListArray;
  outerList.Split(outerListArray);

  // Presumably decodeInputSize has already validated the list size so the followig is legal:

  CTCLList        vmusbList(&interp, outerListArray[1]);
  StringArray     innerList;
  vmusbList.Split(innerList);
  int innerListSize = innerList.size();
  // Now we can start to build the output list:

  vector<uint16_t> result;
  for (int i = 0; i < innerListSize; i++) {
    uint16_t    item;
    const char* pItem = innerList[i].c_str();
    char*      pEnd;
    item  = (0xffff & strtoul(pItem, &pEnd, 0));
    if (pEnd == pItem) {
      throw "List of CC-USB stack value must all be of type uint16_t and are not";
    }
    result.push_back(item);
  }
  return result;
}
/**
 * Marshall the output string for an ok result.  This will be the string
 * OK - {byte1 byte2...}
 * where byten is the n'th byte of the buffer read by the executeList operation.
 * @param buffer - Pointer to the data buffer read from the vmusb
 * @param numBytes -number of bytes read.
 * @return string
 * @retval - value to return to the ultimate caller of the Set command.
 */
string
CCCUSBControl::marshallOutput(uint8_t* buffer, size_t numBytes)
{
  string result = "OK  - {";
  for (int i =0; i < numBytes; i++) {
    char valueBuffer[8];
    uint8_t value = *buffer++;
    unsigned uValue = value;
    sprintf(valueBuffer, "0x%02hhx ", uValue);
    result += valueBuffer;
    
  }

  result += "}";
  return result;
}
