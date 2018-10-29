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


/* macros.h	- Macro definitions for data acquisition user programming */
/*		  assumed variables: INT16 *buffer -> buffer.		*/
/*		  assumed include files: camac.h			*/

    /* CAMAC and buffer manipulation. */
#ifndef CESMACROS_H
#define CESMACROS_H

#ifdef __unix__
#include <stdio.h>
#include <unistd.h>
#endif

#include <camacmap.h>






#define camread16(b,c,n,a,f)	 (CAMRD16(CBDPTR((b),(c),(n),(a),(f),CAM16)))
#define camread24(b, c, n, a, f) (CAMRD24(CBDPTR((b),(c),(n),(a),(f), CAM24)))

#define camwrite16(b,c,n,a,f,d)	(CAMWR16(CBDPTR((b),(c),(n),(a),(f),CAM16),d))
#define camwrite24(b,c,n,a,f,d)	(CAMWR24(CBDPTR((b),(c),(n),(a),(f),CAM24),d))
#define camctl(b,c,n,a,f)	CAMCTL(CBDPTR((b), (c), (n), (a), (f), CAM16))

#define inhibit(b,c)         camctl(b,c,30,9,26);
#define uninhibit(b,c)       camctl(b,c,30,9,24);



#define qtst(b)	((camread16((b),0,29,0,0) & 0X8000) != 0)
#define xtst(b)	((camread16((b),0,29,0,0) & 0X4000) != 0)


extern void CESBranchAccess(int nBranch);

#define branchinit(b)	CESBranchAccess(b); \
                     camwrite16(b,0,29,0,0, MNOX| MTO | MLAM | MIT2 | MIT4);\
		     camwrite16(b,0,29,0,4, 0);			       \
		     camwrite16(b,0,29,0,9, 0);			       \
		     sleep(2)

#define crateinit(b, c)	while((camread16(b,0,29,0,9) & (1 << c)) == 0)        \
		        {						      \
			  fprintf(stderr, "Branch %d Crate %d is offline\n",  \
                                   b,c);                                      \
		          sleep(5);                                           \
                        }                                                     \
			camctl(b,c,28,9,26);                                  \
			camctl(b,c,28,8,26);				      \
			camctl(b,c,30,9,24)


#define BEGINLAM(br,cr)        {                                         \
                          UINT16 _b,_c;                                  \
			  INT16 _timout;				 \
			  UINT32 _lammsk[br][cr];                        \
			  UINT16 MAXBR = br;				 \
			  UINT16 MAXCR = cr;				 \
									 \
			  for(_b = 0; _b<br; _b++)                        \
                            for(_c = 0; _c < cr; _c++) _lammsk[_b][_c] = 0;

#define ENDLAM	}

#define NEEDLAM(b, c, n)	_lammsk[b][c-1] |= (1 << (n-1))

#define IFTIMEOUT(maxloop) _timout = maxloop;     /* maxloop in ~10us */      \
                           _b = 0; _c = 0;                                    \
                           while(_timout >= 0)                                \
                           {                                                  \
                             if(_lammsk[_b][_c] == 0)                          \
                             {                                                \
                               _c++;                                          \
			       if(_c >= MAXCR)                                \
                               {                                              \
                                 _c = 0;                                      \
                                 _b++;                                        \
                                 if(_b >= MAXBR) break;                       \
                               }                                              \
                              } else                                          \
                                {                                             \
                                  _timout--;                                   \
                                  _lammsk[_b][_c] &= ~(_lammsk[_b][_c] &      \
                                                   camread24(_b,_c+1,30,0,0));\
                                }                                             \
                           }                                                  \
                           if(_timout < 0)

#define LAMREAD(b,c)   camread24((b), (c), 30, 0, 0)






#endif
