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

/** @file:  SetFileEditor.cpp
 *  @brief: Implement the DDAS::SetFileEditor class.
 */
#include "SetFileEditor.h"
#include <CrateManager.h>
#include <SetFile.h>

#include <config.h>
#include <config_pixie16api.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include <stdexcept>
#include <sstream>

namespace DDAS {
    /**
     *    constructor
     *  - saves the set file name.
     *  - Maps the set file into memory.
     *  - Creates default varmaps for each of
     *    the slots (for 250MHz digitizers).
     *
     *  @param const char* setfile - Path to the set file.
     */
    SetFileEditor::SetFileEditor(const char* setfile) :
        m_setfile(setfile), m_base(nullptr), m_longs(0)
    {
        // Set files have two piles of slots.  One from 2-14 another
        // from 2-12 for a total of 24 slots. We're going to treat them
        // as a contigous set of slots numbered 2-22.. in fact
        // a typical crate has 14 slots so this is too many.
        
        m_varmaps.resize(24);   // empt maps.
        for (int i =2; i < m_varmaps.size() + 2; i++) {
            setSlotSpeed(i, 250);    // All maps are 250MHz to begin.
        }
        mapFile();
    }
    /**
     * destructor
     *    just munmap the file.m_longs tells us how big it is
     *     and includes any page padding needed.
     */
    SetFileEditor::~SetFileEditor()
    {
        if (m_base) {      // Maybe the map failed?
            checkErrno(
                munmap(m_base, m_longs*sizeof(uint32_t)),
                "Unmapping the setfile in SetFileEditor's destructor"
            );
        }
    }
    /**
     * setSlotSpeed
     *   Sets the speed of a slot.  This is needed to select
     *   the proper varfile that defines the offsets into the
     *   chunk of memory associated with a slot.
     * @param slot  - slot number (2 is the first slot).
     * @param speed - MHz speed of the digitizer.
     * @note Several of the methods called by this method may throw
     *       exceptions that derive from std::exception including
     * @throws std::range_error
     * @throws std::invalid_argument
     * 
     */
    void
    SetFileEditor::setSlotSpeed(unsigned short slot, unsigned short speed)
    {
        std::string varFile = CrateManager::getVarFile(speed);
        auto offsets        = DDAS::SetFile::readVarFile(varFile.c_str());
        int idx = offset(slot);
        m_varmaps.at(idx)   = DDAS::SetFile::createVarNameMap(offsets);
    }
    /**
     * set
     *    Set a uint32_t value in a set file chunk.
     * @param slot - slot number
     * @param what - Name of the item to set.
     * @param value - new value to set.
     * @throws std::range_error
     * @throws std::invalid_argument
     */
    void
    SetFileEditor::set(
        unsigned short slot, const char* what, uint32_t value
    )
    {
        uint32_t* p = pointer(slot, what);
        *p = value;
    }
    /**
     * setChanPar
     *   Set the 16 values of a channel parameter.
     * @param slot slot number
     * @param what parameter name.
     * @param values to set
     * @throws std::range_error
     * @throws std::invalid_argument
     */
    void
    SetFileEditor::setChanPar(
        unsigned short slot, const char* what, const uint32_t* values
    )
    {
        uint32_t* p = arrayPointer(slot, what);
        auto      d = mapEntry(slot, what);
        unsigned  s = d.s_nLongs;
        memmove(p, values, s*sizeof(uint32_t));
    }
    
    /**
     * get
     *    Get a uint32_t value in a set file chunk.
     *  @param slot - slot number.
     *  @param what - name of the item to get.
     *  @return uint32_t
     *  @throws std::range_error
     *  @throws std::invalid_argument
     */
    uint32_t
    SetFileEditor::get(
        unsigned short slot, const char* what
    )
    {
        uint32_t* p = pointer(slot, what);
        return *p;
    }
    /**
     * getChanPar
     *    Get the 16 values of a channel parameter
     *
     * @param slot - slot number,
     * @param what - what to get.
     * @return std::vector<uint32_t>
     * @throws std::range_error
     * @throws std::invalid_argument
     */
    std::vector<uint32_t>
    SetFileEditor::getChanPar(unsigned short slot, const char* what)
    {
        std::vector<uint32_t> result;
        uint32_t* p = arrayPointer(slot, what);
        for (int i =0; i < 16; i++) {
            result.push_back(*p++);
        }
        return result;
    }
    ///////////////////////////////////////////////////////////////
    // Private utilitye methods.
    
