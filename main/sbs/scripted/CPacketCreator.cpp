/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/


////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CPacketCreator.h"    				
#include "CDigitizerDictionary.h"
#include "CReadOrder.h"
#include <TCLInterpreter.h>
#include <TCLResult.h>
#include <assert.h>


// Substantial rewrite see daqdev/NSCLDAQ#510 use extensible factory pattern.


/*!
   Construct a module creator for the packetizing operator.
   \param rType   (const string&) 
      The type of the 'module' this class creates.  
   \param rDictionary (CDigitizerDictionary&)
      A reference to a dictionary of modules that have been
      constructed.  We need this to construct ReadOrder modules.

*/
CPacketCreator::CPacketCreator (const string& rType,
				CDigitizerDictionary* pDictionary) :
  m_pModules(pDictionary)
 
{  
  // If they try to jinx us with a null dictionary throw a string to
  // return the favor:

  if(!m_pModules) {
    throw
      string("CPacketCreator::CPacketCreator got  a null dictionary ptr");
  }
		m_helpText = rType;
		m_helpText += " - Creates a packet (can have modules added to it)";
} 
/*!
  Destructor.
 */
 CPacketCreator::~CPacketCreator ( )
{
}

/*! 

Returns a pointer to a readable object that is actually a CReadOrder
module.

@param name -name of the new module and command registered for it.
@param intepr - interpreter on which the new modules is registered.
@return CReadableObject* - the new packet object.

*/
CReadableObject* 
CPacketCreator::Create(const char* name, CTCLInterpreter& interp)
{ 

  return new CReadOrder(&interp,  m_pModules, name);

  
}  

