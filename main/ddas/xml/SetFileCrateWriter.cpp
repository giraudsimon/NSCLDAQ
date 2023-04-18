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

/** @file:  SetFileCrateWriter.cpp
 *  @brief: Implement the SetFileCrateWriter class.
 */
#include "SetFileCrateWriter.h"
#include "SetFileEditor.h"
#include "SetFileWriter.h"
#include "ModuleSettings.h"

#include <unistd.h>
#include <sstream>
#include <stdio.h>
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace DDAS {

/**
 *  constructor
 *     Construct the base class,
 *     Save the filename
 *     Create the editor
 *     If necessary copy the filename from the setfile template file.
 *  @param setFileName - Path to the output setfile.
 *  @param settings    - The settings to write.
 *  @param slotspeeds  - Vector of slot/speed pairs.
 */
SetFileCrateWriter::SetFileCrateWriter(
    const char* setFileName, const Crate& settings,
    const std::vector<std::pair<uint16_t, uint16_t>>& slotspeeds
) : CrateWriter(settings), m_filename(setFileName),
    m_pEditor(nullptr)
    
{
        createEditor();
        for (int i =0; i < slotspeeds.size(); i++) {
            m_slotSpeeds[slotspeeds[i].first] = slotspeeds[i].second;
        }
}
/**
 * destructor.
 */
SetFileCrateWriter::~SetFileCrateWriter()
{
    delete m_pEditor;
}

/**
 * startCrate
 *   Flip through the slots and set their speeds in the editor.
 *
 * @param id - crate id.
 * @param slots - vector of slots.
 * @not slots that have not been explicitly assigned a speed are
 *      assigned 250.
 */
void
SetFileCrateWriter::startCrate(
    int id, const std::vector<unsigned short>& slots

)
{
    for (int i =0; i < slots.size(); i++) {
        uint16_t slot = slots[i];
        uint16_t speed = 250;       // Default speed.
        auto p = m_slotSpeeds.find(slot);
        if (p != m_slotSpeeds.end()) {
            speed = p->second;
        }
        m_pEditor->setSlotSpeed(slot, speed);  // Probably redundant.
    }
    
}
/**
 * endCrate
 *    After the settings have been written, each slot's
 *    crate id is also set:
 * @param id - crate id.
 * @param slots - slot vector.
 */
void
SetFileCrateWriter::endCrate(
    int id, const std::vector<unsigned short>& slots
)
{
    for (int i =0; i < slots.size(); i++) {
        m_pEditor->set(slots[i], "CrateID", id);
    }
}
/**
 * getWriter
 *   Returns the settings writer to be used by the
 *   base class to write a slot worth of settings.
 *
 *  @param slotnum - the number of the slot being written.
 */
SettingsWriter*
SetFileCrateWriter::getWriter(unsigned short slotNum)
{
    unsigned short speed;
    auto p = m_slotSpeeds.find(slotNum);
    if (p == m_slotSpeeds.end()) speed = 250;
    else speed= p->second;
    return new SetFileWriter(*m_pEditor, slotNum, speed);
}
///////////////////////////////////////////////////////////////////////////////
// Utility methods

/**
 * createEditor
 *     - If the set file is not in place, copy the template setfile.
 *     - New the setfile editor into existence and plot its pointer into
 *       m_pEditor.
 *  @note we need read/write access to the set file too.
 */
void
SetFileCrateWriter::createEditor()
{

    int status = access(m_filename.c_str(), R_OK | W_OK);
    if (status < 0) {
        if (errno == EACCES) {     // Exists but perms no good:
            std::stringstream msg;
            msg << "The set file: " << m_filename
                << " must allow both read and write access!";
            throw std::invalid_argument(msg.str());
            
        } else if (errno == ENOENT) {
            copySetFile();
        } else {
            int e = errno;
            std::stringstream msg;
            msg << "Unexpected error probing for setfile: "
                << strerror(e);
            throw std::logic_error(msg.str());
        }
    }
    m_pEditor = new DDAS::SetFileEditor(m_filename.c_str());
}
/**
 * copySetFile
 *    When the output set file does not exist it gets created by copying
 *    a template that lives in SEFILE_TEMPLATE (determined at build config
 *    time).  Well somebody's got to do this and that's us.
 */
void
SetFileCrateWriter::copySetFile()
{
    const char* inname = SETFILE_TEMPLATE;
    const char* outname= m_filename.c_str();
    
    // Try to open the files:
    
    int infd = open(inname, O_RDONLY);
    if (infd < 0) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to open set file template: " << inname
            << " : " << strerror(e);
        throw std::logic_error(msg.str());
    }
    int perms = S_IRUSR | S_IWUSR | S_IRGRP;
    int ofd   = creat(outname, perms);
    if (ofd < 0) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to create setfile: " << outname
            << " : " << strerror(errno);
        throw std::invalid_argument(msg.str());
    }
    uint8_t buffer[16384];
    ssize_t n;
    while ((n = read(infd, buffer, sizeof(buffer))) > 0) {
        ssize_t nw = ::write(ofd, buffer, n);
        if (nw != n) {
            if (nw < 0) {
                int e = errno;
                std::stringstream msg;
                msg << "Write to setfile copy : " << outname
                    << " failed: " << strerror(e);
                throw std::logic_error(msg.str());
            } else {
                std::stringstream msg;
                msg << "Incomplete write copying template setfile to : " << outname
                    << " should have copied " << n << " copied " << nw;
                throw std::logic_error(msg.str());
            }
        }
    }
    if (n < 0) {
        int e = errno;
        std::stringstream msg;
        msg << "Read from setfile template: " << inname
            << " failed: " << strerror(e);
        throw std::logic_error(msg.str());
    }
}

////////////////////////////////////////////////////////////////////////////////
}                                   // namespace DDAS