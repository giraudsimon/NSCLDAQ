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

// Class: CVMEMapCommand                     //ANSI C++

#ifndef CVMEMAPCOMMAND_H  //Required for current class
#define CVMEMAPCOMMAND_H

                               //Required for base classes
#include <TCLProcessor.h>
#include <VmeModule.h>

                         
class CVMEMapCommand  : public CTCLProcessor        
{ 
  uint32_t            m_nPhysBase;  //!< VME  base address of map.
  uint32_t            m_nSize;      //!< Size of segment in bytes.
  uint32_t            m_nCrate;      //!< Crate segment lives in.
  CVmeModule*         m_pSpace;     //!< Accessor for the space.
  std::string         m_Name;       //!< Name of the space.

 
protected:
  
  int Get (CTCLInterpreter& rInterp, CTCLResult& rResult, int nSize,
	   int nArgs, char** pArgs)   ;
  int Set (CTCLInterpreter& rInterp, CTCLResult& rResult, int nSize,
	   int  nArgs, char** pArgs)    ;
  int Usage(CTCLResult& rResult);

public:
  // Constructor.

  CVMEMapCommand ( std::string      Name, 
		   CTCLInterpreter* pInterp,
		   uint32_t         Base, 
		   uint32_t         nSize,
		   uint32_t         nCrate,
		   CVmeModule::Space space);
   ~ CVMEMapCommand ( ); 

   /// Selectors:

   uint32_t getBase() const {
     return m_nPhysBase;
   }
  
public:       
  virtual   int operator() (CTCLInterpreter& rInterp, 
			    CTCLResult& rResult, int nArgs, char** pArgs);
  virtual void OnDelete();

};

#endif
