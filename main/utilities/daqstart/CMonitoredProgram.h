// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 
/*!
  \class CMonitoredProgram

  This abstract base class provides the interface
  to produce an monitor programs.  Monitored programs
  are started with the same stdin as the running program but
  stdout and stderr connected ultimately to pipes that can be
  monitored by this object.  Each Monitored program also 
  maintains a pair of CSink derived objects that are used
   to log the output and potentially the errors from the program.

*/



#ifndef CMONITOREDPROGRAM_H  //Required for current class
#define CMONITOREDPROGRAM_H

// Headers:

#include <string>

// Forward class references:
class CSink;


class CMonitoredProgram      
{
private:
  
  // Private Member data:

    int    m_nExitStatus;   //!< Program final exit value.  
    std::string m_sProgramName;  //!< Program name string.   
    int    m_nArgc;         //!< Number of command line parameters (including program name).  
    char** m_paArgv;        //!< The command line parameters.  

    CSink* m_pOutput;       //!< Sink for stdout logging.
    CSink* m_pError;        //!< Sink for stderr logging.
    
 protected:
    // Protected member data.
    
 public:
    // Public member data.
    
 public:
    //  Constructors and other canonical operations.
    //  You may need to adjust the parameters
    //  and the visibility esp. if you cannot
    // implement assignment/copy construction.
    // safely.
    CMonitoredProgram (int    argc,
		       char** argv);//!< Constructor.
    virtual  ~CMonitoredProgram ( );    //!< Destructor.
    CMonitoredProgram (const CMonitoredProgram& rSource ); //!< Copy construction.
    CMonitoredProgram& operator= (const CMonitoredProgram& rhs); //!< Assignment.
    int operator== (const CMonitoredProgram& rhs) const; //!< == comparison.
    int operator!= (const CMonitoredProgram& rhs) const { //!< != comparison.
      return !(operator==(rhs));
    }

    // Selectors (essentially):
 public:
    std::string GetName        ()   const; 
    int    GetNargs       ()   const; 
    char** GetArgs        ()   const;
    int    GetFinalStatus ()   const; 

 

    // Mutators (essentially)

public:
    CSink* AttachOutput   (CSink* pSink)   ; 
    CSink* AttachError    (CSink* pSink)   ; 
protected:
    void   SetFinalStatus (int nStatus)   ; 
    
 public:
    
    virtual   void Start      ()          = 0 ; 
    virtual   void PollIO     (int nMs)   = 0 ; 
    virtual   bool PollStatus ()          = 0 ; 
    void StdOut (std::string line)   ; 
    void StdErr (std::string line)   ; 


    // Private utilities.
 private:
    void CopyArgv(int argc, char**argv);
    void FreeArgv();
    void CopyIn(const CMonitoredProgram& rhs);
    bool ArgvSame(int argc, char**argv) const;
    void Log(int fd, CSink* pSink, std::string sLine);

};



#endif
