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

/** @file:  SettingsReader.h
 *  @brief: Define the abstract base class SettingsReader
 *  
 */

#ifndef SETTINGSREADER_H
#define SETTINGSREADER_H

#include <ModuleSettings.h>
#include <vector>
namespace DDAS {
    /**
     * SettingReader objects are able to take some source of DDAS Setings
     * and turn them into a  ModuleSettins struct.
     * Derived classes could get this information from .set files, from the
     * modules in a crate themselves or event from EPICS or a database.
     * Note that an external entity must have initializd the API
     * and read in the var files for this module (normally done in a
     * CrateManager).
     */
    class SettingsReader
    {
    public:
        virtual ~SettingsReader() {}
        virtual DDAS::ModuleSettings get() = 0;     // Get the settings from the source.
    };
}
#endif