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

/** @file:  ModuleSettings.h
 *  @brief: Capture the settings for a module in a nice struct of a package.
 */
#ifndef MODULESETTINGS_H
#define MODULESETTINGS_H
#include <stdint.h>
#include <string>
#include <vector>

namespace DDAS {
    
    
    /**
     *  The settings consist of scalars that are per module
     *  and 16 element arrays that consist of per channel settings:
     */
    static const unsigned CHANNELS(16);
    
    struct ModuleSettings {
        //  Per module parameters:
        
        unsigned int s_csra;          /* CSRA Bitmask    */
        unsigned int s_csrb;          /* CSRB Bitmask    */
        unsigned int s_format;        /* Module format   */
        unsigned int s_maxEvents;     /* Module max events */
        bool         s_synchWait;     /* Wait for synchronization */
        bool         s_inSynch;       /* Synchronized   */
        unsigned int s_SlowFilterRange;
        unsigned int s_FastFilterRange;
        unsigned int s_FastTrgBackPlaneEnables;
        unsigned int s_trigConfig0;
        unsigned int s_trigConfig1;
        unsigned int s_trigConfig2;
        unsigned int s_trigConfig3;
        unsigned int s_HostRtPreset;
        
        // Per channel parameters... these are all doubles.
        
        double s_triggerRiseTime[CHANNELS];
        double s_triggerFlattop[CHANNELS];
        double s_triggerThreshold[CHANNELS];;
        double s_energyRiseTime[CHANNELS];
        double s_energyFlattop[CHANNELS];
        double s_tau[CHANNELS];
        double s_traceLength[CHANNELS];
        double s_traceDelay[CHANNELS];
        double s_vOffset[CHANNELS];
        double s_Xdt[CHANNELS];
        double s_BaselinePct[CHANNELS];
        uint32_t s_Emin[CHANNELS];
        uint32_t s_binFactor[CHANNELS];
        uint32_t s_baselineAverage[CHANNELS];
        uint32_t s_chanCsra[CHANNELS];
        uint32_t s_chanCsrb[CHANNELS];
        double s_blCut[CHANNELS];
        double s_fastTrigBackLen[CHANNELS];
        double s_CFDDelay[CHANNELS];
        double s_CFDScale[CHANNELS];
        double s_CFDThreshold[CHANNELS];
        double s_QDCLen0[CHANNELS];
        double s_QDCLen1[CHANNELS];
        double s_QDCLen2[CHANNELS];
        double s_QDCLen3[CHANNELS];
        double s_QDCLen4[CHANNELS];
        double s_QDCLen5[CHANNELS];
        double s_QDCLen6[CHANNELS];
        double s_QDCLen7[CHANNELS];
        double s_extTrigStretch[CHANNELS];
        double s_vetoStretch[CHANNELS];
        uint32_t s_multiplicityMaskL[CHANNELS];
        uint32_t s_multiplicityMaskH[CHANNELS];
        double s_externDelayLen[CHANNELS];
        double s_FTrigoutDelay[CHANNELS];
        double s_chanTriggerStretch[CHANNELS];
    };

    /**
     *    A slot is a slot number, its event length
     *    its configuration filename and its settings:
     */
    struct Slot {
        unsigned s_slotNum;
        ModuleSettings s_settings;
    };
    /**
     * A crate is a crate ID, the crate configuration file and
     * a vector of slots.
     */
    struct Crate {
      unsigned s_crateId;
      std::vector<Slot> s_slots;
    };
}                                   // Namespace DDAS

#endif
