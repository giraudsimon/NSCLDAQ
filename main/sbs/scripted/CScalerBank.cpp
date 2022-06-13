/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/*! \class CScalerBank   
           CLASS_PACKAGE
           Encapsulates a scaler bank.  Scaler banks
           are objects that readout an ordered list of scalers.
           They are like ReadOrder objects, but they supply
           an {\em additional} Read member that can read data
           into an ordinary UShort_t* buffer rathern than into
           one of the specialized NSCL data buffer types.
*/

////////////////////////// FILE_NAME.cpp /////////////////////////////////////////////////////
#include <config.h>
#include "CScalerBank.h"    	
#include "TCLInterpreter.h"
#include "CDigitizerDictionary.h"
#include "CScalerModule.h"			
#include <typeinfo>

using namespace std;


/*!
      Construct a scaler bank.  All the real work
      is done in the base class:
      \param pInterp CTCLInterpreter* [in]
	 The interpreter in which the bank management
	 commands will be registered.
      \param pDictionary CDigitizerDictionary* [in]
	 The module dictionary that maintains the set of
	 modules elligible to be readout.  Note that
	 this dictionary must be one which contains only
	 objects that are derived from CScalerModule.  This
	 is required because only those have the read
	 member needed. If a non scaler module is inserted,
	 this is detected and complained about at read time.
	 
*/
CScalerBank::CScalerBank (CTCLInterpreter*      pInterp,
			  CDigitizerDictionary* pDictionary) :
   CReadOrder(pInterp, pDictionary,string("bank"))
 
{   
    
} 
/*!
   Destroys a scaler bank.  All the real work is done 
   by the base class.
*/
 CScalerBank::~CScalerBank ( )  //Destructor - Delete dynamic objects
{
}


// Functions for class CScalerBank

/*!  

Reads data from the scaler banks, putting it
into the buffer passed in.  

\param pBuffer ULong_t* [out] Pointer to the place to put
   the scaler data.
\return ULong_t*  a Pointer to the buffer
       following the data.

\throw bad_cast exception if a module is in the
   readout list that is not a CScalerModule. 
*/
TCLPLUS::ULong_t* 
CScalerBank::Read(TCLPLUS::ULong_t* pBuffer)  
{ 
   CReadOrder::ModuleIterator  p = readerbegin();
   while(p != readerend()) {
      CScalerModule* pScaler = dynamic_cast<CScalerModule*>(*p);
      pBuffer = pScaler->Read(pBuffer);
     p++;
   }
   return pBuffer;
   
}
