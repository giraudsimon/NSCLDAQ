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
#ifndef CCAENV830REGISTERS_H
#define CCAENV830REGISTERS_H

/*
   This file defines the register layout and bits for the 
   CAENV830 latching multievent scaler. It can be included in either
   C or C++.
*/
typedef struct _CAENV830Registers {
  unsigned long  MEB[0x400];	/* Multievent buffer (v830 only)0x0000 */
  unsigned long  Counters[32];	/* Live counter registers       0x1000 */
  unsigned long  Testreg;	/* Test data reg (v830 only)    0x1080 */
  unsigned short pad1[7];       /* Unused                       0x1084 */
  unsigned short pad2[0x36];	/* Test registers here too      0x1094 */
  unsigned long  Enables;	/* Channel enables              0x1100 */
  unsigned long  DwellTime;	/* Dwell timing.                0x1104 */
  unsigned short Csr;		/* Control/status reg           0x1108 */
  unsigned short Bitset1;	/* Bit set 1 register           0x110a */
  unsigned short Bitclear1;	/* Bit clear 1 register         0x110c */
  unsigned short Status;        /* Status register              0x110e */
  unsigned short Slot;		/* Geographical address         0x1110 */
  unsigned short IPL;		/* Interrupt priority level     0x1112 */
  unsigned short Vector;	/* Interrupt vector             0x1114 */
  unsigned short Ader_32;	/* Top 4 bits of 32 bit address 0x1116 */
  unsigned short Ader_24;	/* Top 4 bits of 24 bit address 0x1118 */
  unsigned short EnableAder;	/* Enable address relocation    0x111a */
  unsigned short McstCbltAddr;	/* Multicast/CBLT address 4bits 0x111c */
  unsigned short McstCbltCtrl;	/* Multicast/CBLT control reg.  0x111e */
  unsigned short SwReset;	/* Write here for reset         0x1120 */
  unsigned short SwClear;	/* Clear MEB and counters       0x1122 */
  unsigned short SwTrigger;	/* Trigger latch                0x1124 */
  unsigned short pad3;		/* Unused                       0x1126 */
  unsigned long  TriggerCounter; /* Event counter.              0x1128 */
  unsigned short AlmostFull;	/* Defines when to interrupt    0x112c */
  unsigned short pad4;		/* Unused                       0x112e */
  unsigned short BltEventCount; /* # of events xferred on blt   0x1130 */
  unsigned short FirmwareRev;	/* Firmware revision level      0x1132 */
  unsigned short MEBCount;	/* # events in MEB              0x1134 */

} CAENV830Registers, *pCAENV830Registers;

/*
  The prom has some useful configuration information.  It starts at offset
  0x4000 relative to the base of the module, but the useful stuff begins
  at 0x8026:
*/
#define CAENV830_PROMBASE 0x8026

typedef struct _CAENV830ROM {
  unsigned short OUIMSB;	/* Vendor ID high I think      0x8026 */
  unsigned short pad1;		/*                             0x8028 */
  unsigned short OUI;		/* Vendor ID middle            0x802a */
  unsigned short pad2;          /*                             0x802c */
  unsigned short OUILSB;	/* Vendor ID low               0x802e */
  unsigned short pad3;		/*                             0x8030 */
  unsigned short Version;	/* Encoded variant of board    0x8032 */
  unsigned short pad4;		/*                             0x8034 */
  unsigned short ModelH;	/* Model number high           0x8036 */
  unsigned short pad5;		/*                             0x8038 */
  unsigned short ModelM;	/* Model number middle         0x803a */
  unsigned short pad6;		/*                             0x803c */
  unsigned short ModelL;	/* Model number low            0x803e */
  unsigned short pad7[7];       	/*                             0x8040 */
  unsigned short Revision;	/* Hardware rev level          0x804e */
  unsigned short pad8[1881];	/*                             0x8050 */
  unsigned short SerialH;	/* Serial MSB                  0x8f02 */
  unsigned short pad9;		/*                             0x8f04 */
  unsigned short SerialL;	/* Serial LSB                  0x8f06 */
} CAENV830ROM, *pCAENV830ROM;

/* MEB format and access macros: */

				/* Header long access.  */
#define MEBHdrGetSlot(Long)     (((Long) >> 27) & 0x1f)
#define MEBHdrGetCount(Long)    (((Long) >> 18) & 0x3f)
#define MEBHdrGetTrigger(Long)  (((Long) >> 16) & 0x03)
#define MEBHdrGetEventNum(Long) ((Long) & 0xffff)

				/* Data in 26 bit format: */
#define MEBDataGetSlot(Long)    (((Long) >> 27) & 0x1f)
#define MEBDataGet26(Long)      ((Long) & 0x7fffff)

/*  Control register mask bits (same bits in bitset/clear)            */

#define CSRAcqMode 0x03		/* 00 - Disable 01 Random 11 Periodic */
#define CSRFormat  0x04		/* 0 - 32 bit 1 - 26 bit.             */
#define CSRTest    0x08		/* Enable test mode.                  */
#define CSRBerrOn  0x10		/* Enable Bus error on end of block xfer */
#define CSRHeader  0x20		/* Enable MEB header long             */
#define CSRFPClear 0x40		/* 0 - FP Clear only counters         */
				/* 1 - FP clear clears counter & MEB  */
#define CSRAutoClr 0x80		/* Enable counter clear on trigger    */

// More on trigger bits.

#define CSRAcqDisable  0
#define CSRAcqRandom   1
#define CSRAcqPeriodic 3


/* Module status register bits:                                       */

#define STATUSDready  0x01	/* At least one event in MEB          */
#define STATUSAFull   0x02	/* MEB is 'almost' full.              */
#define STATUSFull    0x04	/* MEB is completely full             */
#define STATUSGDready 0x08	/* Global data ready                  */
#define STATUSGBusy   0x10	/* Global busy.                       */
#define STATUSTermon  0x20	/* All terminators are in             */
#define STATUSTermoff 0x40	/* All terminators are out.           */
#define STATUSBERR    0x80	/* Board has generated a bus error    */

/* Address decode relocation enabled register                         */

#define ADEREnable    0x01	/* 0 - Decode switches 1 - Decode register */

/* Mcst/CBLT control register                                        */

#define MCSTDisabled 0x00	/* Multicast/CBLT disabled.     */
#define MCSTLast     0x01	/* Board is last in chain.      */
#define MCSTFirst    0x02	/* Board is first in chain      */
#define MCSTMiddle   0x03	/* Board is in middle of chain. */

/* Firmware revision register macros:  */

#define FWMajor(Word)   (((Word) >> 8) & 0xff)
#define FWMinor(Word)   ((Word) 0xff)


#endif
