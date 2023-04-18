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

/** @file:  SetFileCrateReader.h
 *  @brief: CrateReader to take  module info from .set files.
 */
#ifndef SETFILECRATEREADER_H
#define SETFILECRATEREADER_H

#include "CrateReader.h"
#include <map>
#include <string>
namespace DDAS {
/**
 *  @class SetFileCrateReader
 *     Provides a crate readerthat can read the module settings for a crate
 *     from a set file.  Set files don't, by themselves, provide sufficient
 *     information to read them into a ModuleSettings struct.  In addtion
 *     we need the digitizer speed and a .VAR file that describes the layout
 *     of DSP Parameters in memory.  These are per module items.
 *     The .set file, however is a per crate item...as we use it here.
 *     
 */
class SetFileCrateReader : public CrateReader
{
private:
    std::map<unsigned short, std::string> m_varFiles;
    std::map<unsigned short, unsigned>    m_MHz;
    std::string                      m_setFile;
public:
    SetFileCrateReader(
        const char* setFile, unsigned crateId,
        std::vector<unsigned short> slots,
        std::vector<std::string> varFiles,
        std::vector<unsigned short> MHz
    );
    virtual ~SetFileCrateReader() {}
    
    virtual SettingsReader* createReader(unsigned short slot);
};

}
#endif