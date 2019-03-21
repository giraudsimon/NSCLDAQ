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

/** @file:  CBufferedFragmentReader.h
 *  @brief: Read blocks of data and provide fragments from them.
 */
#ifndef CBUFFEREDFRAGMENTREADER_H
#define CBUFFEREDFRAGMENTREADER_H

namespace EVB {
typedef struct _FlatFragment FlatFragment, *pFlatFragment; 
}

/**
 * CBufferedFragmentReader
 *    Returns a stream of flat fragments from a buffered read.
 *    note that pointers to the fragment are returned rather than
 *    copying them.
 */
class CBufferedFragmentReader
{
public:
    CBufferedFragmentReader(int fd);
    virtual ~CBufferedFragmentReader();
    
    const EVB::pFlatFragment getFragment();
};

#endif