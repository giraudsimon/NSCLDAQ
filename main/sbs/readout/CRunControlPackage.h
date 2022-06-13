#ifndef CRUNCONTROLPACKAGE_H
#define CRUNCONTROLPACKAGE_H
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
#include <list>
#include <TCLObjectPackage.h>

//

class RunState;
class CExperiment;
class CTCLInterpreter;
class RunTimer;

/*!
  This class is a container for run control operations.
  It provides services to the run control commands.  The
  idea is that the run control commands will do command syntax
  checking and verification, this class will do the actual work
  of manipulating the runs tate, and the experiment.

  This is a singleton class, however its instantiation must be
  done at initialization time in order to register the commands that it 
  supports.  These are:
  - begin  - Start a new run.
  - end    - End an active run.
  - pause  - Pause an active run.
  - resume - Resume an paused run.

*/
class CRunControlPackage  : public CTCLObjectPackage
{
  // Class level data:

private:
  static CRunControlPackage*  m_pInstance;

  // Object level data:

private:
  RunState*        m_pTheState;
  CExperiment*     m_pTheExperiment;
  CTCLInterpreter* m_pInterp;
  RunTimer*        m_pTimer;
  
  
  // Canonicals are all private so that this can enforce the singleton pattern.

private:
  CRunControlPackage(CTCLInterpreter& interp);
  ~CRunControlPackage();
  CRunControlPackage(const CRunControlPackage& rhs);
  CRunControlPackage& operator=(const CRunControlPackage& rhs);
  int operator==(const CRunControlPackage& rhs) const;
  int operator!=(const CRunControlPackage& rhs) const;

  // Completeing the singleton pattern:

public:
  static CRunControlPackage* getInstance(CTCLInterpreter& interp);

  // The following are useful functions both for the individual commands, and perhaps
  // for application extensions:
public:
  void begin();			// Begin a new run.
  void end();			// End an active run.
  void pause();			// Pause an active run.,
  void resume();		// Resume an active run.
  const RunState* getState() const; // Return the run state.

private:
  void createCommands(CTCLInterpreter& Interp);


};

#endif
