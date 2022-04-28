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
#include "CConfiguration.h"
#include "CStackCommand.h"

#include <CReadoutModule.h>
#include <TCLInterpreter.h>
#include <TCLObjectProcessor.h>
#include <Exception.h>
#include <CPH7xxCommand.h>
#include <CAD811Command.h>
#include <CLRS2551Command.h>
#include <CLRS2249Command.h>
#include <CLRS2228Command.h>
#include <C1205Command.h>
#include <CCAEN257Command.h>
#include <CPH7132Command.h>
#include <CCCUSBCommand.h>
#include <CAddTclDriver.h>
#include <CULMTrigger.h>
#include <CUserCommand.h>
#include <CMarkerCommand.h>
#include <iostream>
#include <CCCUSB.h>
#include <CCCUSBReadoutList.h>
#include <CCamacCompat.hpp>
#include <CLeCroy4300B.h>
//#include <CLeCroy4448.h>
#include <CLeCroy4434.h>
#include <CLeCroy2551.h>
#include <CCamacCrate.h>
#include <CMarkerCommand.h>
#include <tcl.h>
#include <algorithm>

using namespace std;

//////////////////////////////////////////////////////////////////////////
//////////////////////////// Canonicals //////////////////////////////////
//////////////////////////////////////////////////////////////////////////

/*!
  Construct the object.  This consists of 
  creating a new Tcl interpreter, wrapping into an object and
  the adding the commands to the interpreter needed to manage the
  configuration as well as saving those command processor objects for destruction
  lateron.
*/
CConfiguration::CConfiguration() :
  m_pInterp(0)
{
  Tcl_Interp* pInterp = Tcl_CreateInterp();
  Tcl_Init(pInterp);		// init the library search paths.
  m_pInterp = new CTCLInterpreter(pInterp);


  // Register and keep the commands.


  m_Commands.push_back(new CStackCommand(*m_pInterp, *this));
  m_Commands.push_back(new CPH7xxCommand(*m_pInterp, *this));
  m_Commands.push_back(new CAD811Command(*m_pInterp, *this));
  m_Commands.push_back(new CLRS2551Command(*m_pInterp, *this));
  m_Commands.push_back(new CLRS2249Command(*m_pInterp, *this));
  m_Commands.push_back(new CLRS2228Command(*m_pInterp, *this));
  m_Commands.push_back(new C1205Command(*m_pInterp, *this));
  m_Commands.push_back(new CCAEN257Command(*m_pInterp, *this));
  m_Commands.push_back(new CCCUSBCommand(*m_pInterp, *this));
  m_Commands.push_back(new CPH7132Command(*m_pInterp, *this));
  m_Commands.push_back(new CAddTclDriver(*m_pInterp, *this));
  m_Commands.push_back(new CMarkerCommand(*m_pInterp, *this));
  
  typedef CCCUSB Ctlr;
  typedef CCCUSBReadoutList RdoList;

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "CamacCrate", 
                            compat_clone(CCamacCrate<Ctlr,RdoList>())) );

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "ULMTrigger", 
                            compat_clone(CULMTrigger<CCCUSB,CCCUSBReadoutList>())) );

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "LeCroy4300B", 
                            compat_clone(CLeCroy4300B<Ctlr,RdoList>())) );

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "LeCroy4434", 
                            compat_clone(CLeCroy4434<Ctlr,RdoList>())) );

  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "LeCroy2551", 
                            compat_clone(CLeCroy2551<Ctlr,RdoList>())) );

//  m_Commands.push_back(new CUserCommand(*m_pInterp, *this, "LeCroy4448", 
//                            compat_clone(CLeCroy4448<Ctlr,RdoList>())) );
}
/*!
   Destruction must:
   - Destroy all the CReadoutModule object pointers that were added to m_Adcs
   - Destroy all the CReadoutModule object pointers that were added to m_Scalers.
   - Destroy all readout stacks.
   - Destroy all the CTCLObjectProcessors that were added to m_Commands.
   - Destroy m_pInterp which will implicitly invoke Tcl_DeleteInterp() on
     the interpreter it wraps.
*/
CConfiguration::~CConfiguration()
{
  for (int i = 0; i < m_Adcs.size(); i++) {
    delete m_Adcs[i];
  }
  for (int i = 0; i < m_Scalers.size(); i++) {
    delete m_Scalers[i];
  }
  for (int i = 0; i < m_Stacks.size(); i++) {
    delete m_Stacks[i];
  }
  for (int i = 0; i < m_Commands.size(); i++) {
    delete m_Commands[i];
  }

  delete m_pInterp;
}

