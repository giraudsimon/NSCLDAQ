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


//////////////////////////CCAMACScalerLRS2551.h file//////////////////////////////////

#ifndef CVMESCALERLRS1151_H
#define CVMESCALERLRS1151_H
                               
#include "CVMEScaler.h"
#include <VmeModule.h>
                               
/*!
   Encapsulates an LRS 1151
   CAMAC scaler.  This is a 12 channel
   camac scaler.  Instantiating the scaler
   allows you to specify the camac address.
   
 */		
class CVMEScalerLRS1151  : public CVMEScaler        
{ 
private:
  CVmeModule   m_Scaler;	//!< The scaler module representations.
public:
	// Constructors, destructors and other cannonical operations: 

  CVMEScalerLRS1151 (UInt_t base, int crate = 0);
  CVMEScalerLRS1151(const CVMEScalerLRS1151& rhs); //!< Copy constructor.
  ~CVMEScalerLRS1151 ( ) { } //!< Destructor.
  
  CVMEScalerLRS1151& operator= (const CVMEScalerLRS1151& rhs); //!< Assignment
  int         operator==(const CVMEScalerLRS1151& rhs) ; //!< Comparison for equality.
  int         operator!=(const CVMEScalerLRS1151& rhs)  {
    return !(operator==(rhs));
  }
  
  // Selectors for class attributes:
public:
  
  // Mutators:
protected:  
  
  // Class operations:
public:
  virtual   void Initialize ()  ;
  virtual   void Read (std::vector<uint32_t>& Scalers) ;
  virtual   void Clear ()  ;
  virtual   unsigned int size ()  ;
  
};

#endif
