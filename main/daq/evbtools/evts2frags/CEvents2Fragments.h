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

/** @file:  CEvents2Fragments.h
 *  @brief: Application level class for prepending fragment hdrs to ring items.
 */

#ifndef CEVENTS2FRAGMENTS_H
#define CEVENTS2FRAGMENTS_H

#include <CRingFileBlockReader.h>
#include <CBufferedOutput.h>

// Forward class definitions.

class CFragmentMaker;





/**
 * @class CEvents2Fragments
 *    Takes an input stream of ring items and produces an output stream
 *    of ring items preceded by 'appropriate' fragment headers.  The output
 *    stream is, therefore, a faithful imitation of the output of an event
 *    builder.
 *
 * There's one tunable bit; the blocksize of the reads done on the input
 * stream.
 */

class CEvents2Fragments
{
private:
    int m_nReadSize;                // passsed to CRingFileBlockReader::read
    CRingFileBlockReader&     m_Reader;
    CFragmentMaker&           m_HeaderMaker;
    io::CBufferedOutput&      m_Writer;
public:
    CEvents2Fragments(
        int readSize, CRingFileBlockReader& reader, CFragmentMaker& headerMaker,
        io::CBufferedOutput& writer
    );
    virtual ~CEvents2Fragments();
    
    void operator()();
    
private:
    void processBlock(CRingFileBlockReader::DataDescriptor& desc);
    int  processItem(void* pItem);
    bool runEnded();
};

#endif