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
#include "CReadoutHardware.h"
#include "CReadoutModule.h"
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>

#include <tcl.h>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include <set>

#include <iostream>

using namespace std;



// Return the named integer parameter.
// The configuration validators have ensured the parameter actually
// is an integer.

unsigned int
CReadoutHardware::getIntegerParameter(string name)
{
  string value = m_pConfiguration->cget(name);
  return strtoul(value.c_str(), NULL, 0);
}

// Get the value of a boolean parameter.
// In this case, we create the set of valid true values.  If the
// string matches at least on of them (is in the set), 
// then we can return true.. otherwise false.
// Once again, the validator, and our initial value, ensure the
// string is a valid bool
//
bool
CReadoutHardware::getBoolParameter(string name)
{
  string value = m_pConfiguration->cget(name);
  set<string>  trueValues;
  trueValues.insert("true");
  trueValues.insert("yes");
  trueValues.insert("yes");	// Valid true values.
  trueValues.insert("1");
  trueValues.insert("on");
  trueValues.insert("enabled");

  return (trueValues.count(value) != 0);	// value matches one of the set members.

}
// Retrieve an array of uint16_t values.
// 
void
CReadoutHardware::getArray(string name, vector<uint16_t>& value)
{
  int    argc;
  const char **argv;
  string sValue = m_pConfiguration->cget(name);
  Tcl_SplitList(NULL, sValue.c_str(), &argc, &argv);

  assert(argc == 16);		// Validator should have done this.

  for(int i =0; i < 16; i++) {
    value.push_back(static_cast<uint16_t>(strtol(argv[i], NULL, 0)));
  }
  

  Tcl_Free((char*)argv);

   
}
/**********************************************************************/
/*              Checked immediate CAMAC operations                    */
/**********************************************************************/

/*
   This function performs an immediate control operation on the
   CCUSB.  The return value and the q/x are checked.  If any of them
   indicate the command could not be executed, a string exception
   is thrown.  The user supplies a formatting string
   that can contain appropriate sprintf control codes that insert
   the slot, subaddress, and function code (in that order) into their
   message.  The message is prepended with a stock message explaining
   what went wrong (e.g. "Bad status on CCUSB control operation: user msg")
   and then thrown as a string.

   Parameter:
    controller - Reference to the CCUSB controller object on which the
                 operation will be attempted.
    n          - Target Slot
    a          - Target subadress
    f          - Function code to do.
    message    - Message template.

*/
void
CReadoutHardware::checkedControl(CCCUSB& controller,
				 int n, int a, int f, string message,
				 bool needQ)
{
  uint16_t qx;
  int      status = controller.simpleControl(n,a,f, qx);
  check(status, qx, 
	n,a,f,0,
	string("Error in checkedControl: "), message,
	needQ);
}
/*
  This function is similar to checkedControl but does a write.
  parameters are the same, but the data parameter is the write data.
*/
void
CReadoutHardware::checkedWrite16(CCCUSB& controller,
				 int n, int a, int f, uint16_t data,
				 string message,
				 bool needQ)
{
  uint16_t qx;
  int      status = controller.simpleWrite16(n,a,f, data, qx);
  check(status, qx,
	n,a,f,data,
	string("Error in checkedWrite16: "), message,
	needQ);
}

/*
   Checks the status of a camac function and if it has failed,
   throws an appropriate string exception.
   The string exception is constructed as follows:
   prefix : reason : formatted-text.
   where formatted-text is constructed by doing an 

   sprintf(fomrmattedtext, format, n,a,f,d)
   
*/
void
CReadoutHardware::check(int status, uint16_t qx,
			int n, int a, int f, int d,
			string prefix, string format,
			bool needQ
			)
{
  string message = prefix;
  bool   trouble = false;

  if (status != 0) {
    message += " CCUSB operation failed : ";
    trouble = true;
  }
  // If q/x are missing, then the most serious would be a missing
  // X and then finally a missing Q
  //
  uint16_t needMask = CCCUSB::X;
  if (needQ) needMask |= CCCUSB::Q;
  if (!trouble && ((qx & needMask) != (needMask))) {
    trouble = true;
    if ((qx & CCCUSB::X) == 0) {
      message += " No X response from module : ";
    }
    if (needQ && ((qx & CCCUSB::Q) == 0)) {
      message += " No Q response from module : ";
    }
  }
  if (trouble) {
    char formattedText[1000];
    snprintf(formattedText, sizeof(formattedText), format.c_str(), n,a,f,d);
    message += formattedText;
    throw message;
  }
}
