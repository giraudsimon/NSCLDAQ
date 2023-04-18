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

/** @file:  PixieSettingsReader.h
 *  @brief: Define a class that gets a settomg from a single Pixie16 module.
 *  
 */
#ifndef PIXIESETTINGSREADERS_H
#define PIXIESETTINGSREADERS_H
#include "SettingsReader.h"
#include "ModuleSettings.h"
#include <vector>
namespace DDAS {
    class PixieSettingsReader : public SettingsReader
    {
    private:
        unsigned m_moduleId;
    public:
        PixieSettingsReader(unsigned id);
        virtual ~PixieSettingsReader();
        
        DDAS::ModuleSettings get();
    private:
        ModuleSettings readSettings(unsigned module);
        void checkModuleParamRead(int code);
        void checkChannelParamRead(int code, int channel);
        
    };
}
#endif