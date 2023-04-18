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

/** @file:  SetFileReader.h
 *  @brief: Define the a settings reader for set files.
 */
 #ifndef SETFILEREADER_H
 #define SETFILEREADER_H
 
 #include "SettingsReader.h"
 #include "SetFile.h"
 #include <string>
 #include <map>
 namespace DDAS {
    class SetFileReader : public SettingsReader
    {
    private:
        std::string m_setfile;
        std::string m_varfile;
        unsigned    m_Mhz;
        unsigned    m_slotId;
    public:
        SetFileReader(
            const char* setfile, const char* varfile,
            unsigned Mhz, unsigned slot
        );
        virtual ~SetFileReader() {}
        virtual DDAS::ModuleSettings get();
        
    private:
        std::pair<unsigned, uint32_t*> findSlot(
           const std::pair<unsigned, uint32_t*>& vars,
           const VarOffsetArray& map
        );
        DDAS::ModuleSettings getModuleSettings(
            unsigned nLongs, uint32_t* pVars, const VarOffsetArray& map
        );
        std::pair<unsigned, uint32_t*> next(
            unsigned nLongs, uint32_t* pVars, const VarOffsetArray& map
        );
        
        uint32_t getSingleItem(
            const DDAS::SetFileByName& settings, const char* name
        );
        std::vector<uint32_t> getItemVector(
            const DDAS::SetFileByName& settings, const char* name
        );
        std::vector<float> getFloats(
            const DDAS::SetFileByName& settings, const char* name
        );
        uint32_t getMsPs();
        double   toMicros(uint32_t value, uint32_t MsPs);
  
        void setChannelParamsUs(
            double* dest, const DDAS::SetFileByName& src,
            const char* which, uint32_t msps, double fastFilterRange
        );
        void setChannelParamsV(
            double* dest, const DDAS::SetFileByName& src,
            const char* which, uint32_t range
        );
        void setChannelParamsI(
            uint32_t* dest, const DDAS::SetFileByName& src,
            const char* which
        );
        void setChannelParamsD(
            double* dest, const DDAS::SetFileByName& src,
            const char* which
        );
        void convertTriggerThresholds(
           double* rawThresholds, const DDAS::SetFileByName& src, uint32_t msps
        );
        void convertEnergyTimes(
           double* raw, double multiplier, uint32_t msps
        );
        void convertTraceLength(double* raw, double ffr, uint32_t msps);
        void convertTraceDelay(
           double* raw, const DDAS::SetFileByName& src, double ffr, uint32_t  msps
        );
        void convertBinFactor(uint32_t* raw);
        void convertBlAverage(uint32_t* raw);
        void computeXdt(double* raw);
        void convertQdcLen(double* raw, uint32_t msps);
        const Variable& throwIfNotPresent(
            const DDAS::SetFileByName& src, const char* which
        );
        
        
    };
 }                               // DDASNamespace
 
 #endif