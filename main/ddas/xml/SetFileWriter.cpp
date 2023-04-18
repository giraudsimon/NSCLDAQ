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

/** @file:  SetFileWriter.cpp
 *  @brief: Implement the DDAS::SetFileWriter class.
 */
#include "SetFileWriter.h"
#include "SetFileEditor.h"
#include "ModuleSettings.h"
#include <math.h>
#include <string.h>
#include <config.h>
#include <config_pixie16api.h>

#include <stdexcept>
#include <sstream>

namespace DDAS {
/**
 * constructor
 *    @param m_editor - references a set file m_editor.
 *    @param slot   - Slot we are writing.
 *    @param Mhz    - Speed of the module in MHz. Needed to
 *                    convert from internal -> module parameters.
 */
SetFileWriter::SetFileWriter(
    SetFileEditor& editor, unsigned short slot,
    unsigned short Mhz
) : m_editor(editor), m_slot(slot), m_Mhz(Mhz)
{
}

/**
 * write
 *   Writes the settings described by the parameter to the
 *   slot of the set file we've been given.
 *
 * @param dspSettings - Settings information.
 */
void
SetFileWriter::write(const ModuleSettings& dspSettings)
{
    writeModuleSettings(dspSettings);
    writeChannelSettings(dspSettings);
}
/**
 * writeModuleSettings
 *    Writes the module global settings to the set file.
 *    These can affect channel level parameters but don't
 *    interact internally.  These values are also unit-less
 *    and don't depend on module type.
 *  @param dspSettings - module settings struct.
 */
void
SetFileWriter::writeModuleSettings(const ModuleSettings& dspSettings)
{
     m_editor.set(m_slot, "ModCSRA", dspSettings.s_csra);
     m_editor.set(m_slot, "ModCSRB", dspSettings.s_csrb);
     m_editor.set(m_slot, "ModFormat", dspSettings.s_format);
     m_editor.set(m_slot, "MaxEvents", dspSettings.s_maxEvents);
     m_editor.set(m_slot, "SynchWait", dspSettings.s_synchWait);
     m_editor.set(m_slot, "InSynch",   dspSettings.s_inSynch);
     m_editor.set(m_slot, "SlowFilterRange", dspSettings.s_SlowFilterRange);
     m_editor.set(m_slot, "FastFilterRange", dspSettings.s_FastFilterRange);
     m_editor.set(
        m_slot, "FastTrigBackplaneEna",
        dspSettings.s_FastTrgBackPlaneEnables
    );
    uint32_t trigConfig[4];
    trigConfig[0] = dspSettings.s_trigConfig0;
    trigConfig[1] = dspSettings.s_trigConfig1;
    trigConfig[2] = dspSettings.s_trigConfig2;
    trigConfig[3] = dspSettings.s_trigConfig3;
    m_editor.setChanPar(m_slot, "TrigConfig", trigConfig);
    m_editor.set(m_slot, "HostRunTimePreset", dspSettings.s_HostRtPreset);
}
/**
 * writeChannelSettings
 *    This is the hard one.  We're going to use the Pixie16WriteSglChanPar
 *    to figure out how all this goes.  There _are_ per channel parameters
 *    that depend on the module parameters as well (e.g. the FIFO depth
 *    depends on the maxEvents).  We'll figure those out as well in this section.
 *
 * @param dspSettings - the DSP settings to write.
 * @note The module speed figures in many of these computations.
 */
void
SetFileWriter::writeChannelSettings(const ModuleSettings& dspSettings)
{
    writeTriggerRiseTime(
        dspSettings.s_triggerRiseTime, dspSettings.s_FastFilterRange
    );
    writeTriggerFlattop(
        dspSettings.s_triggerFlattop, dspSettings.s_FastFilterRange
    );
    writeTriggerThresholds(dspSettings.s_triggerThreshold);
    writeVoffsets(dspSettings.s_vOffset);
    writeXDTs(dspSettings.s_Xdt);
    writeBaselines(dspSettings.s_BaselinePct);
    m_editor.setChanPar(m_slot, "EnergyLow", dspSettings.s_Emin); // no conversion
    writeBinFactor(dspSettings.s_binFactor);
    writeBaselineAverages(dspSettings.s_baselineAverage);
    m_editor.setChanPar(m_slot, "ChanCSRa", dspSettings.s_chanCsra);
    m_editor.setChanPar(m_slot, "ChanCSRb", dspSettings.s_chanCsrb);
    setIntFromDoubles("BLcut", dspSettings.s_blCut);
    writeFastTriggerBacklens(dspSettings.s_fastTrigBackLen);
    writeCFDDelays(dspSettings.s_CFDDelay);
    setIntFromDoubles("CFDScale", dspSettings.s_CFDScale, CFDSCALE_MAX);
    setIntFromDoubles(
        "CFDThresh", dspSettings.s_CFDThreshold, CFDTHRESH_MAX
    );
    writeQDCLen("QDCLen0", dspSettings.s_QDCLen0);
    writeQDCLen("QDCLen1", dspSettings.s_QDCLen1);
    writeQDCLen("QDCLen2", dspSettings.s_QDCLen2);
    writeQDCLen("QDCLen3", dspSettings.s_QDCLen3);
    writeQDCLen("QDCLen4", dspSettings.s_QDCLen4);
    writeQDCLen("QDCLen5", dspSettings.s_QDCLen5);
    writeQDCLen("QDCLen6", dspSettings.s_QDCLen6);
    writeQDCLen("QDCLen7", dspSettings.s_QDCLen7);
    writeExtTrigStretch(dspSettings.s_extTrigStretch);
    writeClockScaledWithLimits(
        "VetoStretch", dspSettings.s_vetoStretch,
        VETOSTRETCH_MIN, VETOSTRETCH_MAX
    );
    m_editor.setChanPar(m_slot, "MultiplicityMaskH", dspSettings.s_multiplicityMaskH);
    m_editor.setChanPar(m_slot, "MultiplicityMaskL", dspSettings.s_multiplicityMaskL);
    writeClockScaledWithLimits(
        "ExternDelayLen", dspSettings.s_externDelayLen,
        EXTDELAYLEN_MIN, EXTDELAYLEN_MAX_REVBCD   // Have to guess max.
    );
    writeClockScaledWithLimits(
        "FtrigoutDelay", dspSettings.s_FTrigoutDelay,
        FASTTRIGBACKDELAY_MIN, FASTTRIGBACKDELAY_MAX_REVBCD // guessing.
    );
    writeClockScaledWithLimits(
        "ChanTrigStretch", dspSettings.s_chanTriggerStretch,
        CHANTRIGSTRETCH_MIN, CHANTRIGSTRETCH_MAX
    );
    writeTaus(dspSettings.s_tau);
    
    // Energy filter parameters set things the trace delays depend on.
    // There's a circular dependency so do these twice in hopes
    // that converges
    
    writeERise(dspSettings.s_energyRiseTime);   // slow_length.
    writeEFlattop(dspSettings.s_energyFlattop);  // slow_gap.
    
    writeERise(dspSettings.s_energyRiseTime);   // slow_length.
    writeEFlattop(dspSettings.s_energyFlattop);  // slow_gap.
      
    // DO the trace parameters.
    
    writeTraceLengths(dspSettings.s_traceLength);
    writeTraceDelays(dspSettings.s_traceDelay);   
    


}
/**
 * writeTriggerRiseTime
 *    Depends on the module speed and fast filter range.
 * @param risetimes  - 16 rise times in usec.
 * @param ffrange    - the log2 of the fast filter range (integerized).
 */
void
SetFileWriter::writeTriggerRiseTime(const double* riseTimes, uint32_t ffrange)
{
    // The ffrange multipler depends on the speed:
    
    double mult = clkMult();
    
    double denom = mult *pow(2.0, ffrange);
    uint32_t settings[16];
    for (int i =0; i < 16; i++) {
      settings[i] = round(riseTimes[i]*m_Mhz/denom);
    }
    m_editor.setChanPar(m_slot, "FastLength", settings);
    
    // Note that the FastLength and FastGap are coupled.
    // We'll take that into account when we set the fast gap (trigger flattop).
    
}
/**
 * writeTriggerFlattop
 *    @note this assumes writeTriggerRiseTime has happened.
 *   Compute the fast gap (trigger flattop) value. Note that
 *   the filter rise time (fast length) and gap are
 *   coupled.  This method takes that into account assuming
 *   that writeTriggerRiseTime has already happened so the set file
 *   has a valid value for those parameters
 *
 * @param ftops - pointer to 16 trigger flattop (fast gap) parameters.
 * @param ffrange - the fast filter range value.
 */
void
SetFileWriter::writeTriggerFlattop(const double* ftops, uint32_t ffrange)
{
    // pull out the fast lenghts, we'll need them:
    
    std::vector<uint32_t> fls = m_editor.getChanPar(m_slot, "FastLength");
    std::vector<uint32_t>  gaps;
    
   
    double denom;
    double mult = clkMult();
    denom = mult *pow(2.0, ffrange);
    for (int i = 0; i < 16; i++) {
        uint32_t fl = fls[i];
        uint32_t fg = uint32_t(round(ftops[i] * m_Mhz/denom));
        
        // Adjust based on limits and so on:
        
        if ((fl+fg) > FASTFILTER_MAX_LEN) {
            fl = FASTFILTER_MAX_LEN - fg;
        }
        if (fl < MIN_FASTLENGTH_LEN) {
            fl = MIN_FASTLENGTH_LEN;
            if ((fl + fg) > FASTFILTER_MAX_LEN) {
                fg = FASTFILTER_MAX_LEN - MIN_FASTLENGTH_LEN;
            }
        }
        
        fls[i] = fl;
        gaps.push_back(fg);
    }
    m_editor.setChanPar(m_slot, "FastLength", fls.data());
    m_editor.setChanPar(m_slot, "FastGap", gaps.data());
    
}
/**
 * writeTriggerThreshold
 *    Scale and limit the trigger thresholds.
 *
 *    @param ts - 16 trigger thresholds.
 *    @note we assume the Fast Lengths have been finalized.
 */
void
SetFileWriter::writeTriggerThresholds(const double* ts)
{
    
    double mult = clkMult();
    std::vector<uint32_t> settings;
    std::vector<uint32_t> fls = m_editor.getChanPar(m_slot, "FastLength");
    
    for (int i =0; i < 16; i++) {
        uint32_t thresh = uint32_t(ts[i] * fls[i]) * mult;
        if (thresh >= FAST_THRESHOLD_MAX) {
            thresh = (uint32_t)((double)FAST_THRESHOLD_MAX/((double)fls[i]  - 0.5) * fls[i]);                                            
        }
        settings.push_back(thresh);
        
    }
    m_editor.setChanPar(m_slot, "FastThresh", settings.data());
}
/**
 *  writeVoffsets
 *     Compute and write the voltage offsets.
 *  @param voffs - pointer to the voltage offset values:
 */
void SetFileWriter::writeVoffsets(const double* voffs)
{
    std::vector<uint32_t> offs;
    for  (int i =0; i < 16; i++) {
        uint32_t o = (uint32_t)(65536.0*(voffs[i]/DAC_VOLTAGE_RANGE + 1.0/2.0));
        offs.push_back(o);
    }
    m_editor.setChanPar(m_slot, "OffsetDAC", offs.data());
}
/**
 * writeXDTs
 *    Write channel XDTs.  This is a bit complicated by the fact that in the
 *    code to set the XDTs from a microsecond value, there's a dependency
 *    on whether this is larger or smaller than the prior value.
 *    We can't actually do that because we don't know the prior value.
 *    We'll therefore implement the code that assumes this is a larger value.
 *  
 * @param xdts - The 16 microsecond XDT values to use.
 */
void
SetFileWriter::writeXDTs(const double* xdts)
{
    // The value of the final result has digitizer dependent
    // granularity and the minium
    // value is at least one unit of granularity:
    
    double granularity;
    if ((m_Mhz == 100) || (m_Mhz == 500)) {
        granularity = 6;
    } else if (m_Mhz == 250) {
        granularity  = 8;
    } else {
        throwInvalidMhz();
    }
    // Now we can start the computation of the xdt values:
    
    std::vector<uint32_t> settings;
    for (int i =0; i < 16; i++) {
        uint32_t xwait = round(xdts[i] * DSP_CLOCK_MHZ);
        if (xwait < granularity) {
            xwait = granularity;
        }
        xwait = floor((double)xwait/granularity) * granularity;
        settings.push_back(xwait);
    }
    m_editor.setChanPar(m_slot, "Xwait", settings.data());
}
/**
 * writeBaselines
 *    Write the baseline percentages. This is just an integerized value
 *    that's at least 1 and no greater than 99.
 *
 *  @param bls - the 16 double baseline percentages.
 */
void
SetFileWriter::writeBaselines(const double* bls)
{
    std::vector<uint32_t> values;
    for (int i =0; i < 16; i++) {
        uint32_t bl = bls[i];
        if (bl < 1)  bl = 1;
        if (bl > 99) bl =99;
        values.push_back(bl);
    }
    m_editor.setChanPar(m_slot, "BaselinePercent", values.data());
}
/**
 * writeBinFactor
 *    Write the bin factor.  This is really the difference beetween
 *    log2(32) and our factor... with limits.
 *
 *  @param fs - the 16 bin factors in the internal representation used by
 *              writesgl...
 */
void
SetFileWriter::writeBinFactor(const uint32_t* fs)
{
    std::vector<uint32_t> values;
    for (int i =0; i < 32; i++) {
        uint32_t v = fs[i];
        if (v < 1) v = 1;
        if (v > 6) v = 6;
        v = pow(2.0, 32.0)- (double)(v);  
        values.push_back(v);
    }
    m_editor.setChanPar(m_slot, "Log2Ebin", values.data());
}
/**
 * writeBaselineAverages
 *    Write the baseline averages.  These are also integerized log somethings.
 *
 * @param blas - the baseline averages in internal units.
 */
void
SetFileWriter::writeBaselineAverages(const uint32_t* blas)
{
    std::vector<uint32_t> values;
    for  (int i = 0; i < 16; i++) {
        uint32_t v = blas[i];
        if (v > 16) v = 16;
        if (v <= 0 ) v = 1;          // The writeSglChanPar refuses to do this.
        v = pow(2.0, 32.0)- (double)(v);
        values.push_back(v);
    }
    m_editor.setChanPar(m_slot, "Log2Bweight", values.data());
}
/**
 * setIntFrom Doubles
 *    When there's a double chan parameter that's really just the int,
 *    marshall and set.
 * @param what - name of the setfile parameter.
 * @param vals - 16 - if > 1 the maximum allowed value.
 */
void
SetFileWriter::setIntFromDoubles(
    const char* what, const double* vals, int bound)
{
    std::vector<uint32_t> ivals;
    for (int i =0; i < 16; i++) {
        uint32_t v = vals[i];
        if ((bound > 0) && (v > bound)) v = bound;
        ivals.push_back(v);
    }
    m_editor.setChanPar(m_slot, what, ivals.data());
}
/**
 * writeFastTriggerBacklens
 *    Write  the fast trigger back length value.
 *    This involves some module type dependent computing.
 * @param blens - the 16 backlengths in microseconds.
 */
void
SetFileWriter::writeFastTriggerBacklens(const double* blens)
{
    std::vector<uint32_t> lens;
    double divisor = clkMult();   // we know there's a legal speed now.
    double minlen;
    if (m_Mhz == 250) minlen  = FASTTRIGBACKLEN_MIN_125MHZFIPCLK;
    else              minlen  = FASTTRIGBACKLEN_MIN_100MHZFIPCLK; //500's too.

    for (int i =0; i < 16; i++ ) {
        uint32_t value = round(blens[i] * m_Mhz/divisor);
        if (value < minlen) value = minlen;
        if (value > FASTTRIGBACKLEN_MAX) value  = FASTTRIGBACKLEN_MAX;
        
        lens.push_back(value);
    }
    m_editor.setChanPar(m_slot, "FastTrigBackLen", lens.data());
}
/**
 * writeCFDDelays
 *    Compute the device unites for the CFD delays and apply limits.
 * @param dels - 16 delays in microseconds.
 */
void
SetFileWriter::writeCFDDelays(const double* dels)
{
    writeClockScaledWithLimits(
        "CFDDelay", dels, CFDDELAY_MIN, CFDDELAY_MAX
    );
}
/**
 * writeQDCLen
 *   Write one of the QDC length parameters.
 *
 * @param which - parameter name.
 * @param lens  - array of 16 lengths.
 */
void
SetFileWriter::writeQDCLen(const char* which, const double* lens)
{
    std::vector<uint32_t> vals;
    double divisor;
    if ((m_Mhz == 100) || (m_Mhz == 250)) divisor = 1;
    else if (m_Mhz == 500)  divisor = 5;
    else {
        throwInvalidMhz();
    }

    for (int i = 0; i < 16; i++) {
        uint32_t l = round(lens[i] * m_Mhz/divisor);
        if (l < QDCLEN_MIN) l = QDCLEN_MIN;
        if (l > QDCLEN_MAX) l = QDCLEN_MAX;
        vals.push_back(l);
    }
    m_editor.setChanPar(m_slot, which, vals.data());
}
/**
 * writeExtTrigStretch
 *    Write the external trigger stretch values.
 *    These need scaling dependent on the digitizer and
 *    limiting.
 * @param stretches - the 16 raw stretch values in microseconds.
 */
void
SetFileWriter::writeExtTrigStretch(const double* stretches)
{
    writeClockScaledWithLimits(
        "ExtTrigStretch", stretches, EXTTRIGSTRETCH_MIN,
        EXTTRIGSTRETCH_MAX
    );
}
/**
 * writeTaus
 *   Writes the preamp-taus to the set file.  This is a bit tricky for two
 *   reasons:
 *   1.  The taus are actually in the setfile as a IEEE single precision
 *       floating point, so we need to convert the doubles to singles and then
 *       move those bits into the uint32_t setting.
 *   2.  Normally, the basline cut is coupled to this.  However if
 *       the Tau is written via Pixie16WriteSglChanPar, the XIA APi code
 *       calls Pixie16BLcutFinder which uses the FPGA to collect baselines
 *       and compute the value of blCut.  We're going to ignore that on the
 *       grounds that we think that either:
 *       a) The baseline cut we've already saved is consistent with the
 *          rest of the parameters.
 *       b) Maybe, just maybe, the baseline cut finder is run after the
 *          load of the set file?
 *       one or both of these _should_ be the case.
 *
 *  @param taus - The 16 double precision tau values in microseconds.
 */
void
SetFileWriter::writeTaus(const double* taus)
{
    std::vector<uint32_t> ts;
    for (int i =0; i < 16; i++) {
        float tau = taus[i];
        uint32_t t;
        memcpy(&t, &tau, sizeof(uint32_t));
        ts.push_back(t);
    }
    m_editor.setChanPar(m_slot, "PreampTau", ts.data());
}
//
//  Note there's a circular dependency between the energy rise time
//  (slow length) and energy flattop (slow gap).  Therefore it's a good
//   idea to call these twice in hopes things converge.

/**
 * writeERise
 *   Write the energy rise times.  This can have a whole host of side-effects.
 *
 * @param rises - the 16 per channel rise times in microseconds.
 */
void
SetFileWriter::writeERise(const double* rise)
{
    uint32_t              ffr = m_editor.get(m_slot, "FastFilterRange");
    uint32_t              sfr = m_editor.get(m_slot, "SlowFilterRange");
    std::vector<uint32_t> paflens = m_editor.getChanPar(m_slot, "PAFlength");
    std::vector<uint32_t> tdels   = m_editor.getChanPar(m_slot, "TriggerDelay");
    std::vector<uint32_t> ftops   = m_editor.getChanPar(m_slot, "SlowGap");
    
    std::vector<uint32_t> trdels;
    for (int i = 0; i < 16; i++) {
        trdels.push_back(paflens[i] - (double)tdels[i]/pow(2.0,(double(ffr))));
    }
    
    double divisor = clkMult();
    std::vector<uint32_t> sls;
    
    for (int i =0; i < 16; i++) {
        uint32_t sl = round(rise[i] *m_Mhz/divisor)/pow(2.0, sfr);
        uint32_t sg = ftops[i];
        
        if (sl + sg > SLOWFILTER_MAX_LEN) {
            sl = SLOWFILTER_MAX_LEN - sg;
        }
        if (sl <  MIN_SLOWLENGTH_LEN) {
            sl =  MIN_SLOWLENGTH_LEN;
            if (sl + sg > SLOWFILTER_MAX_LEN)  {
                sg = SLOWFILTER_MAX_LEN - MIN_SLOWLENGTH_LEN;
            }
        }
        sls.push_back(sl);
        ftops[i] = sg;             // update.
    }
    // Set the length and update the gap.
    
    m_editor.setChanPar(m_slot, "SlowLength", sls.data());
    m_editor.setChanPar(m_slot, "SlowGap",    ftops.data());
    
    
    computePeakSampleAndSep(sfr, sls, ftops);
    for (int i =0; i < 16; i++) {
        computeFifo(trdels[i], i);
    }
}
/**
 * writeEFlattop
 *   Write the energy filter flattop values (SLowGap).   Note this can result
 *   in changes to the SlowLength and peaking parameters.
 *
 * @param flats - 16 flattop lengths in microsecond units.
 */
void
SetFileWriter::writeEFlattop(const double* flats)
{
    uint32_t              ffr = m_editor.get(m_slot, "FastFilterRange");
    uint32_t              sfr = m_editor.get(m_slot, "SlowFilterRange");
    std::vector<uint32_t> paflens = m_editor.getChanPar(m_slot, "PAFlength");
    std::vector<uint32_t> tdels   = m_editor.getChanPar(m_slot, "TriggerDelay");
    std::vector<uint32_t> rises   = m_editor.getChanPar(m_slot, "SlowLength");
    
    std::vector<uint32_t> trdels;
    for (int i = 0; i < 16; i++) {
        trdels.push_back(paflens[i] - (double)tdels[i]/pow(2.0,(double(sfr))));
    }
    
    double divisor = clkMult();
    std::vector<uint32_t> sgs;
    for (int i =0; i < 16; i++) {
        uint32_t sg = round(flats[i]*m_Mhz/divisor/pow(2.0, sfr));
        uint32_t sl = rises[i];
        if (sg + sl > SLOWFILTER_MAX_LEN) {
            sg = SLOWFILTER_MAX_LEN - sl;
        }
        if (sg <MIN_SLOWGAP_LEN ) {
            sg = MIN_SLOWGAP_LEN;
            if (sg + sl > SLOWFILTER_MAX_LEN) {
                sl = SLOWFILTER_MAX_LEN - MIN_SLOWGAP_LEN;
            }
        }
        sgs.push_back(sg);
        rises[i] = sl;
        
    }
    // Set the length and update the gap.
    
    m_editor.setChanPar(m_slot, "SlowLength", rises.data());
    m_editor.setChanPar(m_slot, "SlowGap",    sgs.data());
    
    
    computePeakSampleAndSep(sfr, rises, sgs);
    for (int i =0; i < 16; i++) {
        computeFifo(trdels[i], i);
    }
}
/**
 * writeTraceLength
 *    Converts and writes the trace length.  THe length is also limited
 *    to a minimum length that's digitizer type dependent and to the
 *    size of the FIFO.
 *
 * @param lens - the 16 channel trace lengths in microseconds.
 */
void
SetFileWriter::writeTraceLengths(const double* lens)
{
    uint32_t ffr = m_editor.get(m_slot, "FastFilterRange");
    uint32_t fifo= m_editor.get(m_slot, "FIFOLength");
    uint32_t minlen(0);
    if (m_Mhz == 500) minlen =  TRACELEN_MIN_500MHZADC;
    if (m_Mhz  == 250) minlen =  TRACELEN_MIN_250OR100MHZADC;
    if (m_Mhz  == 100) minlen = TRACELEN_MIN_250OR100MHZADC;
    if (minlen == 0) {
        throwInvalidMhz();
    }
    
    std::vector<uint32_t> tlens;
    for (int i =0; i < 16; i++) {
        uint32_t len = (lens[i] * m_Mhz) / pow(2.0, ffr);
        if (len < minlen) len = minlen;
        if (len > fifo) len = fifo;
        tlens.push_back(len);
    }
    
    m_editor.setChanPar(m_slot, "TraceLength", tlens.data());
}
/**
 * writeTraceDelays
 *   Write the trace delay values.  This also affects the fifo
 *   paramters
 *
 * @param dels - the 16 delays in microsecond units.
 */
void
SetFileWriter::writeTraceDelays(const double* dels)
{
    // Note the trace delay is called the PAFlength and is set
    // in computeFifo once we've converted the request to device units.
    
    uint32_t ffl = m_editor.get(m_slot, "FastFilterRange");
    std::vector<uint32_t> tlen = m_editor.getChanPar(m_slot, "TraceLength");
    double  ffd = pow(2.0, double(ffl));
    double divisor = clkMult();
    
    for (int i = 0; i < 16; i++) {
        uint32_t del = dels[i]*m_Mhz/(divisor*ffd);
        if (del > tlen[i]) {
            del = tlen[i]/2.0;
        }
        if (del > TRACEDELAY_MAX) del = TRACEDELAY_MAX;
        computeFifo(del, i);           // Actually sets the parameter.
    }
}
///////////////////////////////////////////////////////////////////////////////
// Utilities for the utilities.

/**
 * computePeakSampleAndSep
 *   Compute the peak samping and separation parameters. (PeakSample, PeakSep)
 *
 *   @param sfr - short filter range.
 *   @param sls - slow lengths.
 *   @param sgs - slow gaps.
 */
void
SetFileWriter::computePeakSampleAndSep(
   uint32_t sfr,
    const std::vector<uint32_t>& sls, const std::vector<uint32_t>& sgs
)
{
    std::vector<uint32_t> pksample;
    std::vector<uint32_t> pksep;
    for (int i = 0; i < 16; i++) {
        uint32_t sep = sls[i] + sgs[i];
        uint32_t samp= sls[i] + sgs[i];
        switch (sfr) {
        case 1:
            samp -= 3;
            break;
        case 2:
        case 3:
            samp -= 2;
            break;
        case 4:
            samp -= 1;
        case 5:
            break;
        case 6:
            samp += 1;
        default:
            samp += 2;
        }
        pksample.push_back(samp);
        pksep.push_back(sep);
    }
    m_editor.setChanPar(m_slot, "PeakSample", pksample.data());
    m_editor.setChanPar(m_slot, "PeakSep",    pksep.data());
}
/**
 * computeFIfo
 *   Reproduces the logic of Pixie_ComputeFIFO in the XIA code.
 *   This updates the set file values for
 *   -   TriggerDelay  (per channel)
 *   -   PAFLength     (per channel)
 * @note We assume the following items are correctly set at this time:
 *   -  SlowFilterRange
 *   -  FastFilterRange
 *   -  PeakSep
 *   -  FifoLength
 *  
 *  @param traceDelay - desired trace delay.
 *  @param chan       - Channel number for the trace delay.
 */
void
SetFileWriter::computeFifo(uint32_t traceDelay, unsigned short chan)
{
    // Get the other stuff we need. the filter ranges and fifo length are
    // module level, PeakSep is per channel.
    
    uint32_t slowRange = m_editor.get(m_slot, "SlowFilterRange");
    uint32_t fastRange = m_editor.get(m_slot, "FastFilterRange");
    uint32_t fifo      = m_editor.get(m_slot, "FIFOLength");
    std::vector<uint32_t> peaksep = m_editor.getChanPar(m_slot, "PeakSep");
    
    uint32_t triggerDelay =
        (double(peaksep[chan])-1.0) * pow(2.0,(double)slowRange);
    uint32_t pafLen =
        (double(triggerDelay) / pow(2.0, (double)fastRange)) + traceDelay;
    if (pafLen > fifo) {
        pafLen = fifo;
        triggerDelay = (pafLen - traceDelay*pow(2.0, (double)fastRange));
    }
    // To update per channel values, we need to fatch them first.
    
    std::vector<uint32_t> tdelays = m_editor.getChanPar(m_slot, "TriggerDelay");
    tdelays[chan] = triggerDelay;
    m_editor.setChanPar(m_slot, "TriggerDelay", tdelays.data());
    
    std::vector<uint32_t> pafs = m_editor.getChanPar(m_slot, "PAFlength");
    pafs[chan] = pafLen;
    m_editor.setChanPar(m_slot, "PAFlength", pafs.data());
    
}
/**
 * writeClockScaledWithLimits
 *    does the *MHz/denom scale and enforces limits before setting.
 *
 * @param what - parameter to set.
 * @param raw  - array of 16 raw values.
 * @param low, high - low and high limits.
 */
void
SetFileWriter::writeClockScaledWithLimits(
    const char* what, const double* raw, uint32_t low, uint32_t high
)
{
    std::vector<uint32_t> cooked;
    double denom = clkMult();
    for (int i =0; i < 16; i++) {
        uint32_t v = round(raw[i]*m_Mhz/denom);
        if (v < low) v = low;
        if (v > high) v = high;
        cooked.push_back(v);
    }
    m_editor.setChanPar(m_slot, what, cooked.data());
}


 /**
  * clkMult
  *    Return the clock multiplier.
  */
 double
 SetFileWriter::clkMult() const
 {
    double mult;
    if (m_Mhz == 100) mult =1.0;
    else if (m_Mhz == 250) mult = 2.0;
    else if (m_Mhz == 500) mult = 5.0;
    else {
        throwInvalidMhz();
    }
    return mult;
 }
/**
 * throwInvalidMhz
 *    Throw an exception because m_Mhz is invalid.
 */
void
SetFileWriter::throwInvalidMhz() const
{
    std::stringstream s;
    s << m_Mhz << " is not a valid speed value.  Must be 100, 250 or 500";
    throw std::invalid_argument(s.str());
}

}                       // DDAS Namespace.
