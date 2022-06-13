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
// Class: CFdInteractor                     //ANSI C++
//
// Represents an interactor where data is exchanged
// across an entity which can be represented by a 
// single file descriptor... e.g. a pipe.
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved FdInteractor.h
//

#ifndef FDINTERACTOR_H  //Required for current class
#define FDINTERACTOR_H

                               //Required for base classes
#include "Interactor.h"
#include <unistd.h>

                               
class CFdInteractor  : public CInteractor        
{                       
			
   int m_nFd;			//File Id connecting to peer.        

protected:

public:

   // Constructors and other cannonical operations:

  CFdInteractor (int fd)    : m_nFd(fd) 
  { 
  } 
  ~ CFdInteractor ( )  // Destructor 
  {
    close(m_nFd);
  }  
   //Copy constructor 

  CFdInteractor (const CFdInteractor& aCFdInteractor )   : 
    CInteractor (aCFdInteractor) ,
    m_nFd(dup(aCFdInteractor.m_nFd))
  { 
  }                                     
   // Operator= Assignment Operator 

  CFdInteractor& operator= (const CFdInteractor& aCFdInteractor) {
    m_nFd = dup(aCFdInteractor.m_nFd);
    CInteractor::operator=(aCFdInteractor);
    return *this;
  }
 
   //Operator== Equality Operator 
private:
  int operator== (const CFdInteractor& aCFdInteractor) const;
public:
// Selectors:

public:

  int getFd() const
  { 
    return m_nFd;
  }
                       
// Mutators:

protected:

  void setFd (const int am_nFd)
  { 
    m_nFd = am_nFd;
  }
       
public:

 virtual   int Read (unsigned int nBytes, void* pBuffer)    ;
 virtual   int Write (unsigned int nBytes, void* pData)    ;
 virtual   void Flush ()    ;


};

#endif
