/*
     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#include <config.h>
#include <CMonCommand.h>
#include "CCtlConfiguration.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include <CControlHardware.h>



using namespace std;

/**
 * Construct the command object.  This is
 * given the command name 'mon'.
 * @param interp - Tcl interpretr on which we will
 *                  be registered.
 * @param server - Reference to the Tcl server
 *                 needed in order to get the driver
 *                 instances.
 */ 
CMonCommand::CMonCommand(CTCLInterpreter&   interp,
                         CCtlConfiguration& configuration) :
  m_configuration(configuration),
  CTCLObjectProcessor(interp, "mon", true)
{}
/**
 * Destructor: In fact destruction never actually
 * happens...if it did the parent destructor
 * takes care of everything we need.
 */
CMonCommand::~CMonCommand()
{}

/*--------------------------------------------------*/

/** Command entry point.  The form of the command is
\verbatim
mon instance-name
\endverbatim
*   Where instance-name is the name of a device.
*   the command will return a string result that is
*   driver dependent in form and content.
* @param interp - Interpreter running this command
* @param objv   - Encapsulated Tcl_Obj's that make up
*                 the command line.
* @return int
* @retval TCL_OK - Command completed successfully.
* @retval TCL_ERROR - Command failed.
*
* @note The intepreter result will contain the
*       data from the driver on success.
*       contents on failure are likely an error message.
*/
int
CMonCommand::operator()(CTCLInterpreter& interp,
		                   	vector<CTCLObject>& objv)
{
  // Must have exactly two command words:

  if (objv.size() != 2) {
    interp.setResult("ERROR - mon command incorrect # params");
    return TCL_ERROR;
  }

  // extract the driver instance name and attempt
  // to get it:

  string instanceName       = static_cast<string>(objv[1]);
  CControlModule* pInstance = m_configuration.findModule(instanceName);
  if (!pInstance) {
    interp.setResult("ERROR - mon - unknown driver instance");
    return TCL_ERROR;
  }

  // At this point nothing can go wrong except
  // exceptions thrown by the driver:

  try  {
    interp.setResult(pInstance->getMonitoredData());
  }
  catch(...) {
    interp.setResult("ERROR - mon - driver threw an exception");
    return TCL_ERROR;
  }
  return TCL_OK;
}
