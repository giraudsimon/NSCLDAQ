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

/** @file:  CAENPHABuffer.cpp
 *  @brief: Implement the PHA buffering utilities.
 */
#include "CAENPhaBuffer.h"
#include <string.h>

/**
 * CAENPhaBuffer::putWord
 *    Put a word into the buffer.
 *
 * @param pDest - pointer to where the word goes
 * @param data  -uint16_t to put.
 * @return void* Pointer to the next free slot in the buffer.
 */
void*
CAENPhaBuffer::putWord(void* pDest, uint16_t data)
{
    uint16_t* p = reinterpret_cast<uint16_t*>(pDest);
    *p++ = data;
    
    return p;
}
/**
 * putLong
 *  Put a 32 bit long into the buffer.
 *
 * @param pDest - pointer to where the data goes.
 * @param data  - uint32_t to put.
 * @return void* - pointer to the next free slot in the buffer.
 */
void*
CAENPhaBuffer::putLong(void* pDest, uint32_t data)
{
    uint32_t* p = reinterpret_cast<uint32_t*>(pDest);
    *p++ = data;
    return p;
}

/**
 * putQuad
 *    Put a 64 bit quadword into the buffer.
 *
 * @param pDest - pointer to where the data goes.
 * @param data  - the data.
 * @return void* - points to the next free slot in the buffer.
 */
void*
CAENPhaBuffer::putQuad(void* pDest, uint64_t data)
{
    uint64_t* p = reinterpret_cast<uint64_t*>(pDest);
    *p++ = data;
    return p;
}


/**
 * putDppData
 *    Puts the DPP data into the event buffer.
 * @param pDest - where to put the DPP Data.
 * @param dpp   - Reference to the dpp data.
 * @return - pointer to the next free slot in the buffer.
 */
void*
CAENPhaBuffer::putDppData(void* pDest, const CAEN_DGTZ_DPP_PHA_Event_t& dpp)
{
    pDest = putQuad(pDest, (dpp.TimeTag));
    pDest = CAENPhaBuffer::putWord(pDest, (dpp.Energy));
    pDest = CAENPhaBuffer::putWord(pDest, (dpp.Extras));
    pDest = putLong(pDest, (dpp.Extras2));
    
    return pDest;
}

/**
 * putWfData
 *    Put the waveform data to the output buffer.
 *    We put the following information:
 *    -   ns - number of samples (32 bits)
 *    -   dt - Non zero if ther are two traces (16 bits).
 *             note that we widen this in order to make alignment better.
 *    -   First trace if ns > 0 ns*16 bits.
 *    -   Second trace if ns > 0 && dual trace  ns * 16 bits.
 *
 * @param pDest - pointer to where the data must go
 * @param wf    - Reference to CAEN_DGTZ_PHA_Waveforms_t - the decoded waveforms.
 * @return void* - Pointer to the next free memory of the buffer (where next data goes).
 */
void*
CAENPhaBuffer::putWfData(void* pDest, const CAEN_DGTZ_DPP_PHA_Waveforms_t& wf)
{
  pDest = putLong(pDest, wf.Ns);                            // # samples.
  pDest = CAENPhaBuffer::putWord(pDest, wf.DualTrace);
    size_t nBytes = wf.Ns * sizeof(uint16_t);
    if (nBytes > 0) {                                   // There are waveforms:
      uint8_t* p = reinterpret_cast<uint8_t*>(pDest);
        memcpy(p, wf.Trace1, nBytes);               // Write trace 1...
        p +=  nBytes;
        
        if (wf.DualTrace) {                            // Write second trace too:
            memcpy(p, wf.Trace2, nBytes);
            p += nBytes;
        }
        pDest = p;
    }
    return pDest;
}

/**
 * computeEventSize
 *    Figure out how big the event is, in bytes, including a 32 bit event size.
 *
 *  @param dppInfo - reference to the CAEN_DGTZ_PHA_Event_t containing the event.
 *  @param wfInfo  - reference to the CAEN_DGTZ_PHA_Waveforms_t containing decoded waveforms.
 *  @return size_t - Number of bytes the event will require in the data stream.
 */
size_t
CAENPhaBuffer::computeEventSize(
    const CAEN_DGTZ_DPP_PHA_Event_t& dppInfo, const CAEN_DGTZ_DPP_PHA_Waveforms_t& wfInfo
)
{
    size_t result = sizeof(uint32_t);               // Size of event
    
    // The dpp info is a time tag (uint64_t), E, extras, (16 bits )and
    // extras2 (32 bits)):
    
    result += sizeof(uint64_t)   +                    // Time tag.
              2*sizeof(uint16_t) +                    // E, Extras.
              sizeof(uint32_t);                       // Extras2.
              
    // Waveform data size is a bit more complex.  See the putWfData
    // method for considerations.
    
    result += sizeof(uint32_t) + sizeof(uint16_t);  // Ns, and dualtrace flag.
    if (wfInfo.Ns) {
        // If there are samples, there's always at least one trace of 16 bit words:
        
      size_t nBytes = wfInfo.Ns * sizeof(uint16_t);
      result += nBytes;
      
      //  If dual trace there's a second one:
      
      if (wfInfo.DualTrace) {
        result += nBytes;
      }
    }
    return result;
}
