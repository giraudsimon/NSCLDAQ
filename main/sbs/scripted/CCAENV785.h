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
#ifndef CCAENV785_H  //Required for current class
#define CCAENV785_H

#include "CDigitizerModule.h"
#include <string>
#include "CCAENModule.h"

using namespace std;

// Forward definitions:

class CAENcard;

  

/*!
  Encapsulates the CAEN 785 peak sensing adc. 
this class is layered on top of Chris Maurice's 
CAENcard class.  This  class supports the following 
configuration keywords:
  - slot      n  - n is the VME slot number holding the 
                   VME card.
  - threshold t[32]  - t are the 32 thresholds in mv. Value 
                    will be computed taking into account 
                    the full scale selection.
  - keepunder b  - b is "on" or "off" depending on whether or not to keep
                  data that does not make threshold.
  - keepoverflow b - b is "on" or "off" depending on whether 
                  or not to keep overflow data.
  - keepinvalid b - b is "on" or "off" depending on whether 
                  or not to keep data that is strobed in 
                  when the module is busy resetting.
  - card  b   - "on" to enable the card "off" to disable it.
  - enable i[32] - 32 channel enables.  Nonzero is enabled, 
                  zero disabled.
  - multievent b - True to run the module in multievent mode.
  in multievent mode, the module is note cleared after each readout 
  in Prepare()
*/
class CCAENV785  : public CCAENModule
{
private:
  
  // Private Member data:

  CAENcard* m_pCAENcard;  //!< Pointer to module driver.
  bool      m_fMultiEvent; //!< true if multievent mode.

public:
  // Constructors and other cannonical functions:

  CCAENV785(const std::string &rName, CTCLInterpreter& rInterp,
	    int nChannels=32);
 ~CCAENV785 ( );
private:
  CCAENV785 (const CCAENV785& aCCAENV785 );
  CCAENV785& operator= (const CCAENV785& aCCAENV785);
  int operator== (const CCAENV785& aCCAENV785) const;
  int operator!=(const CCAENV785& rhs) const;
public:

// Selectors:

public:

CAENcard* getCard() 
  {
    return m_pCAENcard;
  }   


  // Attribute mutators:

protected:
  void setCard (CAENcard* am_pCAENcard)
  {
    m_pCAENcard = am_pCAENcard;
  }   

  // Class operations:

public:

  virtual   void Initialize (); // 
  virtual   void Prepare (); // 
  virtual   void Clear ();
  virtual   std::string getType() const {     return std::string("caenv785");
  }

};

#endif
