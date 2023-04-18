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

/** @file:  tosetfileMain.cpp
 *  @brief: Converts XML crate settings to a set file.
 */

#include "tosetfile.h"
#include "ToUtils.h"
#include "XMLCrateReader.h"
#include "SetFileCrateWriter.h"

#include <stdlib.h>
#include <stdexcept>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <stdint.h>

/**
 * Main
 *    The main logic flow.
 *    This is separated so that exceptions can be thrown but handled
 *    by the actual main.
 * @param args - parsed arguments.
 * @return int status
 */
int Main(const gengetopt_args_info& args)
{
    std::string crateFile = args.xml_arg;
    std::string setFile   = args.setfile_arg;
    
    // Process the stlot speeds:
    
    std::vector<std::pair<uint16_t, uint16_t>> slotSpecs;
    for (int i =0; i < args.msps_given; i++) {
        auto spec = parseMspsOption(args.msps_arg[i]);
        std::pair<uint16_t, uint16_t> p = {uint16_t(spec.first), uint16_t(spec.second)};
        slotSpecs.push_back(p);
    }
    // All the arguments were good.  Read the XML file into a crate:
    
    DDAS::XMLCrateReader reader(crateFile.c_str());
    DDAS::Crate c  = reader.readCrate();
    
    // Build the SetFileCrateWriter and write (it really is that simple).
    
    DDAS::SetFileCrateWriter writer(setFile.c_str(), c, slotSpecs);
    writer.write();
    
    return EXIT_SUCCESS;
}

/**
 * main
 *    Entry point
 */
int
main(int argc, char** argv)
{
    gengetopt_args_info args;
    cmdline_parser(argc, argv, &args);
    
    try {
        Main(args);
    }
    catch (std::exception &e) {
        std::cerr << e.what();
        exit(EXIT_FAILURE);
    }
}