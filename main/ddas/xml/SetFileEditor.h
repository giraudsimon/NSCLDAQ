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

/** @file:  SetFileEditor.h
 *  @brief: Provides the ability to edit a set file.
 */
#ifndef SETFILEEDITOR_H
#define SETFILEEDITOR_H
#include  "SetFile.h"

#include <vector>
#include <string>

namespace DDAS {
    /**
     * @class SetFileEditor
     *      Provides the ability to edit a set file.
     *      The set file must already exist and is mapped into memory.
     */
    class SetFileEditor {
    private:
        std::vector<DDAS::VarMapByName> m_varmaps;
        std::string                     m_setfile;
        uint32_t*                       m_base;             // Mapping address.
        size_t                          m_longs;            // Number of longs.
    public:
        SetFileEditor(const char* setfile);
        virtual ~SetFileEditor();
        
        void setSlotSpeed(unsigned short slot, unsigned short speed);
        
        void set(unsigned short slot, const char* what, uint32_t value);
        void setChanPar(
            unsigned short slot, const char* what, const uint32_t* values
        );
        uint32_t get(unsigned short slot, const char* what);
        std::vector<uint32_t> getChanPar(
            unsigned short slot, const char* what
        );
    private:
        void mapFile();
        unsigned offset(unsigned short slot) const { return slot - 2;}
        const DDAS::VarDescriptor&  mapEntry(
            unsigned short slot, const char* what
        );
        
        uint32_t* pointer(unsigned short slot, const char* what);
        uint32_t* arrayPointer(unsigned short slot, const char* what);
        uint32_t* base(unsigned short slot);

        
        void checkErrno(int stat, const char* doing);
        void checkErrno(void* p, const char* doing);
        void throwErrno(int e, const char* doing);
    };
    
}

#endif
