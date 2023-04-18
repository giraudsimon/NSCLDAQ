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

/** @file:  SetFileWriter.h
 *  @brief: Class to write information from a module
 *          into a set file for a particular slot.
 */
#ifndef SETFILEWRITER_H
#define SETFILEWRITER_H
#include "SettingsWriter.h"

namespace DDAS {

class SetFileEditor;
class ModuleSettings;

/**
 * @class SetFileWriter
 *    Given a slot number and a set file editor, writes the
 *    settings from our internal format into the set file's chunk
 *    for that module.
 */
class SetFileWriter  : public SettingsWriter
{
private:
    SetFileEditor&  m_editor;
    unsigned short  m_slot;
    unsigned short  m_Mhz;
public:
    SetFileWriter(
        SetFileEditor& editor, unsigned short slot,
        unsigned short Mhz
    );
    virtual ~SetFileWriter(){}
    
    virtual void write(const ModuleSettings& dspSettings);
private:
    void writeModuleSettings(const ModuleSettings& dspSettings);
    void writeChannelSettings(const ModuleSettings& dspSettings);
    
    void writeTriggerRiseTime(const double* riseTimes, uint32_t ffrange);
    void writeTriggerFlattop(const double* ftops, uint32_t ffrange);
    void writeTriggerThresholds(const double* ts);
    void writeVoffsets(const double* voffs);
    void writeXDTs(const double* xdts);
    void writeBaselines(const double* bls);
    void writeBinFactor(const uint32_t* fs);
    void writeBaselineAverages(const uint32_t* blas);
    void setIntFromDoubles(const char* what, const double* vals, int bound=-1);
    void writeFastTriggerBacklens(const double* blens);
    void writeCFDDelays(const double* dels);
    void writeQDCLen(const char* which, const double* lens);
    void writeExtTrigStretch(const double* stretches);
    void writeTaus(const double* taus);
    void writeERise(const double* rise);
    void writeEFlattop(const double* flats);
    void writeTraceLengths(const double* lens);
    void writeTraceDelays(const double* dels);

    void computePeakSampleAndSep(
        uint32_t sfr,
        const std::vector<uint32_t>& sls, const std::vector<uint32_t>& sgs
    );
    void computeFifo(uint32_t traceDelay, unsigned short chan);
    void writeClockScaledWithLimits(
        const char* what, const double* raw, uint32_t low, uint32_t high
    );
    double clkMult() const;
    void throwInvalidMhz() const;
};

}                                     // namespace DDAS.
#endif