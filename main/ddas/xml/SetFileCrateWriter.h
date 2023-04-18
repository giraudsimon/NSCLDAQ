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

/** @file:  SetFileCrateWriter.h
 *  @brief: Write a crate into a setfile.
 */
#ifndef SETFILECRATEWRITER_H
#include "CrateWriter.h"
#include <string>
#include <map>
#include <vector>
#include <stdint.h>

namespace DDAS {
    
class SetFileEditor;
    
/**
 * @class CrateSetFileWriter
 *     This class writes an entire PXI crate full of however many
 *     digitizers described to a XIA APi Set file.
 */
class SetFileCrateWriter : public CrateWriter
{
private:
    std::string     m_filename;
    SetFileEditor*  m_pEditor;
    std::map<uint16_t, uint16_t> m_slotSpeeds; 
public:
    SetFileCrateWriter(
        const char* setFileName, const Crate& settings,
        const std::vector<std::pair<uint16_t, uint16_t>>& slotspeeds
    );
    virtual ~SetFileCrateWriter();
    
    // Strategy handlers:
        
        virtual void startCrate(
            int id, const std::vector<unsigned short>& slots
        );
        virtual void endCrate(
            int id, const std::vector<unsigned short>& slots
        );
        virtual SettingsWriter* getWriter(unsigned short slotNum);
private:
    void createEditor();
    void copySetFile();
};
}
#endif