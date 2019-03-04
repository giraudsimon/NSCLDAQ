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

static const char* Copyright= "(C) Copyright Michigan State University 2002, All rights reserved";
///////////////////////////////////////////////////////////////////////
//
// serverinstnace.cpp:
//    Implements the CServerInstance class of the 
//    tcl server.
//    One trick is used in this class to get it to self destruct.
//    Disconnections are detected in a static member function which,
//    on acceptance of a chunk of data passes control to appropriate
//    members of the actual class (passed by pointer as client data) and,
//    if the connection was broken, assumes the client was dynamically
//    allocated and deletes it.
//
#include <config.h>
#include "serverinstance.h"
#include <string.h>
#include <tcl.h>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <iostream>
#include <string>
#include <os.h>


using namespace std;


const char *eol = "\n";

//////////////////////////////////////////////////////////////////////////
//
// CServerInstance(ServerContext& rContext)
//
// Constructor:
//
CServerInstance::CServerInstance(ServerContext& rContext) :
  m_authenticated(true)		// no username authentication needed.
{
  Context.pInterp       = rContext.pInterp;
  Context.RemotePort    = rContext.RemotePort;
  Context.DialogChannel = rContext.DialogChannel;

  Tcl_DStringInit(&(Context.command));
  Tcl_DStringInit(&(Context.RemoteHost));
  Tcl_DStringAppend(&(Context.RemoteHost), 
		    Tcl_DStringValue(&(rContext.RemoteHost)), -1);

  Tcl_CreateChannelHandler(Context.DialogChannel, 
			   TCL_READABLE | TCL_EXCEPTION,
			   CServerInstance::InputHandler,
			   (ClientData)this);
}
///////////////////////////////////////////////////////////////////////
//
//  ~CServerInstance()
// 
// Destructor:
//
CServerInstance::~CServerInstance()
{
  Shutdown();
}
/////////////////////////////////////////////////////////////////////
//
// void ClearCommand()
//
// Function Type:
//   Utilty (command string manipulation).
//
void
CServerInstance::ClearCommand()
{
  Tcl_DStringFree(&(Context.command));
  Tcl_DStringInit(&(Context.command));
}
/////////////////////////////////////////////////////////////////////
//
//   void AppendChunk(const char* pChunk)
//
// Operation Type:
//   Utility - command manipulation
//
void
CServerInstance::AppendChunk(const char* pChunk)
{
  Tcl_DStringAppend(&(Context.command), pChunk, -1);
}
/////////////////////////////////////////////////////////////////////////
//
// virtual int  isChunkCommand()
//
// Operation Type:
//   Utility, overridable, command manipulation.
//
int
CServerInstance::isChunkCommand()
{
  return Tcl_CommandComplete(Tcl_DStringValue(&(Context.command)));
}

