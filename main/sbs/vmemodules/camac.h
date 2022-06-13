/*!
  This file describes interfaces into the CAMAC system.  
  At present the following CAMAC interfaces are supported
  and this file simply includes the one selected at compile time by the
  value of the CAMACINTERFACE macro:

  CESCAMAC  - CES CBD8210 VME Camac branch highway interface.
  VC32CAMAC - WIENER VC32/CC32 VME - single crate controller.

  Acknowledgements:
   - Thanks to Dave Caussyn of FSU for the intial VC32CAMAC support this
     version's software is based on.

*/
#ifndef CAMAC_H
#define CAMAC_H

#include <daqdatatypes.h>
#include <stdint.h>
#include <config.h>

/*----------------------------------------------------------------------
 *
 * At compile time, the base macros that access the camac system are
 * selected by defining one of CESCAMAC or VC32CAMAC below.
 * These provide base camac access, and device specific macros
 */

#ifdef CESCAMAC
#include <cescamac.h>
#else
#ifdef  VC32CAMAC
#include <wienercamac.h>
#endif
#endif

#ifndef CBDPTR                      /* Macro defined by *camac.h */

#error "You must select a supported value for the CAMAC INTERFACE macro"

#endif

/*---------------------------------------------------------------------------
 *  these macros/functions, are all independent of the camac controller.
 */




/*
** Need to define getlong and putlong to support the 24 bit operations.
** these are written in an endian independent way as long as you refer to
** VME address space.
*/

static inline long getlong(volatile void* a) {
  uint16_t* p = (uint16_t*)a;
  long    l = p[0];
  l   = l << 16;
  l  |= p[1];
  return l;
}

static inline void putlong(int32_t v,volatile void *a)
{                                      
  volatile short* p = (volatile short*)a;
  *p = (v >> 16) & 0xff;                 
  p++;                                   
  *p = (v & 0xffff);                     
}


#define putbufw(word)	        (*bufpt = (int16_t)(word)); ++bufpt
#define putbufl(l)		{					\
                                  uint32_t tmp = (l);                         \
                                  putbufw((tmp&0xffff));                  \
                                  putbufw((tmp >> 16));                   \
				}

#define savebufpt(var)		var = (int32_t)bufpt
#define rstbufpt(var)		bufpt = ((int16_t *)var)

    /* Bitwise operators. */

#define IAND(a,b)	((a) & (b))
#define IOR(a, b)	((a) | (b))
#define ISHIFT(a, b)	((a) << (b))

    /* Boolean operators */

#define EQ	==
#define NE	!=
#define GT	>
#define GE	>=
#define LT	<
#define LE	<=
#define AND	&&
#define OR	||
#define NOT	!
#ifndef TRUE
#define TRUE	1
#endif
#ifndef FALSE
#define FALSE	0
#endif


    /* Flow of control */

#define IF	if
#define THEN	{
#define ELSE	} else {
#define ENDIF	}
#define DO
#define WHILE(expr) while(expr) {
#define ENDDO	}
#define CALL	

    /* Declarations */

#define LOGICAL	uint32_t
#define WORD	int16_t
#define INTEGER	int32_t
#define REAL	float

    /* code fragment generators: */


#define qstoptobuf(b,c,n,a,f) do {				\
			        putbufw(camread16(b,c,n,a,f));	\
			      } while(qtst(b));			\
			      --bufpt;

#define rdtobuf16(b, c, n, a, f)  putbufw(camread16((b),(c),(n),(a),(f)))
#define rdtobuf24(b, c, n, a, f)  putbufl(camread24((b),(c),(n),(a),(f)))


#define READBIT(b, c, n, a, f, d) d = camread16(b,c,n,a,f);                 \
				  putbufw(d)
    /* Device specific macros */

	/* ORTEC AD811's */

#define INIT811(b, c, n)	camctl((b),(c),(n),12,11); \
				camctl((b),(c),(n),12,26)
