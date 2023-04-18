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

/** @file:  PixieCrateWriter.h
 *  @brief: Define a class to write settings to a crate of Pixie16 modules.
 */
#ifndef PIXIECRATEWRITER_H
#define PIXIECRATEWRITER_H
#include "CrateWriter.h"

namespace DDAS {
class CrateManager;

/**
 * @class PixieCrateWriter
 *      Crate writer that provides strategy methods to support
 *      writing a crate full of Pixie16 digitizers.
 *      This software uses a CrateManager to ensure the API is initialized
 *      and that the digitizer DSP parameter address maps get loaded.
 */
class PixieCrateWriter : public CrateWriter {
private:
    CrateManager*   m_pCrate;
    std::vector<unsigned short> m_slots;
    unsigned                    m_crateId;
public:
    PixieCrateWriter(const Crate& settings);
    virtual ~PixieCrateWriter();
    
    virtual void startCrate(
            int id, const std::vector<unsigned short>& slots
        );
    virtual void endCrate(
        int id, const std::vector<unsigned short>& slots
    );
    virtual SettingsWriter* getWriter(unsigned short slotNum);
private:
    void writeModuleParam(
        unsigned short id, const char* pname, unsigned int data
    );
};

}                                  // DDAS Namespace.

#endif