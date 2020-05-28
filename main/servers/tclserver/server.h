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

#ifndef SERVER_H
#define SERVER_H
#include <tk.h>

extern int serverport;
#define SERVERPORT serverport	/* Compatibility with 'test version'. */

typedef struct _ServerContext {
  Tcl_Interp*   pInterp;
  Tcl_DString   RemoteHost;
  int           RemotePort;
  Tcl_Channel   DialogChannel;
  Tcl_DString   command;
} ServerContext, *pServerContext;

void Server_Init(Tcl_Interp* pInterp,int SERVERPORT);
#endif











