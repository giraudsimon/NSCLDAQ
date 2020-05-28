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

#ifndef SERVERINSTANCE_H
#define SERVERINSTANCE_H


#include "server.h"
#include <string>


class CServerInstance {		// Connection instance.
  ServerContext Context;
  bool          m_authenticated;
public:
  CServerInstance(ServerContext& rContext);

private:
  CServerInstance(const CServerInstance& rInstance); // No copy construct.
  CServerInstance&
  operator=(const CServerInstance& rInstance) const; // no assignment.
public:

  virtual ~CServerInstance();

  // Maninpulate the command string:

  void ClearCommand();		// Empty the command string.
  void AppendChunk(const char* pChunk);	// Add a substring.
  void AppendChunk(const std::string& rChunk) { 
    AppendChunk(rChunk.c_str());
  }
  void AppendChunk(const Tcl_DString& str) {
    AppendChunk(Tcl_DStringValue(&str));
  }
  virtual int  isChunkCommand(); // Check command for completeness.

  // Things to do with complete commands:

  virtual int OnCommand();		// Execute complete command.
  virtual void OnError(int nError);     // Execute on command error.
  virtual void OnShutdown();	        // Execute when shutting down. 
protected:
  // Utilities:

  void Shutdown();
  virtual Tcl_DString* GetChunk(); // delete pointer in caller.

  // Callback relays.

  static void InputHandler(ClientData cd, int mask); // Input chunking.

};
#endif
