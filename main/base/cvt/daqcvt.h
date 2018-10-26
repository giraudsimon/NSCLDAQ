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

/*
** Facility:
**	Portable Data acquisition
** Abstract:
**	daqcvt.h	- This module defines byte order conversion routines
**			  for binary integer data.  We support integer
**			  and longword data.  The machine that is executing
**			  the code is said to be the host machine.  The
**			  machine the data is from is said to be the foreign
**			  machine.
** Author:
**	Ron Fox
**	NSCL
**	Michigan State University
**	East Lansing, MI 48824-1321
** @(#)daqcvt.h	1.2 7/8/92 IncludeFile
*/

#ifndef DAQCVT_H_INSTALLED_
#define DAQCVT_H_INSTALLED_

/*
** Public data types:
*/


#include <stdint.h>


#define CVT_WORDSIGNATURE 0x0102       /* Word signature for byte ordering */
#define CVT_LONGSIGNATURE 0x01020304l  /* Longword sig. for byte ordering */



#ifdef __cplusplus
extern "C" {
#endif

	/* Byte conversion tables: */

typedef struct {
		  int HostToForeignWord[2];
		  int HostToForeignLong[4];

		  int ForeignToHostWord[2];
		  int ForeignToHostLong[4];
		}   DaqConversion;

      /* Routines to create conversion blocks */

int hostsameasforeign(DaqConversion *conversion);  /* TRUE if conversion same as host */
void makecvtblock(uint32_t lsig, uint16_t ssig,           /* Create a conversion block */
                  DaqConversion *conversion);      /* Given foreign signatures */


	    /* Conversion routines: */

uint32_t  ftohl(DaqConversion *convertdata, /* Convert longword datum to host */
	   uint32_t datum);	/* format from foreign format	  */

uint16_t ftohw(DaqConversion *convertdata, /* Convert short datum to host */
	    uint16_t datum);		/* format from foreign format   */

uint32_t  htofl(DaqConversion *convertdata, /* Convert longword datum to */
	    uint32_t datum);		/* foreign from host format  */

uint16_t htofw(DaqConversion *convertdata, /* Convert short datum from host */
	       uint16_t datum);		/* to foreign format 		*/

#ifdef __cplusplus
}
#endif

#endif


