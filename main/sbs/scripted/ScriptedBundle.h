#ifndef SCRIPTEDBUNDLE_H
#define SCRIPTEDBUNDLE_H
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

class CTCLInterpreter;
class CDigitizerDictionary;
class CReadOrder;
class CModuleCommand;
class CModuleCreator;

/*!
   This class contains the common data members and common code for the
   CScriptedSegment and CScriptedScaler classes.  The intent is that
   while the API is available each class can access the individual data members
   for application specific purposes.
*/
struct ScriptedBundle {
  CTCLInterpreter*        m_pInterp;
  CDigitizerDictionary*   m_pDictionary;
  CReadOrder*             m_pReadOrder;
  CModuleCommand*         m_pModuleCommand;

  ScriptedBundle(CTCLInterpreter* pInterp);
  ~ScriptedBundle();

  // These can be 100% common delegations from both users we know about

  void initialize();
  void clear();
  void disable();

  // Stuff to set up the bundle:

  void processHardwareFile(const char* pFilename, CTCLInterpreter& rInterp);
  void addCreator(const char* type, CModuleCreator& rCreator);


};
#endif
