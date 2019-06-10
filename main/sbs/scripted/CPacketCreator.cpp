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


using namespace std;



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
  CModuleCreator(rType),
  m_pModules(pDictionary)
 
{  
  // If they try to jinx us with a null dictionary throw a string to
  // return the favor:

  if(!m_pModules) {
    throw
      string("CPacketCreator::CPacketCreator got  a null dictionary ptr");
  }
} 
/*!
  Destructor.
 */
 CPacketCreator::~CPacketCreator ( )
{
}

/*!
   Copy constructor.  Construct copy of an existing creator: 
   \param aCPacketCreator (const CPacketCreator&)
      Reference to the module creational we are copying.

*/
CPacketCreator::CPacketCreator (const CPacketCreator& aCPacketCreator ) : 
  CModuleCreator (aCPacketCreator),
  m_pModules(aCPacketCreator.m_pModules)
 
{
} 
/*!
  Assignment: An existing (fully constructed)
  object becomes a copy of another existing
  object.
  \param aCPacketCreator (Const CPacketCreator&)
    reference to the guy we're being assigned from.
  \return *this
     To support assignment chaining.
 */
CPacketCreator& 
CPacketCreator::operator= (const CPacketCreator& aCPacketCreator)
{
  if(this != &aCPacketCreator) {
    CModuleCreator::operator=(aCPacketCreator);
    m_pModules = aCPacketCreator.m_pModules;
  }
  return *this;
}


/*!
   Equality comparison operator.
   \param aCPacketCreator (const CPacketCreator&)
      Reference to the object to which we are being compared.
   \return int
     - 0 If *this is \em not the same as aCPacketCreator.
     - 1 If *this \em is the same as aCPacketCreator
*/
int 
CPacketCreator::operator== (const CPacketCreator& aCPacketCreator) const
{
  return (CModuleCreator::operator==(aCPacketCreator)     &&
	  (m_pModules == aCPacketCreator.m_pModules));
}
/*!
   Inequality comparison operator.
   \param aCPacketCreator (const CPacketCreator&)
      Reference to the object to which we are being compared.
   \return int
     - 1 If *this is \em not the same as aCPacketCreator.
     - 0 If *this \em is the same as aCPacketCreator
*/
int
CPacketCreator::operator!=(const CPacketCreator& aCPacketCreator) const
{
  return !(operator==(aCPacketCreator));
}




/*! 

Returns a pointer to a readable object that is actually a CReadOrder
module.
\param rInterp (CTCLInterpreter& )
    Reference to the interpreter that is running the creational command.
\param rResult (CTCLResult&)
    Reference to the result of the interpreter.  This is the 'return value'
    of the command.
\param nArgs (int):
   Number of remaining command line parameters after the command string.
\param pArgs (char**)
   The remaining command line parameters after the command string.
   There must be at least two: 
   - name  - The name of the module being created.
   - type  - The type of module being created (asserted for equality 
             against getModuleType().
   Any remaining parameters are treated as configuration parameters
   and passed to the module's Configure member function.

\return CReadableObject*
    Pointer to the read order that was created.

*/
CReadableObject* 
CPacketCreator::Create(CTCLInterpreter& rInterp,
		       CTCLResult&      rResult,
		       int              nArgs,
		       char**           pArgs)  
{ 
  assert(nArgs >= 2);		// Our caller should have checked this.
  string Name(*pArgs);		// Module name.
  pArgs++;
  nArgs--;

  string Type(*pArgs);		// Module type.
  pArgs++;
  nArgs--;

  assert(Type == getModuleType()); // A real interesting bug if false.

  CReadOrder* pModule = new CReadOrder(&rInterp,
				       m_pModules,
				       Name);

  //  Configure from the remaining params if there are any:

  if(nArgs) {
    int status = pModule->Configure(rInterp, rResult, nArgs, pArgs);
    if(status != TCL_OK) {	// Configure failed!!
      delete pModule;		// Configure will have filled in rResult.
      pModule = (CReadOrder*)NULL;
    }
  }
  return pModule;
  
}  

/*! 	

Returns the help string associated with the module
creational.

*/
string 
CPacketCreator::Help()  
{
  return string("Creates a packet (can have modules added to it)"); 
}
