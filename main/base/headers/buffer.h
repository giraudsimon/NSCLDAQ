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
**++
**  FACILITY:
**
**      Data acquisition system front end MASTER processor.
**
**  ABSTRACT:
**
**      This file contains type definitions used by the master processor
**  We assume the definition of the following types:
**	int32_t	- 32 bit integer
**      int16_t	- 16 bit integer
**	INT8	- eight bit integer.
**
**  AUTHORS:
**
**      Ron Fox
**
**
**  CREATION DATE:     7-Oct-1987
**
**  MODIFICATION HISTORY:
**     @(#)mtypes.h	1.2 3/25/94 Include
**--
**/

#ifndef MTYPES_H	
#define MTYPES_H

#include <stdint.h>

#define BUFFER_REVISION 5
#define JUMBO_BUFFER_REVISION 6
/*		Absolute time:		*/

struct bftime
    {
	int16_t	month;			/* Month 1-12		*/     /* 3 */
	int16_t	day;			/* Day	 1-31		*/     /* 3 */
	int16_t	year;			/* e.g. 1987		*/
	int16_t	hours;			/* 0-23			*/     /* 3 */
	int16_t	min;			/* 0-59			*/     /* 3 */
	int16_t	sec;			/* 0-59			*/     /* 3 */
	int16_t	tenths;			/* 0-9.			*/     /* 3 */
    };

/*		Structures which describe the final output data buffers */

struct bheader				/* Data buffer header	*/
    {
	int16_t	nwds;			/* Used part of buffer	*/
	int16_t	type;			/* buffer type		*/
	int16_t	cks;			/* checksum over used part of buffer */
	int16_t	run;			/* Run number		*/
	int32_t	seq;			/* Buffer sequence number */
	int16_t	nevt;			/* Event count in buffer    */
	int16_t	nlam;			/* Number of lam masks	    */
	int16_t	cpu;			/* Processor number	    */
	int16_t	nbit;			/* Number of bit registers */
	int16_t	buffmt;			/* Data format revision level */
	int16_t   ssignature;		/* Short byte order signature */
	int32_t   lsignature;		/* Long byte order signature  */
	int16_t	unused[2];		/* Pad out to 16 words.	    */
    };

struct ctlbody				/* Body of control buffer   */
    {					/* start/stop/pause/resume  */
	char    title[80];		/* Run title.		    */
	int32_t	sortim;			/* Time in ticks since run start */
	struct  bftime tod;		/* Absolute time buffer was made    */
    };

struct	usrbufbody			/* Declares user buffer body. */
{
    struct bftime   usertime;		/* Time stamp for user buffer. */
    int16_t  userbody[1];			/* Body of user buffer.	       */

};

struct sclbody				/* body of scaler buffers   */
    {					/* taped and snapshot	    */
	int32_t	etime;			/* Start time since SOR in ticks */
	int16_t	unused1[3];		/* Unused words.	    */
	int32_t	btime
#ifdef __GNUC__
	__attribute__((packed))
#endif
	;			/* end time since SOR in ticks	*/
	int16_t	unused2[3];		/* Unused words.	    */
	int32_t	scalers[1]
#ifdef __GNUC__
	__attribute__((packed))
#endif
;		/* Array with scaler data   */
    };

/*	    The types below define the structure of event packets put in    */
/*	the circular buffer queue by the event acquisition processor.	    */

typedef    int32_t    ctlevt;		/* Control events just have time    */

struct sclevt				/* Scaler event		    */
    {
	int32_t	bticks;			/* Ticks at interval start  */
	int32_t	eticks;			/* Ticks at interval end.   */
	int16_t	nscl;			/* Number of scalers	    */
	int32_t	scls[1]
#ifdef __GNUC__
	__attribute__((packed))
#endif
	;	       	/* The scaler data	    */
    };

struct phydata							       /* 4 */
    {								       /* 4 */
	int16_t    cnt;			/* Size of event data in words */ /* 4 */
	int16_t	 data[1];		/* actual data		    */ /* 4 */
    };								       /* 4 */

struct	usrevt				/* User generated event.	 */
{
    int16_t   usrevtbody[1];		/* User event body.		  */
};



typedef    struct bheader    BHEADER;	    /* Buffer header		*/
typedef    struct event      EVENT;	    /* Event structure		*/
typedef    struct evtpool    EVTPOOL;	    /* Event pool control struct */
typedef    struct part	     PART;	    /* Partition control block	*/ /* 2 */

#endif
