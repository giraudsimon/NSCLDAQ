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

/** @file:  ToXmlMain.cpp
 *  @brief: Main program file for the toxml program.
 */
#include "toxml.h"
#include "ToUtils.h"
#include <XMLCrateReader.h>
#include <SetFileCrateReader.h>
#include <PixieCrateReader.h>
#include <ModuleSettings.h>
#include <XMLCrateWriter.h>
#include <CrateManager.h>

#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>
#include <iterator>


/**
 * updateSpeed
 *    Given the slots, tentaive speeds and the parsed arguments,
 *    update the tentative speeds based on --msps values in the
 *    command line.
 * @param slots - array of  slots.
 * @param[inout] mhz - Sampling speeds of those slots.
 * @param args   - parsed arguments.
 */
static void
updateSpeed(
    const std::vector<unsigned short>& slots,
    std::vector<unsigned short>&       mhz,
    const gengetopt_args_info&         args
)
{
    for (int i =0; i < args.msps_given; i++) {
        std::pair<unsigned, unsigned> speedSpec =
            parseMspsOption(args.msps_arg[i]);
        unsigned short slot = speedSpec.first;
        unsigned short speed = speedSpec.second;
        
        auto p = std::find(slots.begin(), slots.end(), slot);
        if (p == slots.end()) {
            std::stringstream msg;
            msg << args.msps_arg[i] << " specifies a nonexistent slot";
            throw std::invalid_argument(msg.str());
        }
    }
}
/**
 * getVarFiles
 *    Return the DSP variable map files associated with  the
 *    modules we read from the set file.   In fact at this point
 *    in time, all modules use the same DSP layout.  This
 *    function future proofs against the time that may not be the case.
 *
 *    We make the best guess about what information must be passed
 *    to choose a varfile:
 * @param slots - Vector of slots
 * @param mhz   - Speed information for the digitizers in the slots
 * @param args  - Parsed arguments (in case they wind up specified there).
 * @return std::vector<std::string - vector of varfile paths.
 */
static std::vector<std::string>
getVarFiles(
    const std::vector<unsigned short>& slots,
    const std::vector<unsigned short>& mhz,
    const gengetopt_args_info&        args
)
{
    std::vector<std::string> result;
    
    for (int i =0; i < mhz.size(); i++) {
        result.push_back(DDAS::CrateManager::getVarFile(mhz[i]));
    }
    
    return result;
}

/**
 * createSetFileReader
 *     Given the information we have about what we want to read,
 *     creates a set file reader.
 * @param args     - the parsed arguments
 * @param slotInfo - The slot information from the Crate XML file.
 * @param crateId  - The crate id of the crate.
 * @return DDAS::CrateReader* - dynamically created Set file reader.
 */
static DDAS::CrateReader*
createSetFileReader(
    const gengetopt_args_info& args,
    const std::map<unsigned short, DDAS::XMLCrateReader::SlotInformation>& slotInfo,
    unsigned short crateId
)
{
    // The --file is required. 
    
    if (!args.file_given) {
        throw std::invalid_argument("The --file argument is required for --source=setfile");
    }
    std::string setFile = args.file_arg;
    
    // Create the slot vector and tentative mhz vector (defaults to 250).
    
    std::vector<unsigned short> slots = makeSlotVector(slotInfo);
    std::vector<unsigned short> mhz;
    for (int i = 0; i < slots.size(); i++) {
        mhz.push_back(250);
    }
    // If the --msps option was given we need to update the
    // mhz vector in accordance with it.
    
    if (args.msps_given) {
        updateSpeed(slots, mhz, args);
    }
    
    // currently all module types have the same Varfile.
    // this future proofs that in case that changes:
    
    std::vector<std::string> varFiles = getVarFiles(slots, mhz, args);
    
    return new DDAS::SetFileCrateReader(
        setFile.c_str(), crateId, slots, varFiles, mhz
    );
    return nullptr;                   // Stub.
}
/** createModuleReader
 *     Given information we have about what the user wants, create a
 *     PixieCrateReader that can read settings directly from the modules
 *     in an XIA PXI crate.
 *  @param args     - the parsed arguments.
 *  @param slotInfo - Slot information from the Crate XML file.
 *  @param crateId  - Id of the crate desired.
 *  @return DDAS::CrateReader* - pointer to a dynamically created PixieCrateReader
 *                    object to read the settings.
 */
