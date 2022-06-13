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
#include <CScriptedSegment.h>

#include <TCLInterpreter.h>
#include <RangeError.h>
#include <ErrnoException.h>
#include <TCLException.h>
#include "CDigitizerDictionary.h"
#include "CModuleCommand.h"
#include "CReadOrder.h"
#include "ScriptedBundle.h"
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <io.h>


// Standard creators:


#include "CCAENV775Creator.h"
#include "CCAENV785Creator.h"
#include "CCAENV792Creator.h"
#include "CCAENV830Creator.h"
#include "CPacketCreator.h"
#include "CSIS3300Creator.h"
#include "CV1x90Creator.h"

#ifndef PATH_SEP
#define PATH_SEP '/'
#endif


using namespace std;
///////////////////////////////////////////////////////////////
//
// Implementation of the public interface
//
//////////////////////////////////////////////////////////////
/*!
    Constructor initializes all the member data so that it is clear
    we don't have any read orders. 

*/
CScriptedSegment::CScriptedSegment() :
  m_pBundle(0),
  m_pInterp(0)
{}

/*!
   Initialization in our case involves
   - Deleting the old infrastructure and creating new.
   - Adding the supported modules to the new infrastructure.
   - Getting the configuration filename.
   - Getting and processing the configuration file.

*/
void
CScriptedSegment::initialize()
{
  delete m_pBundle;
  delete m_pInterp;

  // Create the new infrastructure:


  m_pInterp        = new CTCLInterpreter();
  m_pBundle        = new ScriptedBundle(m_pInterp);
  

  // Add standard and user written module creators.

  addStandardCreators();
  addUserWrittenCreators();

  // figure out which file is the configuration file and process it.

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
    reportConfigFileFailure(
        msg.c_str(), CTCLException::getTraceback(*m_pInterp).c_str()
    );
    
  }
  catch(const char* msg) {
    reportConfigFileFailure(
      msg, CTCLException::getTraceback(*m_pInterp).c_str()
    );
    
  }
  catch(CTCLException& tclfail) {
    reportConfigFileFailure(
      tclfail.ReasonText(), tclfail.getTraceback().c_str()
    );
    
  }
  catch (...) {
    throw std::string("Configuration file processing failed");
  }
  
}
/*!
   After reading an event, the digitizers must be cleared and
   made ready for the next event:
*/
void
CScriptedSegment::clear()
{
  m_pBundle->clear();
}
/*!
   disable the bundle.
*/
void
CScriptedSegment::disable()
{
  m_pBundle->disable();
}
/*!
   To read we are just going to delegate to the read order:
   @param pBuffer  - Pointer tothe buffer into which the data will be put.
   @param maxwords - Maximum number of words we're allowed to put in the buffer.

   @return size_t
   @retval number of words actually read.

   @throws CRangeError (if we live long enough) if the number of words read 
                        exceeds maxwords.. though in that case it's very possible
                        we'll SEGFAULT first.
*/
size_t
CScriptedSegment::read(void* pBuffer, size_t maxwords)
{
  int nWords = m_pBundle->m_pReadOrder->Read(pBuffer);
  if (nWords > maxwords) {
    throw CRangeError(0, maxwords, nWords,
		      "Overflowed the event buffer in an event read");
  }
  return nWords;
}
///////////////////////////////////////////////////////////////////////
//
// Implementation of the protected interface.
// Note that some entries are documented for doxygen for those who
// may extend this class.
//


/*!
   Returns the name of the configuration file as an STL std::string.
   in the default implementation, this is the first of the following
   to have a readable file:
   HARDWARE_FILE            environment variable
   ~/config/hardware.tcl    where ~ comes from the HOME env name.
   ./hardware.tcl           where . is the cwd.

   @return std::string
   @throws CErrnoException for ENOENTRY if there is no a matching file.
   @note daqdev/NSCLDAQ#700 This has had the penguin snot factored out of it.

*/
string
CScriptedSegment::getConfigurationFile()
{
  return locateConfigFile("HARDWARE_FILE", "hardware.tcl");

}

/*!
   Add a new module creator this is the mechanism a user written addUserWrittenCreators
   would use to add creators it instantiated into the set supported by the module
   command (For that matter, addStandardCrators uses it too).
   @param type    - type of module to add.
   @param creator - reference to the module creator to add.

*/
void
CScriptedSegment::addCreator(const char* type, CModuleCreator& creator)
{
  m_pBundle->addCreator(type, creator);
}

/*!
   Stub for addUserWrittenCreators... in order to extend the set of
   creators supported by the module command, simply derive a class from this
   class, override this addUserWrittenCreators to add support for the new modules,
   and in Skeleton.cpp instantiate your new class and register it as the
   event segment to read out.
*/
void
CScriptedSegment::addUserWrittenCreators()
{
}

/*!

  Add the standard creators to the module command (via addCreator):
  - CCAENV775Creator  - Creates CAEN V775 TDC modules.
  - CCAENV785Creator  - Creates CAEN V785 ADC modules.
  - CCAENV792Creator  - Creates CAEN V792 QDC modules (works for the 892)
  - CCAENV830Creator  - Creates CAEN V830 scaler modules.
  - CSIS3300Creator   - Creates SIS 3300 FADC mdoules.
  - CV1x90Creator     - Creates a CAEN V1190/1290 multihit tdc module.
  - CPacketCreator    - Creates a packet ..which holds other modules.

*/
void
CScriptedSegment::addStandardCreators()
{
  // All creators are dynamically created ..presumably destruction of the
  // CModuleCommand will delete them ??!?

  addCreator("caenv775", *(new CCAENV775Creator()));
  addCreator("caenv785", *(new CCAENV785Creator()));
  addCreator("caenv792", *(new CCAENV792Creator()));
  addCreator("caenv830", *(new CCAENV830Creator()));
  addCreator("sis3300", *(new CSIS3300Creator()));
  addCreator("caenv1x90", *(new CV1x90Creator()));

}

/**
 * reportConfigFileFailure
 *   Formats and throws a string exception that indicates configuration
 *   file processing failed:
 * @param why - reason for the failure.
 * @param where - traceback string that says where.
 * @throw std::string
 */
void
CScriptedSegment::reportConfigFileFailure(const char* why, const char* where)
{
  std::string msg("Configuration file processing failed:\n");
  msg   += why;
  msg   += "\nAt: \n";
  msg   += where;
  throw msg;

}
/**
 * locateConfigFile
 *    Locate a configuration file according to the search rules used by
 *    this program.  This can also be used to locate scaler files.
 *    hence it is public and static:
 * @param envvar - environment variable that can hold the config file name.
 * @param name   - Name of the file exclusive of any path.
 *
 * @return std::string - path to file.
 * @throw std::CErrnoException with ENOENT if not found.\
 * @note search order is envvar, $HOME/config/name, `pwd`/name.
 */
std::string
CScriptedSegment::locateConfigFile(const char* envvar, const char* name)
{
  // SCALER_FILE:

  std::string result = io::getReadableEnvFile(envvar);
  if (result != "") return result;

  // ~/config/scalers.tcl

  result = "/config/";
  result            += name;
  result = io::getReadableFileFromHome(name);
  if (result != "") return result;
  

  //  cwd/scalers.tcl

  result = io::getReadableFileFromWd(name);
  if (result != "") return result;
  
  errno = ENOENT;
  throw CErrnoException("Locating a configuration file");  
}

