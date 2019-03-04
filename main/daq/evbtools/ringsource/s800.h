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


#ifndef __S800_H
#define __S800_H


#ifndef __STDINT_H
#include <stdint.h>
#ifndef __STDINT_H
#define __STDINT_H
#endif
#endif

// S800 packet definitions:

// packet types:

#define S800_OUTERVERSION          0
#define S800_OUTERVSNMASK        0xf000
#define S800_OUTERVSNSHIFT       12

#define S800_PACKET	         0x5800
#define S800_TRIGGER_PACKET	 0x5801
#define S800_TOF_PACKET		 0x5802
#define S800_TIMESTAMP_PACKET	 0x5803
#define S800_EVENT_NUMBER_PACKET 0x5804
#define S800_FP_SCINT_PACKET	 0x5810
#define S800_FP_IC_PACKET	 0x5820
#define S800_FP_TIME_PACKET	 0x5830
#define S800_FP_CRDC_PACKET	 0x5840
#define S800_II_CRDC_PACKET	 0x5850
#define S800_TA_PIN_PACKET	 0x5860
#define S800_II_TRACK_PACKET	 0x5870
#define S800_II_PPAC_PACKET	 0x5880
#define S800_OB_SCINT_PACKET	 0x5890
#define S800_OB_PIN_PACKET	 0x58A0
#define S800_FP_HODO_PACKET	 0x58B0
#define S800_VME_ADC_PACKET      0x58C0

// Struture of a typical packet:

typedef struct _packet {
  uint16_t s_size;		/* in 16 bit words. */
  uint16_t s_type;		/* packet type. */
  uint16_t s_payload[1];	/* First word of the payload. */
} packet, *ppacket;

// Structure of the s800 event:

typedef struct _S800event {
  uint32_t s_size;		/* in words */
  uint16_t s_packetSize;	/* Size of s800packet.  */
  uint16_t s_type;		/* Type should be S800_PACKET */
  uint16_t s_version;           /* We need t least 5 */
  packet   s_firstPacket;       /* Packets start here. */
} S800event, *pS800event;

typedef struct _S800timestamp {
  uint16_t s_size;		/* in 16 bit words. */
  uint16_t s_type;		/* packet type. S800_TIMESTAMP_PACKET */  
  uint64_t s_timestamp;		/* 64 bit timestamp. */

} S800timestamp, *pS800timestamp;




#endif