#define CLR811(b, c, n)		camctl((b),(c),(n),12,11)
#define READ811(b, c, n, a)	rdtobuf16((b),(c),(n),(a),0)

	/* LRS 22xx series devices */

#define INIT22XX(b, c, n)	camctl((b),(c),(n),0,9); \
				camctl((b),(c),(n),0,26)
#define CLR22XX(b, c, n)	camctl((b),(c),(n),0,9)
#define READ22XX(b, c, n, a)	rdtobuf16((b),(c),(n),(a),0)

	/* LRS feras. */

#define FERA_OFS	0x8000
#define FERA_CLE	0x4000
#define FERA_CSR	0x2000
#define FERA_CCE	0x1000
#define FERA_CPS	0x0800
#define FERA_EEN	0x0400
#define FERA_ECE	0x0200
#define FERA_EPS	0x0100
#define busy(br, cr, sl) {					          \
			fprintf(stderr,                                   \
			        "Module busy during programming: \n");      \
		        fprintf(stderr,					  \
				 "Br = %d Cr = %d Slot = %d \n",          \
			         (br), (cr), (sl));			  \
			sleep(2);				  \
			}
#define INITFERA(b, c, n, cmd, ped)	{ int16_t _sub;	                     \
					  int32_t *_p;			     \
					  do                                 \
                                          {                                  \
					   camctl((b),(c),(n),0,9);          \
					   camwrite16((b),(c),(n),0,16,cmd); \
					   if(!qtst(b))                      \
					      busy((b),(c),(n));	     \
                                          }while (!qtst((b))); 		     \
					  _p = &userints[ped];               \
					  for(_sub=0;_sub<=15; _p++,_sub++)  \
					    do                               \
                                            {                                \
					      camctl((b),(c),(n),0,9);       \
					      camwrite16((b),(c),(n),_sub,   \
                                                          17,*_p);           \
                                              if(!qtst(b))                   \
                                                 busy((b),(c),(n)); 	     \
                                            } while(!qtst(b));               \
				        }
#define CLRFERA(b, c, n)	camctl((b),(c),(n),0, 9)
#define READFERA(b, c, n, a)	rdtobuf16((b),(c),(n),(a),2)
#define READFERAALL(b, c, n)	qstoptobuf((b),(c),(n),0,2)


/* Ganelec? 812F ADCS: */


#define INIT812F(b,c,n, cmd, ped)   	{ int16_t _sub;	                     \
					  int32_t *_p;			     \
					  do                                 \
                                          {                                  \
					   camctl((b),(c),(n),0,9);          \
					   camwrite16((b),(c),(n),0,16,cmd); \
					   if(!qtst(b))                      \
					      busy((b),(c),(n));	     \
                                          }while (!qtst((b))); 		     \
					  _p = &userints[ped];               \
					  for(_sub=0;_sub<=7; _p++,_sub++)  \
					    do                               \
                                            {                                \
					      camctl((b),(c),(n),0,9);       \
					      camwrite16((b),(c),(n),_sub,   \
                                                          17,*_p);           \
                                              if(!qtst(b))                   \
                                                 busy((b),(c),(n)); 	     \
                                            } while(!qtst(b));               \
				        }

    /* The definitions below handle Silena ADC's */

