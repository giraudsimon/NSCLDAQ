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

/** @file:  CAENBuffer.h
 *  @brief: Utilities to put data into buffers.
 */
#ifndef CAENPHABUFFER_H
#define CAENPHABUFFER_H
#include <CAENDigitizerType.h>
#include <stdint.h>

/**
 * This namespace contains utilities to put stuff
 * into buffers.
 */
namespace CAENPhaBuffer {
    void*  putWord(void* pDest, uint16_t data);
    void*  putLong(void* pDest, uint32_t data);
    void*  putQuad(void* pDest, uint64_t data);
    void*  putDppData(void* pDest, const CAEN_DGTZ_DPP_PHA_Event_t& dpp);
    void*  putWfData(void* pDest, const CAEN_DGTZ_DPP_PHA_Waveforms_t& wf);
    size_t computeEventSize(
        const CAEN_DGTZ_DPP_PHA_Event_t& dppInfo,
        const CAEN_DGTZ_DPP_PHA_Waveforms_t& wfInfo
    );
        
}

#endif