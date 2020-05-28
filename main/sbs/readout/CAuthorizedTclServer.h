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

#ifndef CAUTHORIZEDTCLSERVER_h
#define CAUTHORIZEDTCLSERVER_h


#include <CTCLServer.h>
#include <tcl.h>
#include <string>


// forward class definitions:

class CTCLInterpreter;
class CTCLAuthorizer;

/*!
   This class builds a Tcl server that does simple host based authorization
   instantiating this class adds the "serverauth" command which supports
   manipulating a list of hosts that are authorized to connect to the server.
   We always allow localhost to connect.

*/

class CAuthorizedTclServer : public CTCLServer
{
  // Member data:
private:
  CTCLAuthorizer*   m_pAuthorizer;

  // canonicals:
public:
  CAuthorizedTclServer(CTCLInterpreter* pPinterp, int port);
  virtual ~CAuthorizedTclServer();

private:
  CAuthorizedTclServer(const CAuthorizedTclServer&);
  CAuthorizedTclServer operator=(const CAuthorizedTclServer&);
  int operator==(const CAuthorizedTclServer&) const;
  int operator!=(const CAuthorizedTclServer&) const;
  
  // base class overrides:


protected:
  virtual bool allowConnection(Tcl_Channel connection, std::string hostname);
};

#endif