/////////////////////////////////////////////////////////////////////////
//
// virtual int OnCommand()
//
// Operation Type:
//    Overridable, action
//
int
CServerInstance::OnCommand()
{
  // A complete command has been received.  Default action
  // is to get Tcl/Tk to interpret it.  Errors are reported via OnError.
  //

  int status;

  if (m_authenticated) {
    status = Tcl_Eval(Context.pInterp, 
		      Tcl_DStringValue(&(Context.command)));
    Tcl_Write(Context.DialogChannel, Tcl_GetStringResult(Context.pInterp),
	      strlen(Tcl_GetStringResult(Context.pInterp)));
    Tcl_Write(Context.DialogChannel, eol, strlen(eol));
    Tcl_Flush(Context.DialogChannel);
    
    if(status != TCL_OK) {
      OnError(status);
    }
    
    ClearCommand();
    return status;
  } 
  else {
    // The first command like string should be an authentication string
    // which consists of the name of the user connecting. This should match
    // our username.. this is intended to prevent port confusion, rather than
    // being a strict security check.
    
    // Need to strip the last char off the line since it's a \n'.

    int len = Tcl_DStringLength(&(Context.command)) - 1;
    Tcl_DStringTrunc(&(Context.command), len);
    
    std::string username = Os::whoami();

    if (strcmp(Tcl_DStringValue(&(Context.command)), username.c_str()) == 0) {
      // Authenticated.

      m_authenticated = true;
      ClearCommand();
      return TCL_OK;
    }
    else {
      // Bad auth string.

      cerr << "Bad authentication string: '" 
	   << Tcl_DStringValue(&(Context.command)) << "'\n";
      Shutdown();
      return TCL_OK;
    }
  }
}
////////////////////////////////////////////////////////////////////////
//
// virtual void OnError(int nError)
//
// Operation Type:
//   Overridable, Action.
//
void
CServerInstance::OnError(int nError)
{
  return;
#ifdef NOTDEFINED
  cerr << "Error detected in client script command: from: "
       << "Host: " << Tcl_DStringValue(&(Context.RemoteHost))
       << "\nPort: " << Context.RemotePort 
       << "\nCommand: '"
       << Tcl_DStringValue(&(Context.command))
       << "'\n";
  cerr << "Tcl Error message was: '"
       << Context.pInterp->result
       << "'";
#endif

}
////////////////////////////////////////////////////////////////////////
//
// virtual void OnShutdown()
// 
// Operation Type:
//   Overridable Action:
void
CServerInstance::OnShutdown()
{
  cerr << "Shutting down connection to: " 
       << "Host: " << Tcl_DStringValue(&(Context.RemoteHost))
       << "\nPort: " << Context.RemotePort 
       << endl;
} 


///////////////////////////////////////////////////////////////////////
//
// void Shutdown()
//
// Operation Type:
//   Cleanup.
//
void
CServerInstance::Shutdown()
{
  OnShutdown();
  Tcl_DStringFree(&(Context.RemoteHost));
  Tcl_DStringFree(&(Context.command));
 

  Tcl_DeleteChannelHandler(Context.DialogChannel, 
			   CServerInstance::InputHandler, (ClientData)this);
  Tcl_Close(Context.pInterp, Context.DialogChannel);
}
///////////////////////////////////////////////////////////////////////////
//
//  Tcl_DString GetChunk()
//
// OPeration type:
//   Overidable utility.
//
Tcl_DString*
CServerInstance::GetChunk()
{
  Tcl_Channel  chan     = Context.DialogChannel;
  if(!Tcl_Eof(chan)) {
    Tcl_DString& str =*(new Tcl_DString);
    Tcl_DStringInit(&str);
    Tcl_Gets(chan, &str);
    Tcl_DStringAppend(&str, eol, -1);
    return &str;		// Needs to be deleted by caller.
  }
  return (Tcl_DString*)NULL;

}
//////////////////////////////////////////////////////////////////////
//
// static void InputHandler(ClientData cd, int mask)
//
// Operation Type:
//   Callback relay.
//
void
CServerInstance::InputHandler(ClientData cd, int mask)
{
  CServerInstance* pObject = (CServerInstance*)cd;

  if(mask & TCL_EXCEPTION) {	// Exceptions are treated like eof.
    pObject->OnError(TCL_EXCEPTION);
    delete pObject;		// Committ suicide
  }
  if(mask & TCL_READABLE) {	// Input's available.
    Tcl_Channel  Chan    = pObject->Context.DialogChannel;
    Tcl_DString* pString = &(pObject->Context.command);
    Tcl_Interp*  pInterp = pObject->Context.pInterp;
    if(!Tcl_Eof(Chan)) {	// Add command chunks.
      Tcl_DString* pChunk = pObject->GetChunk();
      if(pChunk) {
	pObject->AppendChunk(*pChunk);
	if(pObject->isChunkCommand()) {
	  pObject->OnCommand();
	}
	Tcl_DStringFree(pChunk);
	delete pChunk;
      }
    }
    if(Tcl_Eof(Chan)) {		// EOF at end of all this kills self.
      delete pObject;		// Calls OnShutdown etc.
    }
  }
}