/* initialize Silena 4418 ADC
** Arguments:
**	int b,c,n	- Branch, crate, slot
**	int cmd		- status register
**	int thrsh	- number of first of 25 users integers 
**			    common threshold x1
**			    lower level discriminator x8
**			    offset x8
**			    upper level discriminator x8
*/
#define INIT4418(b,c,n,cmd,thrsh) {				    \
    int16_t i;							    \
    int32_t *_p;							    \
    do{								    \
	camctl(b,c,n,0,9);  /* clear */				    \
	camwrite16(b,c,n,14,20,cmd); /* write status word */	    \
	if(!qtst(b))busy(b,c,n);				    \
    }while(!qtst(b));						    \
    _p =  &userints[thrsh];					    \
    do{								    \
	camwrite16(b,c,n,9,20,*_p); /* write common threashold */  \
	if(!qtst(b))busy(b,c,n);				    \
    }while(!qtst(b));						    \
    ++_p;							    \
	    /* write LLD value */				    \
    for(i=8;i<16;_p++,i++)do{					    \
	camwrite16(b,c,n,i,17,*_p);				    \
	if(!qtst(b))busy(b,c,n);				    \
    }while(!qtst(b));						    \
	    /* write offset value */				    \
    for(i=0;i<8;_p++,i++)do{					    \
	camwrite16(b,c,n,i,20,*_p);				    \
	if(!qtst(b))busy(b,c,n);				    \
    }while(!qtst(b));						    \
	    /* write LLD value */				    \
    for(i=0;i<8;_p++,i++)do{					    \
	camwrite16(b,c,n,i,17,*_p);				    \
	if(!qtst(b))busy(b,c,n);				    \
    }while(!qtst(b));						    \
}

/*
** zero suppressed read.
*/
#define READ4418(b,c,n,a) rdtobuf16((b),(c),(n),(a), 2)
#define RDHDR4418(b,c,n)  camread16((b),(c),(n),14, 2)
#define RDPAT4418(b,c,n)  camread16((b),(c),(n),15, 2)
#define BLKREAD4418(b,c,n) qstoptobuf((b),(c),(n),0, 2)


	/* Generic bit registers
						    */

#define INITBIT(b, c, n)	camctl((b),(c),(n),0,11);                     \
				camctl((b),(c),(n),0,26)
#define CLRBIT(b, c, n)		camctl((b),(c),(n),0,11)
#define RDBIT(b, c, n, a)	rdtobuf16((b),(c),(n),(a),0)

	/* NIM out pattern registers. */

#define NIMOUT(b, c, n, pattern) camwrite16((b),(c),(n),0,16,pattern)

	/* LRS 4448 scalers. */

#define ECLSCL_TEST	0x8000
#define ECLSCL_BD	0x2000
#define ECLSCL_RD	0x0080
#define ECLSCL_CL	0x0040
#define ECLSCL_LD	0X0020
#define ECLSCL_NPOS	8

/* GSI scalers. */
#define INITGSIS(b, c, n)	{ camwrite16((b),(c),(n),0,16,0);   \
    	    	    	    	  camctl((b),(c),(n),0,9); 	    \
    	    	    	    	  camctl((b),(c),(n),0,1); }

#define CLRGSIS(b, c, n)	INITGSIS((b),(c),(n))


#define READALLGSIS(b, c, n)	{ int16_t _a;			    \
    	    	    	    	  INITGSIS((b),(c),(n));    	    \
				  for(_a = 0; _a <=47; _a++) {	    \
				      rdtobuf24((b),(c),(n),0,0);   \
				  }                                  \
			        }

#define INIT4434(b, c, n)	camwrite16((b),(c),(n),0,16, ECLSCL_CL |      \
							     ECLSCL_BD)
#define CLR4434(b, c, n)	INIT4434((b),(c),(n))
#define READ4434(b, c, n, a)	camwrite16((b),(c),(n),0,16,          \
                                                             (ECLSCL_LD) |    \
							     (a));         \
				          rdtobuf24((b),(c),(n),0,0);

#define READALL4434(b, c, n)	{ int16_t _a;				      \
				  camwrite16((b),(c),(n),0,16,                \
                                                             (ECLSCL_LD)  |  \
						      (31 << ECLSCL_NPOS));   \
				 for(_a = 31; _a != -1; _a--)		      \
				 {					    \
                                   putbufl(                                   \
				           camread24((b),(c),(n),0,2));	      \
				 }					      \
                                }

	/* LRS 2551 scalers. */

#define INIT2551(b, c, n)	camctl((b),(c),(n),0,9)
#define CLR2551(b, c, n)	INIT2551((b),(c),(n))
#define READ2551(b, c, n, a)	putbufl(                                     \
				camread24((b),(c),(n),(a),0))

