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

/** @file:  ToCrateMain.cpp
 *  @brief: Main program for the tocrate program.
 */
#include "tocrate.h"
#include <SetFileCrateReader.h>
#include <XMLCrateReader.h>
#include <PixieCrateWriter.h>
#include <CrateManager.h>
#include <config.h>
#include <config_pixie16api.h>
#include "ToUtils.h"

#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string>
#include <limits.h>
#include <algorithm>
#include <iterator>


/**
 * getCrateId
 *    Return the crate id from the parsed arguments or throw if
 *    it was not given.
 * @param args  - parsed arguments.
 * @return unsigned short - the crate id.
 * @throw std::invalid_argument if --crate is not given.
 */
static unsigned short
getCrateId(const gengetopt_args_info& args)
{
    if (!args.crate_given) {
        throw std::invalid_argument(
            "If --source=setfile the --crate flag must give a crate id"
        );
    }
    if (args.crate_arg < 0) {           // crate_arg is short not ushort.
        std::stringstream msg;
        msg << "The --crate id argument must be positive but was: "
            << args.crate_arg;
        throw std::invalid_argument(msg.str());
    }
    return args.crate_arg;
}
/**
 * createSetFileReader
 *    Create and return a set file reader given the setfile name
 *    and remaining required command line parameters.
 * @param file - filename of the setfile
 * @param crateid - the crate id to assign.
 * @param args - The parsed argument struct.
 * @return DDAS::SetFileCrateReader*
 */
static DDAS::CrateReader*
createSetFileReader(
    const std::string& file, unsigned short crateid,
    const gengetopt_args_info& args
)
{
    // Create the default speed vector and slots vector.
    
    std::vector<unsigned short> speeds;
    std::vector<unsigned short> slots;
    for (int i =0; i < args.slot_given; i++) {
        slots.push_back(args.slot_arg[i]);
        speeds.push_back(250);             // Default speeds.
    }
    if (slots.size() == 0) {
        // no slots:
        
        throw std::invalid_argument(
            "When --source=setfile there must be at least one --slot flag"
        );
    }
    // Correct the speeds given --msps values:
    
    for (int i =0; i < args.msps_given; i++) {
        std::pair<unsigned short, unsigned short> slotAndSpeed =
            parseMspsOption(args.msps_arg[i]);
            unsigned short slot = slotAndSpeed.first;
            unsigned short speed = slotAndSpeed.second;
            
            auto p = std::find(slots.begin(), slots.end(), slot);
            if (p == slots.end()) {
                std::stringstream msg;
                msg << "The slot in the --msps value: "
                    << args.msps_arg[i] << " is not a --slot";
                throw std::invalid_argument(msg.str());
            }
            unsigned id = std::distance(slots.end(), p);
            speeds[id] = speed;
    }
    // Ok Now make all the VAR files:
    
    std::vector<std::string> varFiles;
    for (int i =0; i < speeds.size(); i++) {
        varFiles.push_back(DDAS::CrateManager::getVarFile(speeds[i]));
    }
    return new DDAS::SetFileCrateReader(
        file.c_str(), crateid, slots, varFiles, speeds
    );

}
/**
 * reportDisallowedXMLFlags
 *   Throw std::invalid_argument exceptions if there are
 *   options present that are not allowed for XML source.
 *
 *  @param args - parsed argument struct.
 */
static void
reportDisallowedXMLFlags(const gengetopt_args_info& args)
{

    std::string   flag;
    if (args.crate_given) {
        flag = " --crate";
    }
    if (args.slot_given) {
        flag += " --slot";
    }
    if (args.msps_given) {
        flag += " --msps";
    }
    if (flag != "") {
        std::stringstream msg;
        msg << " The following flags were given but are not allowed if --source=xml: "
            << flag;
        throw std::invalid_argument(msg.str());
    }
}
/**
 *  bootModules
 *    Do a full boot on all modules in the crate definition.
 *
 * @param crateSettings - crate settings read from the source.
 */
static void
bootModules(const DDAS::Crate& crateSettings)
{
    // We need to build a CrateManager as that's what can
    // boot these modules.
    
    std::vector<unsigned short> slots = makeSlotVector(crateSettings);
    DDAS::CrateManager crate(slots);
    for (int i = 0; i < slots.size(); i++) {
        crate.fullBoot(i);   // Id is slot index.
    }
}
/**
 * Main
 *    Main logic flow of the program.
 * @param args - parsed argument struct.
 * @return int - EXIT_SUCCESS or EXIT_FAILURE - desired program exit code.
 */
static int
Main(const gengetopt_args_info& args)
{
    // We need the name of the file regardless:
    
    std::string    infile = args.file_arg;
    unsigned short crateId(0xffff);              // Either from the XML or --crate.
    
    // We need a reader to get the Crate configuration from the
    // input file regardless of what it is.
    
    DDAS::CrateReader* pReader(nullptr);
    
    if (args.source_arg == source_arg_setfile) {            // setfile
        crateId = getCrateId(args);
        pReader = createSetFileReader(infile, crateId, args);
        
    } else if (args.source_arg == source_arg_xml) {         // XML file.
        reportDisallowedXMLFlags(args);
        DDAS::XMLCrateReader* creader =
            new DDAS::XMLCrateReader(infile.c_str());
        crateId = creader->getCrateId();
        pReader = creader;
        
    } else {                   // I believe cmdline_parser prevents this branch.
        std::stringstream msg;
        msg << "--source value must be either 'setfile' or 'xml' was: "
            << args.source_orig;
        throw std::invalid_argument(msg.str());
    }
    // Read the crate:
    
    DDAS::Crate crateSettings = pReader->readCrate();
    // Do we need to do full module boots?
    
    if (args.fullboot_given) {
        bootModules(crateSettings);
    }
    
    // We need a PixieCrateWriter to write the settings
    
    DDAS::PixieCrateWriter writer(crateSettings);
    writer.write();
    
    
    return EXIT_SUCCESS;
}

/**
 * main 
 *    Entry point wraps Main with a try catch block after parsing the
 *    arguments.
 *  @param argc - number of command line words.
 *  @param argv - the command line words after any shell substitutions.
 */
int
main(int argc, char** argv)
{
    gengetopt_args_info args;
    cmdline_parser(argc, argv, &args);         // Exits on errors.
    
    int status;    
    try {
        status = Main(args);
    }
    catch (std::exception & e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
    exit(status);
}
