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

/** @file:  DDASSorter.h
 *  @brief: Define the class that does all the sorting.
 */
#ifndef DDASSORTER_H
#define DDASSORTER_H

class CRingBuffer;


class DDASSorter
{
private:
    CRingBuffer&  m_source;
    CRingBuffer&  m_sink;
    
public:
    DDASSorter(CRingBuffer& source, CRingBuffer& sink);
    
    void operator()();
};


#endif