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
**      Data acquisition system front end
**
**  ABSTRACT:
**
**      This file symbolically defines all of the buffer types which
**	can be sent back from the front end.
**
**  AUTHORS:
**
**      Ron Fox
**
**
**  CREATION DATE:     8-Oct-1987
**
**  MODIFICATION HISTORY:
**      @(#)buftypes.h	1.2 3/25/94 Include
**--
**/
#ifndef BUFTYPES_H
#define BUFTYPES_H

#define DATABF	        1
#define SCALERBF        2
#define SNAPSCBF	3
#define STATEVARBF      4
#define RUNVARBF        5
#define PKTDOCBF        6
#define BEGRUNBF       11
#define ENDRUNBF       12
#define PAUSEBF        13
#define RESUMEBF       14
#define PARAMDESCRIP   30

/* This is a bit hokey... it's used for trigger statistics: */

#define TRIGGERSTATS 0xffff
#endif

