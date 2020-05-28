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


 
// Class: CStringInteractor                     //ANSI C++
//
// Provides mechanisms to interact with a string.
// E.g. in a network connection where a password
// may have been sent as an initial record.
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved StringInteractor.h
//

#ifndef STRINGINTERACTOR_H  //Required for current class
#define STRINGINTERACTOR_H

#include "Interactor.h"
#include <string>        //Required for include files
                               
class CStringInteractor  : public CInteractor        
{                       
			
  std::string m_sString;		//
  int    m_nReadCursor;	//Char offset for read.        

protected:

public:

   // Constructors and other cannonical operations:

  ~ CStringInteractor ( )  // Destructor 
  { }  
  CStringInteractor (const std::string& am_sString  ) 
    : CInteractor(),
      m_sString(am_sString),
      m_nReadCursor(0)
  { 
    
  }
  
  //Copy constructor 
  
  CStringInteractor (const CStringInteractor& aCStringInteractor )   : 
    CInteractor (aCStringInteractor) 
  {
    
    m_sString = aCStringInteractor.m_sString;
    m_nReadCursor = aCStringInteractor.m_nReadCursor;
    
  }                                     
  
// Operator= Assignment Operator 

  CStringInteractor& operator= (const CStringInteractor& aCStringInteractor) {
    CInteractor::operator=(aCStringInteractor);
    m_sString     = aCStringInteractor.m_sString;
    m_nReadCursor = aCStringInteractor.m_nReadCursor;
    return *this;
  }
 
   //Operator== Equality Operator 
private:
  int operator== (const CStringInteractor& aCStringInteractor) const;
public:
  // Selectors:

public:

  std::string getString() const
  { return m_sString;
  }
  int getReadCursor() const
  { return m_nReadCursor;
  }
                       
// Mutators:

protected:

	void setString (const std::string& am_sString)
  { m_sString = am_sString;
  }
  void setReadCursor (const int am_nReadCursor)
  { m_nReadCursor = am_nReadCursor;
  }
  // Operations:
public:
  void Rewind() {
    m_nReadCursor = 0;
  }
 virtual   int Read (unsigned int nBytes, void* pBuffer)    ;

 virtual   int Write (unsigned int nBytes, void* pBuffer)    ;
 
protected:

private:

};

#endif
