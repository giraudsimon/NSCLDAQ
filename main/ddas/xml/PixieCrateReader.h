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

/** @file:  PixieCrateReader
 *  @brief: Read crate configuration from live Pixie crate
 */
#ifndef PIXIECRATEREADER_H
#define PIXIECRATEREADER_H
#include "CrateReader.h"

namespace DDAS {

class CrateManager;

/**
 * @class PixieCrateReader
 *    This is a crate reader that reads the configuration
 *    of an entire crate from the Pixie/cPCI crate.  Caveats:
 *
 *    *  The crate id passed in at construction time is used
 *       for the crate id regardless of the crate id configured in
 *       the modules.
 *    *  The slots parameters must be the same as those
 *       passed into the Pixie16InitSystem or the CrateManager
 *       constructor if that's being used.
 */
class PixieCrateReader : public CrateReader
{
private:
    CrateManager* m_pManager;
public:
    PixieCrateReader(
        unsigned crate, const std::vector<unsigned short>& slots
    );
    virtual ~PixieCrateReader();
    
    // Provide a PixieSettingsReader for the specified slots.
    
    virtual SettingsReader* createReader(unsigned short slot);
};

}                              // DDAS Namespace.
#endif