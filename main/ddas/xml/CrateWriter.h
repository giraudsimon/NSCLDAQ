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

/** @file:  CrateWriter
 *  @brief: Base class to save a crate to something.
 */
#ifndef CRATEWRITER_H
#define CRATEWRITER_H
#include "ModuleSettings.h"
namespace DDAS {
    class SettingsWriter;
    /**
     * @class CrateWriter
     *    Writes a crate worth of stuff to a destination.
     *    This abstract base class provides a strategy pattern.
     */
    class CrateWriter {
    protected:
        Crate                       m_settings;
    public:
        CrateWriter(const Crate& settings);
        virtual ~CrateWriter() {}
    private:
        CrateWriter(const CrateWriter& );
        CrateWriter& operator=(const CrateWriter&);
        int operator==(const CrateWriter&) const;
        int operator!=(const CrateWriter&) const;
    public:
        virtual void write();
        
        // Strategy handlers:
        
        virtual void startCrate(
            int id, const std::vector<unsigned short>& slots
        )  = 0;
        virtual void endCrate(
            int id, const std::vector<unsigned short>& slots
        )    = 0;
        virtual SettingsWriter* getWriter(unsigned short slotNum) = 0;
        
    private:
        std::vector<unsigned short> makeSlotVector() const;
        
    };
    
}                                     // namespace DDAS

#endif