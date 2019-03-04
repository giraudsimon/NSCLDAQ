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
#include "CScriptedScalers.h"
#include <TCLInterpreter.h>
#include "CDigitizerDictionary.h"
#include "CModuleCommand.h"
#include "CReadOrder.h"
#include "ScriptedBundle.h"
#include <RangeError.h>
#include <ErrnoException.h>
#include <TCLException.h>


#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h> 
// Standard scaler creators.

#include "CCAENV830Creator.h"
#include "CSIS3300Creator.h"

#ifndef PATH_SEP
#define PATH_SEP '/'
#endif

using namespace std;

////////////////////////////////////////////////////////////
// Implementation of the public interfaces
////////////////////////////////////////////////////////////

/*!
  Constructor initializes all the member data pointers to null.

  initialize will be what actually constructs them:
*/
CScriptedScalers::CScriptedScalers() :
  m_pBundle(0),
  m_pInterp(0)
{}

/*!
   Initialization in our case involves
   - Deleting the old infrastructure and creating new.
   - Adding the supported modules to the new infrastructure.
   - Process the configuration file.
*/
void
CScriptedScalers::initialize()
{
  delete m_pBundle;
  delete m_pInterp;

  // Create new infrastructure:

  m_pInterp     = new CTCLInterpreter();
  m_pBundle     = new ScriptedBundle(m_pInterp);

  // Add standard and user written module creators.

  addStandardCreators();
  addUserWrittenCreators();

  // Figure out which file is the config file and process it:

  try {
    string configFile = getConfigurationFile();
    m_pBundle->processHardwareFile(configFile.c_str(),
				   *m_pInterp);
    m_pBundle->initialize();

  }
  catch(CErrnoException& e) {
    string error = "Configuration file processing failed\n";
    error        += "Unable to open configuration file\n";
    error        += e.ReasonText();
    throw error;
  }
  catch(string msg) {
    string error = "Configuration file processing failed:\n";
    error       += msg;
    error       += "\nAt: \n";
    error       += CTCLException::getTraceback(*m_pInterp);
    throw error;
  }
  catch(const char* msg) {
    string error = "Configuration file processing failed:\n";
    error       += msg;
    error       += "\nAt: \n";
    error       += CTCLException::getTraceback(*m_pInterp);
    throw error;
  }
  catch(CTCLException& tclfail) {
    string error = "Configuration file processing failed\n";
    error       +=tclfail.ReasonText();
    error       += "\nTraceback:\n";
    error       += tclfail.getTraceback();
    throw error;
  }
  catch (...) {
    throw "Configuration file processing failed";
  }
}
/*!
  Clear is delegated to the bundle:
*/
void
CScriptedScalers::clear()
{
  m_pBundle->clear();
}

/*!
   Disable the bundle:
*/
void
CScriptedScalers::disable()
{
  m_pBundle->disable();
}
/*!
   Read the data from the scalers into the output vector.
   We're going to assume we need 64 channels per module..hopefully that's 2x what
   we actually need:
   @return std::vector<uint32_t>
   @retval the scalers we read out.

*/
vector<uint32_t>
CScriptedScalers::read()
{
  size_t maxLongs = m_pBundle->m_pReadOrder->readersize();
  uint32_t* pBuffer = new uint32_t[maxLongs*64];
  vector<uint32_t>  result;

  int numRead = m_pBundle->m_pReadOrder->Read(pBuffer);

  // A buffer overrun is really serious stuff at this point we'll throw
  // the exception.. consider dying later as the resulting program
  // may not be recoverably sane...depending on what got ovewritten.

  if (numRead*sizeof(uint16_t) > maxLongs*sizeof(uint32_t)) {
    throw CRangeError(0, maxLongs*sizeof(uint32_t),
		      numRead*sizeof(uint16_t),
		      "CScriptedScalers::read - buffer overrun while reading scalers");
  }
  // Put the scalers in the result vector.. none of this is
  // terribly efficient, but scalers don't get read that often (at least that's the
  // theory).
  
  result.assign(pBuffer, pBuffer + numRead*sizeof(uint16_t)/sizeof(uint32_t));


  delete []pBuffer;
  return result;
}

/*!
 From the point of view of CScaler as practiced by the readout framework we 
 are not composite:
*/
bool
CScriptedScalers::isComposite() const
{
  return false;
}
/*
** delegate add a creator to the bundle:
*/
void
CScriptedScalers::addCreator(CModuleCreator& creator)
{
  m_pBundle->addCreator(creator);
}

/*!
  Stub for adding user written creators.  If the user derives from this class
  and overrides this function to register some of their own creators they
  can extend the set of modules known to the 'module' command.
*/
void
CScriptedScalers::addUserWrittenCreators()
{
}

/*
 *  Add the standard set of creators specifically creators for the V830/820 and 
 *  the SIS 3300:
 */
void 
CScriptedScalers::addStandardCreators()
{
  addCreator(*(new CCAENV830Creator));
  addCreator(*(new CSIS3300Creator));
}

/*
 * figure out what the full configuration file path is. This can be any of the
 * following in order:
 *  - Translation of the "SCALER_FILE" env variable.
 *  - ~/config/scalers.tcl
 *  - ./scalers.tcl
 * This is virtual so that it can be overidden in derive classes if none of this
 * is correct.
 *  @return string
 *  @throws CErrnoException for ENOENTRY if there is no matching file.
 */
string
CScriptedScalers::getConfigurationFile()
{
  // SCALER_FILE:

  const char* path = getenv("SCALER_FILE");
  if (path && (access(path, R_OK) == 0) ) {

    return string(path);
  }

  // ~/config/scalers.tcl

  const char* home = getenv("HOME");
  if (home) {
    string path(home);
    path += string("/config/scalers.tcl");
    if (access(path.c_str(), R_OK) == 0) {
      return path;
    }

  }

  //  cwd/scalers.tcl

  char here[PATH_MAX];
  char* pResult = getcwd(here, PATH_MAX);
  if (pResult) {
    string path(here);
    path += "/scalers.tcl";
    if (access(path.c_str(), R_OK) == 0) {
      return path;
    }
  }
  errno = ENOENT;
  throw CErrnoException("Locating a configuration file");

  

}
 