    /**
     * mapFile
     *    Now that the file is all set, map it.
     *    - ensure it exists and can be opened.
     *    - ensure it's at lease 24*N_DSP_PAR*sizeof(uint32_t)
     *      bytes long.
     *   -  round the file size up to the nearest page size.
     *   -  mmap it.
     *   - save the pointer and number of longs in the map (not file).
     * @throws std::invalid_argument
     */
    void
    SetFileEditor::mapFile()
    {
        // Open the file:
        
        const char* pName = m_setfile.c_str();
        int fd = open(pName, O_RDWR);
        checkErrno(fd, "Attempting to open the set file");
        
        // Get its size
        
        struct stat info;
        int status = fstat(fd, &info);
        checkErrno(status, "Attempting to ascertain set file size");
        
        if (info.st_size < 24*N_DSP_PAR*sizeof(uint32_t)) {
            std::stringstream msg;
            msg << "Set files must be at least "
                << 24*N_DSP_PAR*sizeof(uint32_t)
                << " bytes long.  " << pName << "  is only "
                << info.st_size << " bytes long. "
                << "It's not  setfile.";
            throw std::invalid_argument(msg.str());
            
        }
        // round the file up to the next page:
        
        long pagelen = sysconf(_SC_PAGESIZE);
        checkErrno(pagelen, "Determining system page size");
        size_t mapLen = info.st_size;
        mapLen = ((mapLen + pagelen-1)/pagelen) * pagelen;
        
        // Map the file:
        
        void* pMap = mmap(
            nullptr, mapLen, PROT_READ | PROT_WRITE, MAP_SHARED,
            fd, 0
        );
        checkErrno(pMap, "Mapping the set file into memory");
        checkErrno(close(fd), "Closing set file once mapped");
    
        // Set the member data:
        
        m_longs = mapLen/sizeof(uint32_t);  // assum pagelen is a multiple
        m_base  = static_cast<uint32_t*>(pMap);
    }
    
    /**
     *  mapEntry
     *     Returns a const reference to a variable descriptor
     *     in a DDAS::VarMapByName
     *
     *  @param slot - the slot of of the map.
     *  @param what - Name of the item to find.
     *  @return const DDAS::VarDescriptor&
     *  @throws std::range_error
     *  @throws std::invalid_argument
     */
    const DDAS::VarDescriptor&
    SetFileEditor::mapEntry(unsigned short slot, const char* what)
    {
        unsigned o = offset(slot);   // Slot to vector index.
        DDAS::VarMapByName& map = m_varmaps.at(o);
        auto p = map.find(what);
        if (p == map.end()) {
            std::stringstream msg;
            msg << "There is no DSP parameter named: " << what;
            throw std::invalid_argument(msg.str());
        }
        
        return p->second;
    }
    /**
     * pointer
     *    return a pointer to a scalar parameter.
     *  @param slot  slot number.
     *  @param what  which named DSP parameter.
     *  @return uint32_t*
     *  @throws std::invalid_argument
     *  @throws std::range_error
     */
    uint32_t*
    SetFileEditor::pointer(unsigned short slot, const char* what)
    {
        uint32_t* pBase = base(slot);
        auto&     descr = mapEntry(slot, what);
        
        if (descr.s_nLongs !=  1) {
            std::stringstream msg;
            msg << what << " is not a module parameter.";
            throw std::invalid_argument(msg.str());
        }
        pBase += descr.s_longoff;
        return pBase;
    }
    /**
     * arrayPointer
     *    Same as  pointer but require the parameter be an array.
     */
    uint32_t*
    SetFileEditor::arrayPointer(unsigned short slot, const char* what)
    {
        uint32_t* pBase = base(slot);
        auto&     descr = mapEntry(slot, what);
        
        if (descr.s_nLongs ==  1) {
            std::stringstream msg;
            msg << what << " is not a channel parameter.";
            throw std::invalid_argument(msg.str());
        }
        pBase += descr.s_longoff;
        return pBase;
    }
    /**
     * base
     *   @param slot - slot number
     *   @return uint32_t* - Pointer to the base of the chunk of the
     *                 setfile for the slot.
     *  @throws std::range_error if the slot is out of range.
     */
    uint32_t*
    SetFileEditor::base(unsigned short slot)
    {
        unsigned o = offset(slot);
        unsigned longOff = o*N_DSP_PAR;
        if (longOff >= m_longs) {
            std::stringstream msg;
            msg << slot << " Is not a valid slot number";
            throw std::range_error(msg.str());
        }
        uint32_t* result = m_base;
        return result + longOff;
    }
    
    
    /**
     * checkErrno
     *    Overloaded errno checker.
     *  @param stat - Status to check.
     *  @param p    - Pointer from mmap to check.
     *  @param doing - What was being done at the time.
     *  @throws std::invalid_argument
     */
    void
    SetFileEditor::checkErrno(int stat, const char* doing)
    {
        int e = errno;
        if (stat < 0) {
            throwErrno(e, doing);
        }
    }
    void
    SetFileEditor::checkErrno(void* p, const char* doing)
    {
        int e = errno;
        if (p == ((void*)(-1))) {
            throwErrno(e, doing);
        }
    }
    /**
     * throwErrno
     *    @param e - errno to throw form.
     *    @param doing - context
     */
    void
    SetFileEditor::throwErrno(int e, const char* doing)
    {
        std::stringstream msg;
        msg << doing << " failed: " << strerror(e);
        throw std::invalid_argument(msg.str());
    }
}
