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
#include "CVMUSBModule.h"
#include "CVMUSB.h"
#include "CVMUSBReadoutList.h"	// for the AM codes.

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
CVMUSBModule::CVMUSBModule(string name) 
{}
/**
 * Copy construction
 */

/**
 * Destruction is literally a no-op.
 */
CVMUSBModule::~CVMUSBModule()
{
}

/**
 *  Identical configuration means identical so they are all equal since there's not really
 *  a configuration.
*/
int
CVMUSBModule::operator==(const CVMUSBModule& rhs) const
{
  return 1;
}
/**
 * So therefore none are ever unequal.
 */
int
CVMUSBModule::operator!=(const CVMUSBModule& rhs) const
{
  return 0;
} 


/**
 * Set - this is responsible for accepting a 
 *       VMUSB immediate list and executing it.
 * @param vme       - CVMUSB handle to the VMUSB device.  
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
CVMUSBModule::ImmediateList(CVMUSB& vme, string value)
{


  uint8_t* readdata(0);
  try {
    size_t           maxBuffer    = decodeInputSize(value);
    size_t           bytesread;
    readdata                      = new uint8_t[maxBuffer];
    vector<uint32_t> listContents = decodeList(value);
    CVMUSBReadoutList theList(listContents);
    int status                    = vme.executeList(theList,
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
      msg += "usb_bulk_read faild: ";
    }
    char errorMessage[1000];
    strerror_r(ecode, errorMessage, sizeof(errorMessage)); // Safest to use in threaded code.
    msg += errorMessage;
    throw msg;
    
  }
  catch (string msg) {		// Deep calls throw a string error message:
    string error  = "ERROR - ";
    error        += msg;
    throw error;
    delete readdata;
  }
}
/**
 * Set the action register.
 * @param controller - the VMUSB controller reference.
 * @param value      - the action register value.
 *
 * @return string - 
 * @retval "OK" -  success.
 * @retval "ERROR - <reason>"  on failure.
 */
string
CVMUSBModule::writeActionRegister(CVMUSB& vme, uint16_t value)
{
  vme.writeActionRegister(value);
  return "";
} 


/**
 * loadList
 *   Load a VMUSB list.  Loaded lists can respond to triggers which in turn
 *   result in data buffers that can be read via bulk reads.
 *
 * @param vmusb  - reference to the controller object.
 * @param number - The list number.
 * @param offset - The offset at which the list will be loaded.
 * @param list   - The stringified list.
 *                 The list is stringified in a way that decodeList can give us the
 *                 vector of list elements.
 *
 * @return std::string
 * @retval "OK" - The list was loaded.
 * @retval "ERROR - <message>" The list could not be loaded and <message> is the reason why.
 */
std::string
CVMUSBModule::loadList(CVMUSB& vme, int number, int offset, std::string list)
{
  // validate the list number:

  if ((number < 0) || (number > 7)) {
    return std::string("ERROR - list number invalid");
  }
  // Offset we'll just require not < 0 since later VM-USBs might have bigger memories.

  if (offset < 0) {
    return std::string("ERROR - offset cannot be less than zero");
  }

  try {
    std::vector<uint32_t> listVector = decodeList(list);
    CVMUSBReadoutList     vmusbList(listVector);
    int status = vme.loadList(number, vmusbList, offset);
    if (status < 0) {
      std::string error = strerror(errno);
      std::string msg   = "load failed: ";
      msg              += error;
      throw msg;
    }


  }
  catch (std::string msg) {
    std::string result = "ERROR - ";
    result += msg;
    return result;
  }
  return "OK";

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
CVMUSBModule::decodeInputSize(std::string& list)
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
  unsigned long result = strtoul(pSize, &endPointer, 0);
  if (endPointer == pSize) {
    throw "First element of the Set parameter must be a valid buffer size and is not";
  }
  return (size_t) result;
  
  
}
/**
 * Decode the second element of the value list...that represents the VM-USB list
 * @param list   - reference to the data list.
 * @return vector<uint32_t>
 * @return the VM-USB list desired.
 * @throw string - if the second element is not a list and the second element's list elements are not
 *                 all uint32_t's.
 */
vector<uint32_t>
CVMUSBModule::decodeList(string& list)
{
  CTCLInterpreter interp;
  CTCLList        outerList(&interp, list);
  StringArray     outerListArray;
  outerList.Split(outerListArray);

  // Presumably decodeInputSize has already validated the list size so the followig is legal:

  CTCLList        vmusbList(&interp, outerListArray[1]);
  StringArray     innerList;
  vmusbList.Split(innerList);

  // Now we can start to build the output list:

  vector<uint32_t> result;
  for (int i = 0; i < innerList.size(); i++) {
    uint32_t    item;
    const char* pItem = innerList[i].c_str();
    char*      pEnd;
    item  = strtoul(pItem, &pEnd, 0);
    if (pEnd == pItem) {
      throw "List of VM-USB stack values must all be uint32_t and is not";
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
CVMUSBModule::marshallOutput(uint8_t* buffer, size_t numBytes)
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
