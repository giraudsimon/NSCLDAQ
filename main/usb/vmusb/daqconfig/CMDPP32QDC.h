/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Genie Jhang
	     FRIB
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CMDPP32QDC_H
#define __CMDPP32QDC_H

#ifndef __CMESYTECBASE_H
#include "CMesytecBase.h"
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

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

#include <CVMUSBReadoutList.h>

#ifndef Const
#define Const(name) static const int name =
#endif

// Forward class definitions:
class CReadoutModule;
class CVMUSB;
class CVMUSBReadoutList;

static const uint8_t initamod(CVMUSBReadoutList::a32UserData);   //  setup using user data access.
static const uint8_t readamod(CVMUSBReadoutList::a32UserBlock);  //  Read in block mode.

Const(MDPPDELAY)            1;
Const(MDPPCHCONFIGDELAY)    101;  // 200 ns x 101 = 20.2 us

// Mcast/CBLT control register bits
Const(MCSTENB)              0x80;
Const(MCSTDIS)              0x40;
Const(FIRSTENB)             0x20;
Const(FIRSTDIS)             0x10;
Const(LASTENB)              0x08;
Const(LASTDIS)              0x04;
Const(CBLTENB)              0x02;
Const(CBLTDIS)              0x01;

Const(eventBuffer)          0;

Const(AddressSource)        0x6000;
Const(Address)              0x6002;
Const(ModuleId)             0x6004;
Const(Reset)                0x6008; // write anything here to reset the module
Const(FirmwareRev)          0x600e;

// IRQ
Const(Ipl)                  0x6010;
Const(Vector)               0x6012;
Const(IrqTest)              0x6014;
Const(IrqReset)             0x6016;
Const(IrqDataThreshold)     0x6018;
Const(MaxTransfer)          0x601a;
Const(IrqSource)            0x601c;
Const(IrqEventThreshold)    0x601e;

// Mcast/CBLT addresses
Const(CbltMcstControl)      0x6020;
Const(CbltAddress)          0x6022;
Const(McstAddress)          0x6024;

Const(LongCount)            0x6030;
Const(DataFormat)           0x6032;
Const(ReadoutReset)         0x6034;
Const(MultiEvent)           0x6036;
Const(MarkType)             0x6038;
Const(StartAcq)             0x603A;
Const(InitFifo)             0x603c;
Const(DataReady)            0x603e;

Const(TDCResolution)        0x6042;
Const(OutputFormat)         0x6044;
Const(ADCResolution)        0x6046;

// Trigger
Const(WindowStart)          0x6050;
Const(WindowWidth)          0x6054;
Const(TriggerSource)        0x6058;
Const(FirstHit)             0x605c;
Const(TriggerOUtput)        0x605e;

Const(ECL3)                 0x6060;
Const(ECL2)                 0x6062;
Const(ECL1)                 0x6064;
Const(ECL0)                 0x6066;
Const(NIM4)                 0x6068;
Const(NIM3)                 0x606a;
Const(NIM2)                 0x606c;
Const(NIM1)                 0x606e;

// Test pulser
Const(TestPulser)           0x6070;
Const(PulserAmplitude)      0x6072;
Const(NIM0)                 0x6074;
Const(MonSwitch)            0x607a;
Const(SetMonChannel)        0x607c;
Const(SetWave)              0x607e;

// RC-bus registers
Const(RCBusNo)              0x6080;
Const(RCModNum)             0x6082;
Const(RCOpCode)             0x6084;
Const(RCAddr)               0x6086;
Const(RCData)               0x6088;
Const(RCStatus)             0x608a;

Const(EventCounterReset)    0x6090;
Const(EventCtrLow)          0x6092;
Const(EventCtrHigh)         0x6094;
Const(TimingSource)         0x6096;
Const(TimingDivisor)        0x6098;
Const(TimestampReset)       EventCounterReset;
Const(TSCounterLow)         0x609c;
Const(TSCounterHi)          0x609e;

Const(TDCCtrBTimeL)         0x60a8;
Const(TDCCtrBTimeM)         0x60aa;
Const(TDCCtrBTimeH)         0x60ac;
Const(TDCStopCtrB)          0x60ae;

// Multiplicity filter
Const(Bank0HighLimit)       0x60b0;
Const(Bank0LowLimit)        0x60b2;

Const(ChannelSelection)     0x6100;
Const(SignalWidth)          0x6110;
Const(InputAmplitude)       0x6112;
Const(JumperRange)          0x6114;
Const(QDCJumper)            0x6116;
Const(IntegrationLong)      0x6118;
Const(IntegrationShort)     0x611a;
Const(Threshold0)           0x611c;
Const(Threshold1)           0x611e;
Const(Threshold2)           0x6120;
Const(Threshold3)           0x6122;
Const(ResetTime)            0x6128;
Const(LongGainCorrection)   0x612a;
Const(ShortGainCorrection)  0x612e;

