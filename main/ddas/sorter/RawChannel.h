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

/** @file:  RawChannel.h
 *  @brief: Define a class to contain a raw channel of data,.
 *  @note:  This can operate either in copy or zero copy mode.
 */
#ifndef RAWCHANNEL_H
#define RAWCHANNEL_H

#include <stdint.h>
#include <stddef.h>

namespace DDASReadout {
    /**
     * @cstruct RawChannel
     *    RawChannel is a struct that has a pointer to the data of a hit
     *    as well as slots for things extracted from the hit.  The
     *    struct can be used in either zero copy or copy mode.  In
     *    Zero copy mode, a channel's data pointer points to some buffer
     *    that may have raw data from more than one hit.  In copy mode,
     *    the data are dynamically allocated to hold the raw hit.
     */
    struct RawChannel {
        uint32_t s_moduleType;              // Type of module this comes from.
        double s_time;                      // Extracted time possibly calibrated.
        int    s_chanid;                    // Channel within module.
        bool   s_ownData;                   // True if we own s_data.
        int    s_ownDataSize;               // if own data how many uint32_t's.
        int    s_channelLength;             // Number of uint32_t in  s_data
        uint32_t* s_data;
        
        RawChannel();
        RawChannel(size_t nWords);
        RawChannel(size_t nWords, void* pZcopyData);
        virtual ~RawChannel();
        int SetTime();
        int SetLength();
        int SetTime(double clockcal, bool useExt=false);
        int SetChannel();
        int Validate(int expecting);
        
        // Zero copy suport:
        
        void setData(size_t nWords, void* pZCopyData);
        
        // Copy new data:
        
        void copyInData(size_t nWords, const void* pData);
        
        // Copy construction and assignment do deep copies if dynamic memory
        
        RawChannel(const RawChannel& rhs);
        RawChannel& operator=(const RawChannel& rhs);
        
        static uint32_t channelLength(void* pData);
        static double moduleCalibration(uint32_t moduleType);
    };
    
}

// Comparison operators -- operate on the timestamp.

bool operator<(const DDASReadout::RawChannel& c1, const DDASReadout::RawChannel& c2);
bool operator>(const DDASReadout::RawChannel& c1, const DDASReadout::RawChannel& c2);
bool operator==(const DDASReadout::RawChannel& c1, const DDASReadout::RawChannel& c2);

// for containers with pointers:



#endif