#define READALL2551(b, c, n)	{ int16_t _a;				      \
				  for(_a = 0; _a < 12; _a++)		      \
				       putbufl(                               \
					camread24((b),(c),(n),(_a), 0));	      \
				}

/*
** Handle phillips adc/qdc/tdc:
*/
/*   For the 7164 ADCs:
*     cmd      is the control register
*              bits 1,2,3 are ped, lower thrsh, upper thrsh enable
*              bits 9-16 are conversion delay
*     ped      is the first of 16 integers for pedestals */
#define INIT7164(b,c,n,cmd,ped) {                        \
    int16_t i;                                         \
    int32_t *_p;                                       \
    camctl(b,c,n,0,9);               /* clear */     \
    do{	                                         \
          camwrite16(b,c,n,0,19,cmd);                    \
          if(!qtst(b))busy(b,c,n);                       \
	  }while(!qtst(b));                                \
	  _p = &userints[ped];                             \
	  for(i=0;i<16;_p++,i++)do{                        \
	      camwrite16(b,c,n,0,17,i); /* ped to follow*/ \
            camwrite16(b,c,n,i,20,*_p); /* here it is */ \
            if(!qtst(b))busy(b,c,n);                     \
	      }while(!qtst(b));                              \
        camctl(b,c,n,0,26);                              \
}
#define INIT7186(b,c,n,ip,ilow,iup,ped){                 \
        int16_t i,ibit,icmd=0;                             \
        int32_t *_p;                                       \
        camctl(b,c,n,0,9);              /* clear */      \
        if(ip)icmd |= 1;     /* set pedestals */         \
        if(ilow)icmd |= 2;   /* set lower thresh */      \
        if(iup)icmd |= 4;    /* set upper thresh */      \
        camwrite16(b,c,n,0,19,icmd);                     \
        _p = &userints[ped];                             \
        for(ibit=0;ibit<3; ibit++){ /* ped, low, upper */ \
          if((1 < ibit) & icmd){                         \
            camwrite16(b,c,n,ibit,17,0);                 \
            for(i=0; i<16; _p++,i++)do{                  \
              camwrite16(b,c,n,i,20,*_p);                \
              if(!qtst(b))busy(b,c,n);                   \
            }while(!qtst(b));                            \
          }                                              \
        }                                                \
        camctl(b,c,n,0,26);    /* enable LAM */          \
}	
/*   Reads and clears are straightforward 
 * bits 12-16 contain the address, which we don't want
 * I don't know what a LONGBRANCH is, so I'll
 * follow what precedes this */
#define READ7164(b,c,n,a)  putbufw(camread16((b),(c),(n),(a),0) & 0x0fff)
#define READ7186(b,c,n,a)  putbufw(camread16((b),(c),(n),(a),0))
#define CLR7164(b,c,n) camctl((b),(c),(n),0,9)


	/* Initialize the triggers; */
	/* User trigger 1 is done by a */

#define STARTUSR1TRIG(frequency)	trig1init((frequency))
#define STOPUSR1TRIG			trig1kill

	/* I/O macros */

#define msg(txt)	fprintf (stderr, txt)
#define hexout(val)	fprintf (stderr, "%x", val)
#define decout(val)	fprintf (stderr, "%d", val)
#define newline		fprintf (stderr, "\n")


/* Data packetizing macros:


/*
** Macros for building subevent structures:
*/

		/* Fixed size sub event packet. */

#define Packet(size, type) {                                             \
                                putbufw(size); putbufw(type);            \

#define EndPacket          }

		/* Variable sized sub event packet */

#define VPacket(type)     {                                            \
                            WORD *_pktstart = bufpt; ++bufpt;         \
                            putbufw(type);

#define EndVPacket            *_pktstart = (WORD)(bufpt - _pktstart);     \
                           }


/*
**   Reserve a fixed length chunk of buffer which will be filled in
**   later.  ptr will point to the start of this buffer. 
*/
                     
#define Reserve(ptr, n)   ptr = bufpt; bufpt += (n);




#endif
