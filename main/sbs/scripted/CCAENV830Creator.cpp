////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////

#include <config.h>
#include "CCAENV830Creator.h"    				
#include "CCAENV830Module.h"
#include "CReadableObject.h"
#include <TCLInterpreter.h>
#include <TCLResult.h>

#include <assert.h>

//  Complete rewrite see daqdev/NSCLDAQ#510

/**
 * constructor
 *    Just creates the help text.
 */
CCAENV830Creator::CCAENV830Creator()
{
  m_helpText = string("caenv830 - Creates a CAEN 830 32 channel scaler.");
}

/**
 * destructor
 */
CCAENV830Creator::~CCAENV830Creator()
{
   
}

/**
 * Create
 *    Create a CV830Module instance.
 * @param name - name of the module/command to create.
 * @param interp -interpreter on which the command should be registered.
 * @return CReadableObject* - Pointer to newly created object.
 */
CReadableObject*
CCAENV830Creator::Create(const char* name, CTCLInterpreter& interp)
{
  return new CCAENV830Module(name, interp);
}
