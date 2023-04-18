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

/** @file:  ModuleWriter.h
 *  @brief: Defines a class for writing settings to modules.
 */
#ifndef MODULEWRITER_H
#include "SettingsWriter.h"

namespace DAQ {
    namespace DDAS {
        class Configuration;
    }
}
 
 namespace DDAS {
 /**
  * @class SettingsWriter
  *     Class that writes  settings data to modules attached
  *     to the system.  The vector of module settings define what's written
  *     this allows for module level granularity on settings written to
  *     the module.
  */
class ModuleWriter : public SettingsWriter
{
    unsigned m_moduleId;
    unsigned m_crateId;
    unsigned m_slotId;

public:
    ModuleWriter(unsigned crate, unsigned slot, unsigned module);
    virtual ~ModuleWriter();
    virtual void write(const ModuleSettings& dspSettings);
private:
    void loadModule(unsigned modId, const ModuleSettings& settings);
    void loadModuleSettings(unsigned modid, const ModuleSettings& settings);
    void loadChannelSettings(
        unsigned modId, unsigned chan, const ModuleSettings& settings
    );
    void writeModuleSetting(
        unsigned modid, const char* name, unsigned int value
    );
    void writeChannelSetting(
        unsigned modid, unsigned chan, const char* name, double value
    );
};
}                              // DDAS Namespace.
 #endif
 
 