
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

   
/*!
  \class CReadableObject
  \file CReadableObject.h
This class defines a readable object.  Readable
objects are are objects that can be stuck into a
CReadOrder object to be read out during an event.

The ReadableObjerct class:
- Provides interface definitions for the readout code
  that all readable objects must support and no-op'd
  default implementatiouns.
- Provides support for linking the object into a CReadOrder
  object and unlinking it later.
*/

// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef CREADABLEOBJECT_H  //Required for current class
#define CREADABLEOBJECT_H


//
// Include files:
//

                               //Required for base classes
#ifndef __CCONFIGURABLEOBJECT_H     //CConfigurableObject
#include "CConfigurableObject.h"
#endif

#ifdef HAVE_STD_NAMESPACE
using namespace std;
#endif


 
#ifndef __STL_STRING
#include <string>        //Required for include files  
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif



// Forward class definitions:

class CReadOrder;
class CTCLInterpreter;

class CReadableObject  : public SConfigurableObject       
{
private:
  
  // Private Member data:
  
  CReadOrder* m_pOwner;		//!< Pointer to our reader.

public:

  // Constructors and other canonical objects:
  //
  CReadableObject (const std::string& rName,
		   CTCLInterpreter& rInterp); //!< constructor.
 ~ CReadableObject ( ); 	//!< Destructor...


  // Copy construction etc. is illegal.
private:
  CReadableObject            (const CReadableObject& aCReadableObject );
  CReadableObject& operator= (const CReadableObject& aCReadableObject);
  int              operator==(const CReadableObject& aCReadableObject) const;
  int              operator!=(const CReadableObject& aCReadableObject) const;
public:

  // Class operations:

public:

  virtual   void   Initialize ()   = 0 ; //!< Initialize the readable (pure virt.).
  virtual   void   Prepare ()   = 0 ;    //!< Prepare module for readout (pure virt.). 

  virtual   int    Read (void*  pBuffer)   = 0 ; //!< Read to memory. (pure virt)
  virtual   void   Clear ()   = 0 ;      //!< Clear (after read e.g.).(Pure virt)
  void             Link (CReadOrder* pReader)   ; //!< Link to a reader/packet. 
  void             Unlink ()   ;         //!< Unlink from reader/packet.
  bool             isLinked ()   ;       //!< Query am I linked? 
  virtual   std::string getType () const  = 0 ;    //!< Return module type string (pure virt).

  virtual   void   OnDelete ()   ; // 

};

#endif
