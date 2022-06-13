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

/** @file:  fragcmdstubs.cpp
 *  @brief: Implement fragment command stubs.
 */
#include "fragcmdstubs.h"

CFragmentHandler* CFragmentHandler::m_pInstance(nullptr);

void
CFragmentHandler::addFragments(unsigned long nFrags, const EVB::FlatFragment* pFrags)
{
    m_nLastSize = nFrags;
    m_pLastFrags = pFrags;
}

CFragmentHandler*
CFragmentHandler::getInstance()
{
    if (!m_pInstance) {
        m_pInstance = new CFragmentHandler;
    }
    return m_pInstance;
}
