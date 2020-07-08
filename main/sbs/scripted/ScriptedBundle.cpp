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
#include "ScriptedBundle.h"

#include <TCLInterpreter.h>
#include "CDigitizerDictionary.h"
#include "CReadOrder.h"
#include "CModuleCommand.h"
#include "CModuleCreator.h"

using namespace std;

/*!
  Construction is a matter of building the chunks in the right order and,
  where necessary binding them to an interpreter:
  @param pInterp  - Pointer to an object encapsulated Tcl interpreter.

*/
ScriptedBundle::ScriptedBundle(CTCLInterpreter* pInterp)
{
  m_pDictionary    = new CDigitizerDictionary();
  m_pReadOrder     = new CReadOrder(m_pInterp, m_pDictionary);
  m_pModuleCommand = new CModuleCommand(m_pInterp, m_pDictionary);
  
}
/*!
  Destruction is a matter of destroying these chunks in the reverse order:
*/
ScriptedBundle::~ScriptedBundle()
{
  delete m_pModuleCommand;
  delete m_pReadOrder;
  delete m_pDictionary;
}

/*!
   Initialization is a matter of calling the initializers in each of the
   members of the read order, clearing and preparing to read:
*/
void
ScriptedBundle::initialize()
{
    m_pReadOrder->Initialize();	// One-time intialization.
    clear();			// Clear everything for good measure

}
/*!
  Clearing is a matter of calling the clearers in the read order:
*/
void
ScriptedBundle::clear()
{
  m_pReadOrder->Clear();	// Clear and...
  m_pReadOrder->Prepare();	// Prepare for the next event.
}
/*!
   Disbabiling.. for now a no-op but we leave this in in case it's going to be
   implemented later.
*/
void
ScriptedBundle::disable()
{

}

/*!
   Process an initializationfile:
   @param pFilename - Full path to the file to process.
   @param rInterp   - Reference to the interpreter object that will be used
                      to process the file
*/
void
ScriptedBundle::processHardwareFile(const char* pFilename, CTCLInterpreter& rInterp)
{
  rInterp.EvalFile(pFilename);
}
/*!
  Add a module creator to the bundle.  Adding a module creator makes support for the
  module type known to the interpreter that processes the configuration file:
  @param type     - module type string.
  @param creator  - Reference to the module creator to add.
*/
void
ScriptedBundle::addCreator(const char* type, CModuleCreator& creator)
{
  m_pModuleCommand->AddCreator(type, &creator);
}
