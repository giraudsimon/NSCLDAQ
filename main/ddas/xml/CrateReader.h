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

/** @file:  CrateReader.h
 *  @brief: Defines a class that can read a crate.
 */
#ifndef CRATE_READER_H
#define CRATE_READER_H
#include "ModuleSettings.h"
#include <vector>


namespace DDAS {
    class SettingsReader;

    /**
     * @class CrateReader
     *     This class is able to read and process a crate
     * The first <slot> element is id 0 and so on.  This is
     * an abstract base class that is used to fetch the
     * configuration of a crate from any of a number of
     * types of places depending on the type of concrete
     * subclass.
     *
     */
    class CrateReader {
    protected:
        std::vector<unsigned short> m_slots;
        unsigned                    m_crateId;
    public:
        CrateReader() {}
        CrateReader(unsigned crate, const std::vector<unsigned short>& slots);
        virtual ~CrateReader();
    private:
        CrateReader(const CrateReader&);
        CrateReader& operator=(const CrateReader& );
        int operator==(const CrateReader&) const;
        int operator!=(const CrateReader&) const;
    public:
        // These methods form a strategy pattern which can
        // be completely overidden if needed.
        
        virtual Crate readCrate();
        virtual SettingsReader* createReader(unsigned short slot) = 0;
    };
     

}                                    // End namespace DDAS

#endif