static DDAS::CrateReader*
createModuleReader(
    const gengetopt_args_info& args,
    const std::map<unsigned short, DDAS::XMLCrateReader::SlotInformation>& slotinfo,
    unsigned short crateId
)
{
    // The --file and the --msps flags are not allowed for
    // --source=modules
    
    if (args.file_given) {
        throw std::invalid_argument(
            "If --source=modules, the --file flag is not allowed"
        );
    }
    if (args.msps_given) {
        throw std::invalid_argument(
            "If --source=modules, the --msps flag is not allowed"
        );
    }
    
    
    // Create the slot map.  For the Pixie crate reader, that, and
    // the crate Id is all we need:
    
    std::vector<unsigned short> slots = makeSlotVector(slotinfo);
    return new DDAS::PixieCrateReader(crateId, slots);

}
/**
 * createMetadata
 *    Using the slot information from the crate reader, create
 *    the metadata vector needed by the XMLCrateWriter.
 *    We traverse the slot info map by iterator which ensures
 *    the slots come out in increasing order.
 *
 *  @param crate    - Crate settings (to get the slots).
 *  @param slotInfo - Slot information from the reader
 *  @return std::vector<DDAS::XMLCrateWriter::ModuleInformation>
 */
static std::vector<DDAS::XMLCrateWriter::ModuleInformation>
createMetadata(
     const DDAS::Crate& crate,
    const std::map<unsigned short, DDAS::XMLCrateReader::SlotInformation>& slotinfo
)
{
    std::vector<DDAS::XMLCrateWriter::ModuleInformation> result;
    auto& slots(crate.s_slots);
    for (int i =0; i < slots.size(); i++) {
        auto p = slotinfo.find(slots[i].s_slotNum);
        if (p == slotinfo.end()) {
            std::stringstream msg;
            msg << "There is no slot information for slot: "
                << slots[i].s_slotNum;
            throw std::invalid_argument(msg.str());
        }
        DDAS::XMLCrateWriter::ModuleInformation m;
        auto& info(p->second);
        
        m.s_eventLength = info.s_evtlen;
        m.s_moduleSettingsFile = info.s_configFile;
        m.s_specifyFifoThreshold = true;
        m.s_fifoThreshold = info.s_fifothreshold;
        m.s_specifyTimestampScale = true;
        m.s_timestampScale = info.s_timestampscale;
        m.s_specifyInfinityClock = true;
        m.s_infinityClock = info.s_infinityclock;
        m.s_specifyExternalClock = true;
        m.s_externalClock = info.s_externalclock;
        result.push_back(m);
    }
    
    return result;
}
/**
 * createWriter
 *   Creates the XML crate writer that will be used to write the settings
 *   to the XML files specified by the crate file.
 *
 * @param args    - Parsed command line arguments.
 * @param slotInfo - Information about the slots from the XML Crate file.
 * @param crate    - Crate settings to write.
 * @return DDAS::CrateWriter* - pointer to dynamically allocated XMLCrateWriter.
 */
static DDAS::CrateWriter*
createWriter(
    const gengetopt_args_info& args,
    const std::map<unsigned short, DDAS::XMLCrateReader::SlotInformation>& slotinfo,
    const DDAS::Crate& crate
)
{
    auto metadata = createMetadata(crate, slotinfo);
    return new DDAS::XMLCrateWriter(args.crate_arg, crate, metadata);

}
/**
 * Main
 *    This is wrapped by the actual main in a try/catch block
 *    for last chance error handling.
 *
 * @param args - the arguments parsed by gengetopt
 * @return int - Final program status.
*/
static int
Main(const gengetopt_args_info& args)
{
    // Process the crate file and use it to crate the metadata
    // required by the crate writer.
    
    DDAS::XMLCrateReader creader(args.crate_arg);
    const std::map<unsigned short, DDAS::XMLCrateReader::SlotInformation>&
        slotInfo(creader.getSlotInformation());
    unsigned short crateId = creader.getCrateId();
    
    // Figure out the source and use it to produce a crate reader:
    
    DDAS::CrateReader* pReader;
    if (args.source_arg == source_arg_setfile) {
        pReader = createSetFileReader(args, slotInfo, crateId);
    } else if (args.source_arg == source_arg_modules) {
        pReader = createModuleReader(args, slotInfo, crateId);
    } else {
        throw std::invalid_argument(
            "Invalid --source value. Must be 'setfile' or 'modules'"
        );
    }
    DDAS::Crate settings = pReader->readCrate();
    delete pReader;
    // Read the crate and then write the crate:
    
    DDAS::CrateWriter* pWriter = createWriter(args, slotInfo, settings);
    pWriter->write();
    delete pWriter;
    
    return EXIT_SUCCESS;
}
/**
 * main
 *    The main program.  Just parses the arguments and calls
 *    Main wrapped in a try/catch block.
 * @param argc - count of command line words.
 * @param argv - array of pointers to the command line words.
 * @return int - final program status.
 */
int
main(int argc, char** argv)
{
    gengetopt_args_info args;
    cmdline_parser(argc, argv, &args);              // Exits on error.
    try {
        return Main(args);
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    // Control should never land here:
    
    std::cerr << "Logic error - control should not land here\n";
    return EXIT_FAILURE;
}