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

/** @file:  fragcmdstubs.h
 *  @brief: Stubs to support fragment handler command tests.
 */

#ifndef FRAGCMDSTUBS_H
#define FRAGCMDSTUBS_H
#include "fragment.h"

class CFragmentHandler
{
public:
    unsigned long      m_nLastSize;
    const EVB::FlatFragment* m_pLastFrags;
   static CFragmentHandler* m_pInstance;
public:
    void addFragments(unsigned long nFrags, const EVB::FlatFragment* pFrags);
    static CFragmentHandler* getInstance();
};

#endif