////////////////////////////////////////////////////////////////////////////
//////////////////// Support for configuration /////////////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
   Process a configuration file.  Since configuration files are just
   Tcl scripts for our enhanced interpreter, we just EvalFile.
   errors are allowed to propagate upwareds.
   \param configFile : std::string
       Name of the configuration file to process.
*/
void
CConfiguration::processConfiguration(string configFile)
{
  try {
    m_pInterp->EvalFile(configFile);
  }
  catch (string msg) {
    cerr << "CConfiguration::processConfiguration caught string exception: "
	 << msg << endl;
    throw;
  }
  catch (char* msg) {
    cerr << "CConfiguration::processConfiguration caught char* exception: "
	 << msg << endl;
    throw;
  }
  catch (CException& error) {
    cerr << "CConfiguration::processConfiguration caught CExcpetion : "
	 << error.ReasonText() << " while " << error.WasDoing() << endl;
    cerr << Tcl_GetStringResult(m_pInterp->getInterpreter()) << endl;
    throw;
  }
  catch (...) {
    cerr << "CConfiguration::processConfiguration caught an unknown exception type\n";
    throw;
  }
}
/*!
   Locate an adc module by name.  This is used e.g. by configuration commands
   to prevent duplicat module creation and to locate a module to be configured.
   \param name : std::string
      Name of the module to locate.
   \return CReadoutModule*
   \retval NULL - Readout module not found.
   \retval other - A pointer to the CReadoutModule that was being looked for.

   \note - Since the configuration is likely only a few objects there's no
           need for anything fancier than a linear search.
*/
CReadoutModule*
CConfiguration::findAdc(string name)
{
  return find(m_Adcs, name);
}
/*!
   See findAdc above, but this version finds a scaler.
*/
CReadoutModule*
CConfiguration::findScaler(string name)
{
  return find(m_Scalers, name);
}
/*!
   See findAdc above, but this version finds a readout stack.
*/
CReadoutModule*
CConfiguration::findStack(string name)
{
  return find(m_Stacks, name);
}


/*!
   Add a new scaler module to the configuration.
   \param module : CReadoutModule
      Pointer to the new module.. Restrictions:
      - The module must have been dynamically created.
      - The module's storage ownership is passsed to this object and will be
        deleted by us when *this is destroyed.
*/
void
CConfiguration::addScaler(CReadoutModule* module)
{
  m_Scalers.push_back(module);
}

/*!
    Add a new adc module to the configuration.  See addScaler for
    description.
*/
void
CConfiguration::addAdc(CReadoutModule* module)
{
  m_Adcs.push_back(module);
}

/*!
   Adds a new stack to the list of stacks.  Note that there a ton
   of errors we could but don't check for:
   - There should not be duplicate stack numbers.
   - There should not be colliding vector/IPLS for triggered stacks etc.
*/
void
CConfiguration::addStack(CReadoutModule* module)
{
  m_Stacks.push_back(module);
}

/*!
  set the interpreter result.
  \param msg : std::string
*/
void
CConfiguration::setResult(string msg)
{
  Tcl_Obj* result = Tcl_NewStringObj(msg.c_str(), -1);
  Tcl_SetObjResult(m_pInterp->getInterpreter(), result);
}

////////////////////////////////////////////////////////////////////////////
/////////////////////// Support for config retrieval ///////////////////////
////////////////////////////////////////////////////////////////////////////

/*!
  Return the adc configuration vector (a copy of it).

*/
vector<CReadoutModule*>
CConfiguration::getAdcs()
{
  return m_Adcs;
}
/*!
  Return the Scaler configuration vector
*/
vector<CReadoutModule*>
CConfiguration::getScalers()
{
  return m_Scalers;
}

/*!
  Return the stack configuration vector
*/
vector<CReadoutModule*>
CConfiguration::getStacks()
{
  return m_Stacks;
}

////////////////////////////////////////////////////////////////////////////
////////////////////// Private utilities ///////////////////////////////////
////////////////////////////////////////////////////////////////////////////

CReadoutModule* 
CConfiguration::find(std::vector<CReadoutModule*>& modules,
		     std::string name)
{
  vector<CReadoutModule*>::iterator i = find_if(modules.begin(), modules.end(),
						MatchName(name));
  if(i != modules.end()) {
    return *i;
  } 
  else {
    return static_cast<CReadoutModule*>(NULL);
  }
}


bool
CConfiguration::MatchName::operator()(CReadoutModule* module)
{
  return (module->getName() == m_name);
}
