/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  TclRunInstance.pp
 *  @brief:  Implements run instances
 */

#include "TclRunInstance.h"
#include <TCLObject.h>
#include <TCLInterpreter.h>
#include "LogBookRun.h"
#include <stdexcept>
#include <sstream>
#include <Exception.h>


#include <sstream>

std::map<std::string, TclRunInstance*> TclRunInstance::m_instanceRegistry;

/**
 * constructor
 *    @param interp - intepreter on which the command will be registered.
 *    @param name   - command name.
 *    @param pRun   - run to wrap.
 */
TclRunInstance::TclRunInstance(
    CTCLInterpreter& interp, const char* name, LogBookRun* pRun
) :
    CTCLObjectProcessor(interp, name, true),
    m_pRun(pRun)
{
    m_instanceRegistry[name] = this;        
}

/**
 * destructor
 */
TclRunInstance::~TclRunInstance()
{
    delete m_pRun;
    auto p = m_instanceRegistry.find(getName());
    if (p != m_instanceRegistry.end()) {
        m_instanceRegistry.erase(p);
    }
}


/**
 * operator()
 *   Process commands.
 *
 * @param interp - interpreter on which we're running.
 * @param objv   - Command word objects.
 * @return int   - TCL_OK on success, TCL_ERROR on failure.
 */
int
TclRunInstance::operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv)
{
    try {
        bindAll(interp, objv);
        requireAtLeast(objv, 2, "Usage: <run-instance> <subcommand> ?...?");
        std::string subcommand(objv[1]);
        if (subcommand == "destroy" ) {
            delete this;
        } else {
            std::stringstream msg;
            msg << subcommand
                << " is not a valid subcommand of the run instance command "
                << std::string(objv[0]);
            std::string e(msg.str());
            throw e;
        }
    }
    catch (std::string& msg) {
        interp.setResult(msg);
        return TCL_ERROR;
    }
    catch (const char* msg) {
        interp.setResult(msg);
        return TCL_ERROR;
        
    }
    catch (std::exception& e) {       // Note LogBook::Exception is derived from this
        interp.setResult(e.what());
        return TCL_ERROR;
    }
    catch (CException& e) {
        interp.setResult(e.ReasonText());
        return TCL_ERROR;
    }
    catch (...) {
        interp.setResult(
            "Unexpected exception type caught in TclPersonInstance::operator()"
        );
        return TCL_ERROR;
    }
    return TCL_OK;
}
//////////////////////////////////////////////////////////////////////////////
// static methods

/**
 * getCommandObject
 *    Looks up a command instance object by command name
 * @param name -name of the command to lookup
 * @return TclRunInstance*  - pointer to the command object.
 * @throw std::out_of_range if there's no match.
 */
TclRunInstance*
TclRunInstance::getCommandObject(const std::string& name)
{
    auto p = m_instanceRegistry.find(name);
    if (p == m_instanceRegistry.end()) {
        std::stringstream msg;
        msg << "There is no run intance command object named: '"
            << name << "'";
        std::string e(msg.str());
        throw std::out_of_range(e);
    }
    return p->second;
}