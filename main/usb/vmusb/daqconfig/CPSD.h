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

#ifndef __CPSD_H
#define __CPSD_H

#ifndef __CREADOUTHARDWARE_H
#include "CReadoutHardware.h"
#endif

#ifndef __CRT_STDINT_H
#include <stdint.h>
#ifndef __CRT_STDINT_H
#define __CRT_STDINT_H
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __CXLM_H
#include "CXLM.h"		/* Base class definition. */
#endif

// register layout of PSD XLM
/* define bit masks for output from motherboard register */
#define acqall       0x10000

#define glbl_enable  0x40000
#define serin        0x80000
#define serclk       0x100000
#define tokenin      0x200000            // active low, inverted in FPGA
#define forcereset   0x400000
#define dacstb       0x800000
#define dacsign      0x1000000
#define selextbus    0x2000000
#define ld_dacs      0x4000000
#define vetoreset    0x8000000
#define force_track  0x10000000
#define XLMout       0x80000000

/************* Inputs  ***************/

#define ser_busy      0x80000000  // for XLM XXV or dual-port XLM 80M
// #define ser_busy      0x2000  // for old XLM 80M
#define token_out   0x4000
#define serout      0x8000
#define acqack      0x10000
#define orout       0x20000

/********* register addresses in XLM FPGA *********/
// these are word addresses
#define FPGA_reset       0x00       // write to this reg resets XLM
#define FPGA_acq_a       0x01      // write starts acq cycle on A bus
#define FPGA_acq_b       0x02       // write starts acq cycle on B bus
#define FPGA_set_delay   0x03       // set delay timings
#define FPGA_set_timeout 0x04       // set timeout counters
#define FPGA_ABus        0x05       // read or write to A bus bits
#define FPGA_Bbus        0x06       // read or write to B bus bits
#define FPGA_enblA       0x07       // External Enable for Bus A
#define FPGA_enblB       0x08       // External Enable for Bus B
#define FPGA_clear_veto  0x09       // Strobes glbl_enbl veto clear
#define FPGA_trig_delay  0x0a       // Sets the trigger delay
#define FPGA_coin_window 0x0b       // Sets width of coincidence window
#define FPGA_force_A     0x0c       // Force Readout for Bank A
#define FPGA_force_B     0x0d        // Force Read for Bank B
#define FT_DELAY         0x0e        // Force Track Delay Register
#define AA_DELAY         0x0f        // ACQ_ALL DELAY REGISTER
#define GD_DELAY         0x10        // GLOBAL DISABLE DELAY REGISTER
#define FAST_SERA        0x11        // fast serial A (16 bits at a time)
#define FAST_SERB        0x12        // fast serial B (16 bits at a time)
#define FPGA_FRC         0x13        // a free-running counter
#define FPGA_TEST        0x15        // a read-write test register
#define FPGA_ID          0x16        // FPGA version ID
#define ReadoutMode      0x18        // select legacy or unified readout mode

// Forward class definitions:

class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;


/*!
  Provides support for the Washington U. PSD XLM based readout system.  This class uses the CXLM base class
  to provide the basic services and configuration parameters it requires.
  We don't require any additional configuration parameters.  Note, however that it is
  recommended that the event stack have a 10usec delay between trigger and list start
  in order to allow the MASE hardware to complete data acquisition and transfer to the
  XLM SRAM B.
  structure of the SRAMB is
\verbatim
+-----------------------------------------+
|  data count (noninclusive               |
+-----------------------------------------+
|  ... data ...                           |
~                                         ~
+-----------------------------------------+
\endverbatim
   At end of read the readout must 'reset' the XLM by touching its register set.
*/

class CPSD : public XLM::CXLM
{
public:
  CPSD();
  CPSD(const CPSD& rhs);
  virtual ~CPSD();
  CPSD& operator=(const CPSD& rhs);

private:
  int operator==(const CPSD& rhs) const;
  int operator!=(const CPSD& rhs) const;

  // Standard readout interface.. note that onAttach is implemented by the base class
  // and that its implementation is perfectly fine for us.
  // 

public:
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual CReadoutHardware* clone() const;

};

#endif
