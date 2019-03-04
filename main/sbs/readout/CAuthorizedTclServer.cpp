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
#include "CAuthorizedTclServer.h"

#include <TCLInterpreter.h>
#include "CTCLAuthorizer.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////////////////

/*!
 Construction does 90% of the work.. In addtion to base class construction,
we'll build an authorizer which will register the serverauth command.
\param pInterp  - Pointer to the interpreter object that will be the targe
                  of client commands as well as in which the serverauth command is installed.
\param port     - Port on which the server will listen for connections.

*/
CAuthorizedTclServer::CAuthorizedTclServer(CTCLInterpreter* pInterp, int port) :
  CTCLServer(pInterp, port),
  m_pAuthorizer(new CTCLAuthorizer(pInterp))
{}

/*!
  Destruction will also destroy the server auth command.
*/
CAuthorizedTclServer::~CAuthorizedTclServer()
{
  delete m_pAuthorizer;
}


////////////////////////////////////////////////////////////////////////////////////////////

/*!
  allow connection will
  - return true if the base class one will.
  - return the results of consulting the authorizer, if the base class did not return true.

  \param connection - Tcl_Channel open to the client.
  \param hostname   - host to check.
 
*/
bool
CAuthorizedTclServer::allowConnection(Tcl_Channel connection, string hostname)
{
  if (CTCLServer::allowConnection(connection, hostname)) return true;

  return m_pAuthorizer->Authenticate(hostname);
}
