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

// Class: CVmeCommand                     //ANSI C++

#ifndef VMECOMMAND_H  //Required for current class
#define VMECOMMAND_H

#include <iostream>                 
#include <TCLProcessor.h>
#include <VMEMapCommand.h>
#include <map>        //Required for include files
#include <string>        //Required for include files
#include <VmeModule.h>

class CVmeCommand  : public CTCLProcessor        
{                       
			
  std::map<std::string,CVMEMapCommand*>        m_Spaces;    //Attribute data member        

  enum Switches {		// switch values
    device,			// -device
    crate,			// -crate
    unknown };			// Anything else.
protected:

    int AddMap (CTCLInterpreter& rInterp, 
		CTCLResult& rResult, int nArgs, char** pArgs)   ;

    int ListMaps (CTCLInterpreter& rInterp, 
		  CTCLResult& rResult, int nArgs, char** pArgs)   ;

    int DeleteMap (CTCLInterpreter& rInterp, 
		   CTCLResult& rResult, int nArgs, char** pArgs)   ;

    int Usage(CTCLResult& rResult);

public:

  CVmeCommand (CTCLInterpreter* pInterp)    :
    CTCLProcessor("vme", pInterp)
  { 
  } 
  virtual ~ CVmeCommand ( );
				
  //Get accessor function for non-static attribute data member

  std::map<std::string,CVMEMapCommand*>& getm_Spaces()
  { 
    return m_Spaces;
  }
protected:
                       
  void setm_Spaces (const std::map<std::string,CVMEMapCommand*>& am_Spaces)
  { 
    m_Spaces = am_Spaces;
  }

public:
  virtual   int operator() (CTCLInterpreter& rInterpreter, 
			    CTCLResult& rResult, int nArgs, char** pArgs)    ;


  // Utility functions:
public:

  static unsigned long     TextToULong(const char* text);

private:
  // The functions below throw strings if they fail.
  //
  static  CVmeModule::Space  UserToLogical(const std::string& rsUser);
  static Switches MatchSwitch(const char* pSwitch);
};


#endif
