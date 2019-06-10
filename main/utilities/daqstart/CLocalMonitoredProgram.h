// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

/*!
  \class CLocalMonitoredProgram
  \file  .h

  This class starts a subprocess in the local system
  and provides hooks to monitor it.  The subprocess
  is started on the ends of pipes (pipes for stdout
  and stderr). 

*/


#ifndef __CLOCALMONITOREDPROGRAM_H  //Required for current class
#define __CLOCALMONITOREDPROGRAM_H

//
// Include files:
//

                               //Required for base classes
#ifndef __CMONITOREDPROGRAM_H     //CMonitoredProgram
#include "CMonitoredProgram.h"
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __CRTL_SYS_TYPES_H		// All this just to get pid_t.
#include <sys/types.h>
#ifndef __CRTL_SYS_TYPES_H
#define __CRTL_SYS_TYPES_H
#endif
#endif
 
class CLocalMonitoredProgram  : public CMonitoredProgram        
{
private:
  
  // Private Member data:
  int    m_nStdout;      //!< Output end of program's stdout pipe.  
  int    m_nStderr;      //!< Output end of program's stderr pipe.  
  bool   m_fRunning;	 //!< True if the process is running.
  pid_t  m_nPid;         //!<  PID of child process.  
  std::string m_sStdoutLine;  //!<  The accumluating line for stdout.  
  std::string m_sStderrLine;  //!<  The accumulating line for stderr.  
  
public:
  //  Constructors and other canonical operations.
  //  You may need to adjust the parameters
  //  and the visibility esp. if you cannot
  // implement assignment/copy construction.
  // safely.
  CLocalMonitoredProgram (int argc, char** argv); //!< Constructor.
  virtual  ~CLocalMonitoredProgram ( );           //!< Destructor.

  // I'm not sure how to make the various copy operations work, this
  // also implies I'm not sure what equality means.  Therefore:
  // These canonicals are illegal/unimplemented.
private:
  CLocalMonitoredProgram (const CLocalMonitoredProgram& rSource ); //!< Copy construction.
  CLocalMonitoredProgram& operator= (const CLocalMonitoredProgram& rhs); //!< Assignment.
  int operator== (const CLocalMonitoredProgram& rhs) const; //!< == comparison.
  int operator!= (const CLocalMonitoredProgram& rhs) const;
public:  
  // Insert any selectors you wish to export here
  // visibility should be public.
  
  // Insert any mutators you wish to export here.
  // visibility should be protected.
  
  // Class operations:
  
public:
  
  virtual   void Start      (); 
  virtual   void PollIO     (int nMs); 
  virtual   bool PollStatus (); 

  // Local utilities:

 private:
  static void BlockingOff(int nFd); //!< Turn off fd blocking.
  bool ProcessInputFd(int     nFd,
		      std::string& sOutline,
		      short   nPollFlags); //!< Process input on an fd.
};

#endif
