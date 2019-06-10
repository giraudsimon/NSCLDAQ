   

//! \class: CDAQStart           
//! \file:  .h
/*!
  \class CDAQStart
  \file  .h

  The DAQStart main application class. We make all
  of our applications a class that is invoked from main.
  This makes it easier to re-use program logic sections for
  other applications as well as combining several singly 
  threaded applications into a multithreade app.
  

*/

// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef __CDAQSTART_H  //Required for current class
#define __CDAQSTART_H

//
// Include files:
//

#ifndef CMDLINE_H
#include "cmdline.h"		// From gengetopt.
#ifndef  CMDLINE_H		// Just in case they change their gaurd.
#define  CMDLINE_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


                               //Required for 1:1 association classes

// Forward class definitions (convert to includes if required):

class CMonitoredProgram;
class CSink;

class CDAQStart      
{
private:
  
  // Private Member data:
  
  CMonitoredProgram* m_pProgram; // Pointer to monitored program
  bool               m_fNotifyExit; // Notify on exit
  int                m_nArgc;
  char**             m_pArgv;

public:
  //  Constructors and other canonical operations.
  //  You may need to adjust the parameters
  //  and the visibility esp. if you cannot
  // implement assignment/copy construction.
  // safely.
  CDAQStart ();		   	                //!< Constructor.
  virtual  ~ CDAQStart ( );             //!< Destructor.
private:
  CDAQStart (const CDAQStart& rSource );        //!< Copy construction.
  CDAQStart& operator= (const CDAQStart& rhs); //!< Assignment.
  int operator== (const CDAQStart& rhs) const; //!< == comparison.
  int operator!= (const CDAQStart& rhs) const; //!< != comparison.
public:
  

public:

  virtual   int operator() (int argc , char** argv)   ; 


  // Utilities:

protected:
  void Usage ()   ; 
  gengetopt_args_info* ParseArgs (int argc, char** argv)   ; 
  virtual   int MainLoop ()   ; 
  void StartSubprocess ()   ; 
  void MonitorOutput (int nMs)   ; 
  bool MonitorExit ()   ;
  void ReportExit();
private:
  char* SinkType(char* name);
  char* SinkName(char* name);

  CSink* CreateSink(const char* name);
  
  
};

#endif
