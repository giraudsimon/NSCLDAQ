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

/** @file:  ModuleReader.h
 *  @brief: Provides a class that can read data from a Pixie16 module.
 *  
 */
#ifndef MODULEREADER_H
#define MODULEREADER_H

#include "BufferArena.h"
#include <deque>
#include <stdint.h>


namespace DDASReadout {

class ZeroCopyHit;
/**
 * @class ModuleReader
 *     This class provides a minimal copy module reader for Pixie16 modules.
 *     It tries to provide for minimal copy readout from the module by
 *     maintining a BufferArena into which data are read.  Data are
 *     then parsed into ZeroCopyHits.  Pointers to these ZeroCopyHits are
 *     placed into dequeues and made available to the caller.
 *
 *     Note that the zero copy hits themselves can be recycled.
 *     
 */

class ModuleReader {
public:
    typedef std::pair<ModuleReader*, ZeroCopyHit*> HitInfo;
    typedef std::deque<HitInfo>       HitList;
private:
    typedef std::deque<ZeroCopyHit*>  HitPool;
private:
    unsigned        m_nModuleNumber;            // Module index in crate
    unsigned        m_nExpectedEventLength;     // From ModEvLen.txt e.g.
    double          m_tsMultiplier;             // raw timestamps -> ns multiplier
    BufferArena     m_freeBuffers;              // Storage comes from here.
    HitPool         m_freeHits;
    double          m_lastStamps[16];           // last timestamps for each channel.
public:
    uint32_t        m_moduleTypeWord;
    // canonicals.
    
public:
    ModuleReader(
        unsigned module, unsigned evtlen, uint32_t moduleType,
        double timeMultiplier = 1.0
    );
    virtual ~ModuleReader();
private:
    ModuleReader(const ModuleReader& rhs);
    ModuleReader& operator=(const ModuleReader& rhs);
public:    
    size_t read(HitList& hits, size_t nWords);
    static void freeHit(HitInfo& hit);
    unsigned module() const { return m_nModuleNumber; }
    void reset();
private:
    void parseHits(HitList& hits, ReferenceCountedBuffer& pBuffer, size_t nUsedWords);
    ZeroCopyHit* allocateHit();
    void checkOrder(ZeroCopyHit* pHit);
};
 
}                            // Namespace.

#endif