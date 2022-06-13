/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

//! \class: CCAENV775           
//! \file:  .h
// Author:
//   Ron Fox
//   NSCL
//   Michigan State University
//   East Lansing, MI 48824-1321
//   mailto:fox@nscl.msu.edu
//
// Copyright 

#ifndef CCAENV775_H  //Required for current class
#define CCAENV775_H

//
// Include files:
//

#include "CDigitizerModule.h"
#include <CCAENModule.h>
#include <string>

using namespace std;




// Forward class definitions.

class CAENcard;

/*!
  Encapsulates a CAEN V755 module.  The V775 module 
  is a 32 channel multievent TDC.  Programmatically,
  this module is a V785 ADC with front ends consisting of
  32 channels of TAC (time to analog converters).   The 
  configuration parameters accepted by this module include:
  - slot      n  - n is the VME slot number holding the VME card.
  - threshold t[32]  - t are the 32 thresholds in nanoseconds.  Value will 
                    be computed taking into account the full scale
                    selection.
  - keepunder b  - b is "on" or "off" depending on whether or not to keep
                  data that does not make threshold.
  - keepoverflow b - b is "on" or "off" depending on whether or not to
                  keep overflow data.
  - keepinvalid b - b is "on" or "off" depending on whether or not to keep
                data that is strobed in when the module is busy resetting.
  - commonstop b - b is "on" to run in common stop mode, "off" to run in 
a              common start mode.
  - range t   - Sets the tdc range in 1ns units.  Note that
                the range of t is between 140 and 1200 and
                the resolution is good to about the nearest
                4ns.
  - card  b   - "on" to enable the card "off" to disable it.
  - enable i[32] - 32 channel enables.  Nonzero is enabled, zero disabled.
  - multievent b - On to enable multi-event mode.  In multievent
                   mode, the module is not cleared 
		   in Prepare() otherwise it is.

  
*/
class CCAENV775  : public CCAENModule
{
private:
  
public:
// Constructors and other cannonical functions.
CCAENV775(const std::string& rName, CTCLInterpreter& rInterp);
 virtual ~CCAENV775( );  
private:
  CCAENV775 (const CCAENV775& aCCAENV775 );
  CCAENV775& operator= (const CCAENV775& aCCAENV775);
  int operator== (const CCAENV775& aCCAENV775) const;
  int operator!= (const CCAENV775& aCCAENV775) const;
public:

  // Class operations:

public:

  virtual   void Initialize ();  
  virtual   void Prepare ();  
  virtual   void Clear ();
  virtual std::string getType() const {
     return std::string("caenv775");
  }

};

#endif
