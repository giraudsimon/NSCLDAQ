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
// Class: CIOInteractor                     //ANSI C++
//
// Performs the duties of an interactor when there's
// a need for separate input and output connections.
// Typically, m_InputInteractor and m_OutputInteractor
// will be CFdInteractors, but there' s no pressing need
// to make that requirement so we don't.
//
// Author:
//     Ron Fox
//     NSCL
//     Michigan State University
//     East Lansing, MI 48824-1321
//     mailto: fox@nscl.msu.edu
// 
// (c) Copyright NSCL 1999, All rights reserved IOInteractor.h
//

#ifndef IOINTERACTOR_H  //Required for current class
#define IOINTERACTOR_H

                               //Required for 1:1 association classes
#include "Interactor.h"
#include "Interactor.h"
                                                               
class CIOInteractor : public CInteractor
{                       
			
   CInteractor* m_OutputInteractor; //1:1 association object data member
   CInteractor* m_InputInteractor; //1:1 association object data member      
			
public:

  virtual  ~ CIOInteractor ( )  // Destructor 
     { }  
  CIOInteractor (CInteractor& rInput, CInteractor& rOutput) 
  { 
    m_InputInteractor = &rInput;
    m_OutputInteractor= &rOutput;

  }      
  
   //Copy constructor  - would require all interactors to have copy construct.

private:
  CIOInteractor (const CIOInteractor& aCIOInteractor ) ;
  CIOInteractor& operator= (const CIOInteractor& aCIOInteractor);
public: 
                       
  CInteractor* getOutput() const
  { return m_OutputInteractor;
  }
  CInteractor* getInput() const
  { return m_InputInteractor;
  }
                       
// Mutators:

protected:
       
  void setOutput (CInteractor* am_OutputInteractor)
  { m_OutputInteractor = am_OutputInteractor;
  }
  void setInput (CInteractor* am_InputInteractor)
  { m_InputInteractor = am_InputInteractor;
  }

public:

 virtual   int Read (unsigned int nBytes, void* pBuffer)    ;
 virtual   int Write (unsigned int nBytes, void* pBuffer)    ;
 virtual   void Flush ()    ;
 
protected:

private:

};

#endif
