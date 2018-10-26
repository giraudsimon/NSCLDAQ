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
*++
**  FACILITY:
**
**      Data acquisition system
**
**  ABSTRACT:
**
**      camac.h	- This file contains definitions and MACROS which 
**		  can be used to access the CAMAC system 
**
**  AUTHORS:
**
**      Ron Fox
**
**
**  MODIFICATION HISTORY:
**--
**/
#ifndef CESCAMAC_H
#define CESCAMAC_H

#include <daqdatatypes.h>



#include <stdint.h>




/* CAMAC 'keywords'		 */


#define    CSRCRATE    0	    /* 'crate' for Branch CSR */
#define    CSRSLOT    29	    /* 'slot' for branch CSR  */
#define    CSRSUBAD    0	    /* 'subaddress' for branch CSR */
#define    CSRFUN    0		    /* 'f code' for branch CSR	    */



/* The define below is used in the construction of CAMAC pointers: */

#define    CAM16    2		    /* 16 bit operation */
#define    CAM24    0		    /* 24 bit operation */
#define    CAMF    2		    /* Shift count to F field. */
#define    CAMA    7		    /* Shift count to A field  */
#define    CAMN    11		    /* Shift count to N field  */
#define    CAMC    16		    /* Shift count to C field  */
#define    CAMB    19		    /* Shift count to B field  */
				    /* Base address of CAMAC branches: */
#ifdef __unix__
extern void*  (pBranchBases[]);
#ifdef ADDR64
#define CAMBAS(b) ((uint64_t)pBranchBases[(b)])
#else
#define CAMBAS(b) ((uint32_t)pBranchBases[(b)])
#endif
#else
#define    CAMBAS(b)  ((int)(camac.busbase + (int)busbases[camac.localaccess]))
#endif

/* The definitions below are bits in the BRANCH CSR.		     */

#define    IT4    1                 /* Interrupt 4 pendng.           */
#define    IT2    2                 /* Interrupt 2 pending           */
#define    MIT4   0x4		    /* Mask interrupt 4.	     */
#define    MIT2   0x8		    /* Mask interrupt 2.	     */
#define    MLAM   0x10		    /* Mask LAM interrupts.	     */
#define    MTO    0x20		    /* MASK Branch timeout.	     */
#define    MNOX	  0x800		    /* Mask BERR on no x.	     */
#define    QMASK  0x8000	    /* Q bit in branch CSR.	    */
#define    XMASK  0x4000	    /* X bit in branch CSR.	     */


/* The macros below can be used to do accesses to the CAMAC at higher */
/* speed than function calls might allow.			      */

    /*			    CBDPTR				     */
    /* CBDPTR	- Generate a CAMAC pointer for the branch driver at */
    /*		- compile time.  To generate a pointer which might  */
    /*		- vary at run time, use the lower case version	     */
    /*		- cbdptr which has the same call sequence.	     */
    /*		- Both produce a pointer type			     */
    /*		- Arguments: b - branch, c - crate, n - slot,		*/
    /*		-            a - subaddress, f - function	    */
    /*		-	     s - size, one of CAM16 or CAM24	    */
    /*		By compile time generation, we mean that this macro */
    /*		can be used on the right hand side of #define directives */
    /*		or variable initializers for example:		     */
    /* #define READDE1 CBDPTR(0, 1, 1, 0, 0, CAM16)		     */
    /* uint16_t   *readde1 = CBDPTR(0, 1, 1, 0, 0, CAM16);		     */
    /* produce a macro READDE1 which produces the same pointer that the	*/
    /* variable readde1 is initialized to point to.		     */

#define CBDPTR(b, c, n, a, f, s) \
    ((volatile uint16_t *)((uint8_t*)CAMBAS(b) + (s) +  ( (c) << CAMC) + \
              ( (n) << CAMN ) + ( (a) << CAMA) + ( (f) << CAMF )))

    /*				CAMCTL					 */
    /* Do a CAMAC control operation (non data transfer)  The argument is */
    /* A pointer which has been constructed via either CBDPTR or cbdptr	 */
    /* the size specified when constructing the pointer must have been	 */
    /* CAM16 or else this will flop abysmally				 */
    /* use in a staement by itself, e.g.    CAMCTL(ADC1RST);		 */

#define    CAMCTL(ptr)    { if(*((volatile uint16_t *)(ptr))){} }
#ifndef __unix__
#define    CAMCTLP(ptr)   probew((uint16_t *)(ptr))
#endif
    /*				CAMRD16					 */
    /* Do a 16 bit read from CAMAC via a pointer constructed with either */
    /* CBDPTR or cbdptr. The macro returns a value (may be treated like a */
    /* function) which is the result of the read.   The size argument	 */
    /* specified when constructing the pointer must have been CAM16	 */
    /* Sample usage:							 */
    /* result = (CAMRD16(DE1) * scale) / offset;			 */

#define    CAMRD16(ptr)    (*(volatile uint16_t *)(ptr))
#ifndef __unix__
#define    CAMRD16P(ptr,dest) prbrdw(((uint16_t *)(ptr)), ((uint16_t *)(&dest)))
#endif

    /*				CAMRD24					 */
    /* Same as CAMRD16, but a 24 bit read is done, and the pointer must	 */
    /* have been generated with size CAM24				 */
    /* This operation will be quite a bit slower than CAMRD16		 */

#define    CAMRD24(ptr)    (getlong(ptr) & 0xffffff)
#ifndef VME16
#ifndef __unix__
#define    CAMRD24P(ptr,dest) prbrdl(((uint32_t *)(ptr)), ((uint32_t *)(&dest)))
#endif
#endif
    /*				CAMWR16					 */
    /* Does a 16 bit write CAMAC cycle. Use as a single statment e.g.	 */
    /* CAMWR16(ptr, value);						 */
    /* Arguments:							 */
    /*		    ptr - Pointer produced by either CBDPTR or cbdptr.	 */
    /*			   size must have been CAM16			 */
    /*		    value - Least significant 16 bits of this are written */

#define    CAMWR16(ptr, val)    (*((volatile uint16_t *)(ptr)) = (val))
#ifndef __unix
#define    CAMWR16P(ptr, val)  prbwtw(((volatile uint16_t *)(ptr)), ((uint16_t)(val)))
#endif
    /*				CAMWR24					 */
    /* Just like CAMWR16, but the operation writes the lower 24 bits of	 */
    /* the data.  The pointer argument must have been constructed with	 */
    /* size CAM24							 */

#define    CAMWR24(ptr, val)    (putlong( ((val) & 0xFFFFFF), ((volatile uint32_t *)ptr)))
#ifndef VME16
#ifndef __unix__
#define    CAMWR24P(ptr, val)  prbwtl(((uint32_t *)(ptr)), ((uint32_t)(val)))
#endif
#endif


#include <cesmacros.h>   /* Don't allow mis-match screwups. */

#endif



