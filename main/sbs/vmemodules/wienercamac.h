
#ifndef WIENERCAMAC_H
#define WIENERCAMAC_H
#include <daqdatatypes.h>
/* 
The Wiener VC32 each control only one crate, so an array of pointers is
required for multiple crate operation
*/

extern void* (pBranchBases[]);
#define CAMBAS(b) (pBranchBases[(b)])


/* New camac inlines when wiener vc32cc32 modules are used --ddc */

/* 
HUH... better check what's going on here because getlong and putlong are
also defined as macros in vme.h!  Ah... ok. For __unix__ vme.h is
not included.  So camac.h is the only place this defined...  --ddc
*/

/* not loved... don't know why 
static inline UINT32 getlong(volatile void* a) {
    return (*((volatile UINT32*)a));
};
*/




/* 
 *CAM16 and CAM32 were previously used in computing CAMAC pointers...
 * but that will be done with type casting now (all addresses are for
 * 32bit words with Wiener, so 32bit words will be accessed with a type cast
 * after the address is computed) --ddc.
 */

#define    CAMF    2              /* Shift count to F field. */
#define    CAMA    6              /* Shift count to A field  */
#define    CAMN    10             /* Shift count to N field  */
#define    CAM24   0              /* unused for wiener interface */
#define    CAM16   0              /* Unused for wiener interface */
#define    CAMB    0		/*  Unused if wiener interface */

/*
 * The macros are used to do accesses to the CAMAC at higher 
 * speed than function calls might allow.
 */

/*
 *                       CBDPTR                        
 * CBDPTR     - Generate a CAMAC pointer for the VC32 cards.
 *            - Arguments: c - crate, n - slot,
 *            -            a - subaddress, f - function
 *          By compile time generation, we mean that this macro
 *          can be used on the right hand side of #define directives
 *          or variable initializers for example:
 * #define READDE1 CBDPTR(1, 1, 0, 0)
 * INT16   *readde1 = CBDPTR(1, 1, 0, 0);
 * produce a macro READDE1 which produces the same pointer that the 
 * variable readde1 is initialized to point to.
*/
/*
 * NOTE! The version of CBDPTR varies significantly from the version
 * in camac.h used at MSU.  There is NO branch(b) or size(s) parameters!
 * also note that only four bits are used for "f" (the 5th bit is implicit
 * in the operation).  Also note that to match common usage, crates are
 * numbered starting with "1" (so the index is decremented).
 * --ddc.
 */

/* Unhappiness with recasting a pointer in 24bit reads... replace this --ddc
#define CBDPTR(c, n, a, f) \
    ((volatile INT16 *)(((unsigned long)CAMBAS(c-1)) + \
              ( (n) << CAMN ) | ( (a) << CAMA) | ( (f&0x0f) << CAMF )))
*/

#define CBDPTR(b, c, n, a, f, width) \
    ((unsigned long)(((unsigned long)CAMBAS(c-1)) + \
              ( (n) << CAMN ) | ( (a) << CAMA) | ( (f&0x0f) << CAMF )))

/*
 *                    CAMCTL
 * Do a CAMAC control operation (non data transfer)  The argument is
 * use in a statement by itself, e.g.    CAMCTL(ADC1RST); 
*/

/* camctl for cc32 assumes writes (but no data transfer --ddc */

#define    CAMCTL(ptr)    { if(*((volatile INT16 *)(ptr))=0){} }

/*
 *                    CAMRD16
 * Do a 16 bit read from CAMAC via a pointer cons4ructed with
 * CBDPTR. The macro returns a value (may be treated like a
 * function) which is the result of the read.
 * Sample usage:
 * result = (CAMRD16(DE1) * scale) / offset;
*/

/* camrd16 is unchanged --ddc */

#define    CAMRD16(ptr)    (*(volatile INT16 *)(ptr))

/*
 *                    CAMRD24
 * Same as CAMRD16, but a 24 bit read is done... (pointer still for 16bit)
 * This operation will be quite a bit slower than CAMRD16
 */

/*This is unchanged, BUT getlong is quite different.  It recasts the
pointer, which is ALWAYS for a 16 bit word.  It is unclear to me if
how much slower this is with vc32!  Manual claims ~10% --ddc  */

/*#define    CAMRD24(ptr)    (getlong(ptr) & 0xffffff) */
/*replacing CAMRD24 with macro to eliminate inline getlong... --ddc */

#define CAMRD24(ptr)  ((*(INT32 *)(ptr))&0xffffff)
#define CAMRD32(ptr)  (*(INT32 *)(ptr))

/*
 *                    CAMWR16
 * Does a 16 bit write CAMAC cycle. Use as a single statment e.g.
 * CAMWR16(ptr, value);
 * Arguments:
 *         ptr - Pointer produced by CBDPTR.
 *         value - Least significant 16 bits of this are written
 */

     /* this is unchanged --ddc */
#define    CAMWR16(ptr, val)    (*((volatile INT16 *)(ptr)) = (val))

/*
 *                    CAMWR24
 * Just like CAMWR16, but the operation writes the lower 24 bits of
 * the data.  The pointer argument must have been constructed with
 * size CAM24
 */

#define    CAMWR24(ptr, val)    (putlong( ((val) & 0xFFFFFF), ((volatile INT32 *)ptr)))

/*
 * This is unchanged... although I don't understand why the ptr was recast
 * to the pointer for a long (INT32) pointer in the original camac.h
 * (since it is recast there to int16 * in putlong!) --ddc
 */

/*  The definitions below allows the library code to figure out which version
    of the headers the user has selected.
*/



#include <wienermacros.h>    /* Don't allow mis-match screwups */

#endif
