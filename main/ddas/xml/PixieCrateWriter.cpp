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

/** @file:  PixieCrateWriter.cpp
 *  @brief: Implement the PixieCrateWriter class.
 */

#include "PixieCrateWriter.h"
#include "CrateManager.h"
#include "ModuleWriter.h"
#include <config.h>
#include <config_pixie16api.h>
#include <sstream>
#include <stdexcept>

namespace DDAS {
    
    /** constructor
     *    Construct the crate writer. We just pass the
     *    settings on to the base class.
     * @param settings - crate settings.
     */
    PixieCrateWriter::PixieCrateWriter(const Crate& settings) :
        CrateWriter(settings),
        m_pCrate(nullptr),
        m_crateId(0)
    {}
    /**
     * destructor
     *   ensure the crate manager is destroyed.  That does an exit system.
     */
    PixieCrateWriter::~PixieCrateWriter()
    {
        delete m_pCrate;
    }
    /**
     * startCrate
     *    Called before the crate is written.
     *    Save the crate id.
     *    Save the slots so we can compute ids.
     *    Create the crate manager.
     *
     * @param id - the crate id.
     * @param slots - The slot map.
     */
    void
    PixieCrateWriter::startCrate(
        int id, const std::vector<unsigned short>& slots
    )
    {
        m_pCrate  = new CrateManager(slots);
        m_crateId = id;
        
        
    }
    /**
     * endCrate
     *   ALl modules are now programmed with their crate ids.  
     *   The crate manager is also deleted, which
     *   does a Pixie16ExitSystem.
     *
     *  @param id - the crate id.
     *  @param slots - the slot map.
     */
    void
    PixieCrateWriter::endCrate(
        int id, const std::vector<unsigned short>& slots
    )
    {
        
        delete m_pCrate;
        m_pCrate = nullptr;
    }
    /**
     * getWriter
     *   Produce a ModuleWriter which will write the settings to a module.
     *   Note that we'll write the slotid and the module id parameters
     *   directly ourselves.
     *
     * @param slot - the slot number to write.
     * @note the crate manager must have been instantiated already.
     */
    SettingsWriter*
    PixieCrateWriter::getWriter(unsigned short slot)
    {
        // Turn slot into id:
        
        unsigned short id = m_pCrate->moduleId(slot);
        
        // Load the DSP address map:
        
        m_pCrate->loadDSPAddressMap(id);
        
        // Program the slot id and and ModId:
        
        writeModuleParam(id, "SlotID", slot);
        writeModuleParam(id, "ModID", id);
        writeModuleParam(id, "CrateID", m_crateId);
        
        // Return the ModuleWriter
        
        return new ModuleWriter(m_crateId, slot, id);
        
    }
    ////////////////////////////////////////////////////////////
    //  Private utilities.
    
    /**
     * writeModuleParam
     *    Wraps Pixie16WriteSglModPar inside of error
     *    checking.
     * @param id -module id.
     * @param pname  - Parameter name C string.
     * @param data   - Data to write.
     * @throw std::invalid_argument if the write fails.
     */
    void
    PixieCrateWriter::writeModuleParam(
        unsigned short id, const char* pname, unsigned int data
    )
    {
        int stat = Pixie16WriteSglModPar(pname, data, id);
        if (stat < 0) {
            std::stringstream s;
            switch (-stat) {
                case 1:
                    s << " Invalid pixie id number: " << id;
                    break;
                case 2:
                    s << "Invalid Pixie module parameter name: "
                        << pname;
                    break;
                case 3:
                    s << "Failed to program Fippi after setting parameter";
                    break;
                case 4:
                    s << "Failed to compute BLCut after setting parameter";
                    break;
                default:
                    s << "Undocumented error from WriteSglModPar "
                        << stat;
                    break;
                
            }
            throw std::invalid_argument(s.str());
        }
    }
}                      // Namespace 
