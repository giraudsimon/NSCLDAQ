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
#include "CReadableObject.h"    				
#include "CReadOrder.h"

using namespace std;


#include <TCLInterpreter.h>

/*
   Construct a readable object.  Note this class is abstract.  While our
   constructor can be called as a result of a subclass construction, we will never
   be constructing a CReadableObject directly.
   - Construct our base class.
   - Initialize m_pOwner to null.

   \param rName (const string&)
      Name of the object (also gets registered as our manipulation command).
   \param rInterp (CTCLInterpreter&)
       Interpreter object that will run our command.
*/
CReadableObject::CReadableObject (const string& rName,
				  CTCLInterpreter& rInterp) :
  SConfigurableObject(rName, rInterp),
  m_pOwner(0)
{
}
 
/*!
   Destructor is a no-op.
*/

CReadableObject::~CReadableObject ( )
{
}


/*!

Links a readable object into its read order.
Read order objects contain ordered lists 
of modules to read.  

The entire readout
scheme can be though of as a hierarchy
consisting of a top level readout object
that is an ordered list of CDigitizerModule
and CReadOrder objects.  Each object
is allowed to only be controlled by one
ReadOrder module.  This member connects
a module to its read order.

\throw string
    A string exception describing the problem is
thrown e.g. if the module is already linked.

*/
void 
CReadableObject::Link(CReadOrder* pReader)  
{
  if(m_pOwner) {
    throw string("CReadableObject::Link - object is already linked.");
  }
  else {
    m_pOwner = pReader;
  }
}  

/*!  

Unlinks a module from its reader (see the Link member function).
It is an error to unlink a module that is not alread linked.

\throw string
     The function throws a descriptive error message if there
is a problem.

*/
void 
CReadableObject::Unlink()  
{ 
  if(m_pOwner) {
    m_pOwner = (CReadOrder*)NULL;
  } 
  /*
  else {
    throw string("CReadableObject::Unlink - object is not linked");
  }
  */
}  

/*! 


\return bool
   - true if the module has been linked to a reader.
   - false if the module is not currently linked to a reader.


*/
bool 
CReadableObject::isLinked()  
{ 
  return (m_pOwner != (CReadOrder*)NULL);
}  

/*!


Called prior to object deletion (destruction).  If the module
The object must remove itself from its reader.  If it has any submodules
it must unlink them from itself.

If we are still linked, we remove ourselves from our reader...
*/
void 
CReadableObject::OnDelete()  
{ 
  if(m_pOwner) {
    m_pOwner->Remove(this);
    m_pOwner = (CReadOrder*)NULL; // In case we  don't really get deleted.
                                  // Although in fact, our owner will unlink. 
  }
}  

