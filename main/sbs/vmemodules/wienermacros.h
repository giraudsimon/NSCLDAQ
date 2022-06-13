/* This software was contributed by Dave Caussyn of Florida State
   University.
*/

/*
 * macros_vc32.h  - Macro definitions for data acquisition user programming
 *                  assumed variables: INT16 *buffer -> buffer.  Modifications
 *                  to macros.h for using Wiener vc32.
 */


    /* CAMAC and buffer manipulation. */

#ifndef WIENERMACROS_H  
#define WIENERMACROS_H


#include <stdio.h>
#include <unistd.h>
#include <camac.h>




/*All definitions referring to CBDPTR are changed from macros.h--ddc */
#define camread16(b,c,n,a,f)     (CAMRD16(CBDPTR((b), (c),(n),(a),(f), CAM16)))
#define camread24(b, c, n, a, f) (CAMRD24(CBDPTR((b), (c),(n),(a),(f), CAM24)))
#define camread32(b, c, n, a, f) (CAMRD32(CBDPTR((b), (c),(n),(a),(f), CAM24)))

#define camwrite16(b,c,n,a,f,d)  (CAMWR16(CBDPTR((b), (c),(n),(a),(f), CAM16),d))
#define camwrite24(b,c,n,a,f,d)  (CAMWR24(CBDPTR((b), (c),(n),(a),(f), CAM24),d))


/* 
camctl...
unfortunately, the prior version of camctl assumed that a READ was
was always being done.  Even though there is (or should be!) NO data for 
commands that use "camctl", the wiener VC32-CC32 depends on the type of 
access to determine the actual function that it executes (reading for 
F<16, write for F>=16). So, we replace the camctl macro. --ddc
*/ 

/* #define camctl(b,c,n,a,f)        CAMCTL(CBDPTR((b), (c), (n), (a), (f), CAM16)) */
#define camctl(b,c,n,a,f)  {  \
            if(f<16){ \
                if( *(volatile INT16 *)CBDPTR((b), (c),(n),(a),(f), CAM16) ){}; \
            } else *(volatile INT16 *)CBDPTR((b), (c),(n),(a),(f), CAM16)=0;    \
	}

#define inhibit(b,c)    camctl(b,c,27, 0, 26)  
#define uninhibit(b,c)  camctl(b,c,27, 1, 24)


   /*Wiener cc32 uses NAF of 0,0,0 to get status word --ddc */
  /*NOTE! BRANCH is not used any longer,  specify crate */
#define qtst(c)       ((camread16(0,(c),0,0,0) & 0x08) != 0)
#define xtst(c)       ((camread16(0,(c),0,0,0) & 0X04) != 0)

/*Until my next comment... there are no changes from macros.h --ddc*/
    


/*
 * Ok... obviously branchinit and crateinit are largely different.
 * The "branchinit" we use to initialize the vc32 interface(s).  The
 * branch number is irrelevent, but each vc32 has a "number" same as
 * crate number. --ddc
 */

 extern void WIENERBranchAccess(int branch);

#define AR 0x1000             /* turns on autoread ... NORMALLY OFF! */
#define ROAK 0x0800           /* ROAK mode, RORA mode if zero */
#define VME_LAM_INT 0x0000    /* If vme int generated for LAMS (bits8-11) */
#define VME_INT_VEC 0x0000    /* If vme int vector for LAMS (bits0-7) */
#define branchinit(b)  WIENERBranchAccess(b);  \
                      camwrite16(b,b+1,0,0,3,VME_LAM_INT|VME_INT_VEC);     \
                       camctl(b,b+1,31,0,16);                              \
                     sleep(2)

/* NAF (0,3,16) Z+I, (0,2,16) C+Uninhibit, (28,1,16,0) set lam mask */
 
#define crateinit(b, c) while((camread16(b,c,0,0,3) & 0x4000) == 0)    \
                      {                                                \
                        fprintf(stderr, "Crate %d is offline\n",c);    \
                        sleep(5);                                      \
                      }                                                \
                     camwrite24(b,c,28,1,16,0);                        \
                     camctl(b,c,0,3,16);                               \
                     camctl(b,c,0,2,16)



#define BEGINLAM(br,cr) {                                                 \
                          UINT16 _b,_c;                                   \
                          INT16 _timout;                                  \
                          UINT32 _lammsk[br][cr];                         \
                          UINT16 MAXBR = br;                              \
                          UINT16 MAXCR = cr;                              \
                                                                          \
                          for(_b = 0; _b<br; _b++)                        \
                          for(_c = 0; _c < cr; _c++) _lammsk[_b][_c] = 0;

#define ENDLAM       }

#define NEEDLAM(b, c, n)       _lammsk[b][c-1] |= (1 << (n-1))

#define IFTIMEOUT(maxloop) _timout = maxloop;     /* maxloop in ~10us */      \
                           _b = 0; _c = 0;                                    \
                           while(_timout >= 0)                                \
                           {                                                  \
                             if(_lammsk[_b][_c] == 0)                         \
                             {                                                \
                               _c++;                                          \
                               if(_c >= MAXCR)                                \
                               {                                              \
                                 _c = 0;                                      \
                                 _b++;                                        \
                                 if(_b >= MAXBR) break;                       \
                               }                                              \
                             } else                                           \
                               {                                              \
                                  _timout--;                                  \
                                  _lammsk[_b][_c] &= ~(_lammsk[_b][_c] &      \
                                                   camread24(_b,_c+1,28,4,0));\
                                }                                             \
                           }                                                  \
                           if(_timout < 0)

#define LAMREAD(b,c)   camread24((b), (c), 28, 4, 0)

/* LAM read commands in macros changed to NAF 28,4,0 --ddc */



/*
** Handle phillips adc/qdc/tdc:
*/

       /* I/O macros */


/*#endif  * endif for MACROS_H defined */
#endif         /*  _WIENERMACROS_H */
