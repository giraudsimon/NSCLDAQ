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
  \class CCaenIO
  \file CaenIO.h

  Encapsulates a CAEN mod. v262 multipurpose i/o register card. Caen262's
  are mapped using a CVME<uint16_t> object. The module operates in the
  a24d16 vme space at base address 0x4444. The card consists of 4 NIM
  level inputs, 4 NIM level outputs, 4 140ns NIM pulsed outputs, and 16
  ECL levels. The read/write functions receive the input/output value to
  read from or write to, and those pararmeters must be between 0 and 3 for
  the NIM inputs/outputs. The ECL outputs are set by simply placing a bit
  pattern in its register.

  Author:
     Jason Venema
     NSCL
     Michigan State University
     East Lansing, MI 48824-1321
     mailto: venemaja@msu.edu
*/




#ifndef CCAENIO_H
#define CCAENIO_H

#include <config.h>
#include <stdint.h>
#include <VmeModule.h>

class CCaenIO : public CVmeModule
{
  uint32_t m_nOutputMask;  /*! each bit in the mask corresponds to a level
			   output. A 1 indicates the output is set. */
  enum { LENGTH = 256 };
  
 public:

  // Default constructor
  CCaenIO(uint32_t base, int nCrate = 0);

  CCaenIO(CVME<uint16_t>& am_CVME);

  // Copy constructor
  CCaenIO(const CCaenIO& aCCaenIO);

  // Destructor
  virtual ~CCaenIO() { }

  // Operator= Assignment operator
  CCaenIO& operator=(const CCaenIO& aCCaenIO);
  
  // Operator== Equality operator
  Int_t operator== (const CCaenIO& aCCaenIO);

  // Public member functions
 public:
 
  // Read from inputs
  uint16_t ReadInput(uint32_t input);
  uint16_t ReadInputs();
  
  // Writing to NIM pulse and level outputs
  void PulseOutput(uint32_t output);
  void SetLevel(uint32_t output);
  void ClearLevel(uint32_t output);
  void ClearAll();
  
  // Writing to ECL outputs
  void SetECL(uint16_t value);
  void ClearECL();
 
  uint16_t* getInputPointer();
  uint16_t* getPulsedOutputPointer();
  uint16_t* getLatchedOutputPointer();
  uint16_t* getECLOutputPointer();

private:
  void throwIfUnrecognized(uint32_t base);

};

#endif
