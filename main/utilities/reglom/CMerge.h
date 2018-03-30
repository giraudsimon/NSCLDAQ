/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CMerge.h
 *  @brief: Header for the merge sort class.
 */
#ifndef CMERGE_H
#define CMERGE_H

#include <stdio.h>
#include <vector>
#include <cstdint>

class CDataSource;
class CRingItem;


/**
 * CMerge  This class does a merge sort on the data in the data sources.
 * Data are then output to the file descriptor that's passsed in.  This
 * results in a totally ordered data stream which $DAQBIN/glom can then build
 * into events on the other end.
 *
 * Note that we support two barrier types.  Begin run (barrier type 1)
 * and End run (barrier type 2).
 *
 * The data sources are assumed  to be complete runs.  If necessary,
 * the raw event files fed to unglom can be contanated together to ensure this.
 * Thus we require:
 *     - Each source file starts with a begin run item.
 *     - Each source file ends with an end run item.
 *
 * That makes barrier synchronization simple.  We sync at the start and hold
 * the end file items until all sources have one buffered up.
 *
 * If we have a begin without one, we complain.
 * If we have an end without one, we complain.
 *
 * None of those are fatal:
 * Missing begin, we just output the begins we have.
 * Missing ends,  we just output the ends we have.
 */

class CMerge
{
public:
    typedef struct _sourceInfo {
        CRingItem*      s_pItem;         // most recent ring item from source.
        std::uint64_t   s_thisStamp;     // Timestamp of current item.
        std::uint64_t   s_lastStamp;     // Last timestamp not NULL_TIMESTAMP.
    } sourceInfo, *pSourceInfo;
private:
    FILE*                       m_pOutput;
    std::vector<CDataSource*>   m_dataSources;
    pSourceInfo                 m_sources;
    
    
public:
    CMerge(FILE* output, std::vector<CDataSource*> sources);
    virtual ~CMerge();
    
    void operator()();
    
private:
    void  Begin();
    void  outputOldest();
    void  End();
    bool  atEnd();
    void  outputFragment(unsigned sourceIndex);
    void  readFragment(unsigned sourceIndex);

};


#endif