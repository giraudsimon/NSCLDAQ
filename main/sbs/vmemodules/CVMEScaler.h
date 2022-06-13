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


//////////////////////////CVMEScaler.h file//////////////////////////////////

#ifndef CVMESCALER_H  
#define CVMESCALER_H

#include <vector>
#include <stdint.h>

                               
/*!
   ABC for scaler modules.
   A scaler module is a collection  of 
   high performance counters which can be
   sequentially read and cleared.
 */		
class CVMEScaler   
{ 
private:

public:
	// Constructors, destructors and other cannonical operations: 

  CVMEScaler ();                      //!< Default constructor.
  CVMEScaler(const CVMEScaler& rhs); //!< Copy constructor.
  ~ CVMEScaler ( ) { } //!< Destructor.
  
  CVMEScaler& operator= (const CVMEScaler& rhs); //!< Assignment
  int         operator==(const CVMEScaler& rhs) const; //!< Comparison for equality.
  int         operator!=(const CVMEScaler& rhs) const {
    return !(operator==(rhs));
  }
  
  // Selectors for class attributes:
public:
  
  // Mutators:
protected:  
  
  // Class operations:
public:
  virtual   void Initialize ()   = 0;
  virtual   void Read (std::vector<uint32_t>& Scalers)   = 0;
  virtual   void Clear ()   = 0;
  virtual   unsigned int size ()   = 0;
  
};

#endif
