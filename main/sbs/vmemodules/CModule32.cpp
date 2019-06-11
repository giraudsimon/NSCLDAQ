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

#include "CModule32.h"

using namespace std;



/*!
  Construct the object.
   \param base (uint32_t [in])
      base address of the module.
   \param crate (int [in] = 0):
      Crate in which the module lives.

*/
CModule32::CModule32(uint32_t base, size_t size, int crate) :
    m_Module(CVmeModule::a32d32, base, size, crate)
{
} 

/*!
  Destructor (currently a no-op).
*/   
CModule32::~CModule32() {
}

/*!
   Write to a key register:
   \param offset  (uint32_t [in]):
     The offset to the register.
*/
void 
CModule32::Key(uint32_t offset) const
{
  poke(0, offset);
}

/*!
   Throw a string message built up out of two components a leader
   for the function and the actual message. To make this even
   easier, the two strings are passed as const char*
   \param pLeader (const char* [in]):
      The first part of the message.
   \param pMessage (const char* [in]):
      The second part of the message.

   \throws string
    (that's the purpose after all).  A string exception
    made up of  "CModule32::" + pLeader + " - " + pMessage
   is thrown
*/
void 
CModule32::ThrowString(const char*  pLeader,
		       const char*  pMessage)
{
  string message("CModule32::");
  message += pLeader;
  message += " -  ";
  message += pMessage;
  throw message;
}