// Special trigger outputs:
Const(TrigToIRQ1L)          0x6300; // Ch 0-16 from low to high bit
Const(TrigToIRQ1H)          0x6302;
Const(TrigToIRQ2L)          0x6304;
Const(TrigToIRQ2H)          0x6306;
Const(TrigToIRQ3L)          0x6308;
Const(TrigToIRQ3H)          0x630a;
Const(TrigToIRQ4L)          0x630c;
Const(TrigToIRQ4H)          0x630e;
Const(TrigToIRQ5L)          0x6310;
Const(TrigToIRQ5H)          0x6312;
Const(TrigToIRQ6L)          0x6314;
Const(TrigToIRQ6H)          0x6316;
Const(TrigToIRQ7L)          0x6318;
Const(TrigToIRQ7H)          0x631a;

/*!
   The MDPP-32 is a 32 channel fast high resolution time and amplitude digitizer module produced by Mesytec.
   The following configuration parameters can be sued to tailor
   the module:

\verbatim
   Name                 Value type          Description
   -base                integer             Base address of the module in VME space.
   -id                  integer [0-255]     Module id (part of the module header).
   -ipl                 integer [0-7]       Interrupt priority level 0 means disabled.
   -vector              integer [0-255]     Interrupt vector.
   -irqdatathreshold    integer [0-32256]   Threshold of the number of 32bit words in FIFO to transfer
   -irqeventthreshold   integer [0-32256]   Threshold of the number of events in FIFO to transfer
   -irqsource           enum (data,event)   Which IRQ threshold to be applied
   -maxtransfer         integer [0-irqth]   The maximum amount of data being transferred at once. See Doc.
   -datalenformat       integer [0-4]       Data length format. See Doc.
   -multievent          integer             Multi event register. See Doc.
   -marktype            enum (eventcounter,timestamp,extended-timestamp)
   -tdcresolution       integer [0-5]       25ns/2^(10-value)
   -adcresolution       enum (4k,8k,16k,32k,64k)
   -outputformat        integer [0-3]       0:Time(T) and long integral(L), 1:L, 2:T, 3:LT and short integral
   -signalwidth         int[8] [0-1023]     FWHM in ns
   -inputamplitude      int[8] [0-65535]    0 to peak voltage in mV. Maximum value is the jumper range value.
   -jumperrange         int[8] [0-65535]    Range printed on jumper top.
   -qdcjumper           bool[8]             If QDC jumper is used.
   -intlong             int[8] [2-506]      Long integration time. Multiple of 12.5 ns.
   -intshort            int[8] [1-intlong)  Short integration time. Multiple of 12.5 ns.
   -threshold           int[32] [1-65535]   Threshold to start measuring. Calculated as value/0xFFFF percentage.
   -resettime           int[8] [0-1023]     When OF/UF, input preamp and digital section is resetted.
   -gaincorrectionlong  enum (div4,mult4,none) Either divide by 4 or multiply by 4 to the integral value.
   -gaincorrectionshort enum (div4,mult4,none) Either divide by 4 or multiply by 4 to the integral value.
   -printregisters      bool                Print out all the register values on screen.
\endverbatim

   Comment by Genie:
     - MDPP-16 QDC firmware has tf_gain_correction at 0x612C while MDPP-32 doesn't have one listed in the doc.
     - MDPP-32 QDC chain methods are implemented, but chain mode is not supported as of 05/24/22.
*/

class CMDPP32QDC : public CMesytecBase
{
public:
  typedef std::map<std::string, uint16_t> EnumMap;

private:
  CReadoutModule* m_pConfiguration;

public:
  CMDPP32QDC();
  CMDPP32QDC(const CMDPP32QDC& rhs);
  virtual ~CMDPP32QDC();

private:
  CMDPP32QDC& operator=(const CMDPP32QDC& rhs); // assignment not allowed.
  int operator==(const CMDPP32QDC& rhs) const;	  // Comparison for == and != not suported.
  int operator!=(const CMDPP32QDC& rhs) const;


public:
  virtual void onAttach(CReadoutModule& configuration);
  virtual void Initialize(CVMUSB& controller);
  virtual void addReadoutList(CVMUSBReadoutList& list);
  virtual void onEndRun(CVMUSB& controller);
  virtual CReadoutHardware* clone() const; 

public:
  static EnumMap gainCorrectionMap();

  void setChainAddresses(CVMUSB& controller,
                         CMesytecBase::ChainPosition position,
                         uint32_t      cbltBase,
                         uint32_t      mcastBase);

  void initCBLTReadout(CVMUSB& controller,
                       uint32_t cbltAddress,
                       int wordsPermodule);


private:
  void printRegisters(CVMUSB& controller);
};

#endif
