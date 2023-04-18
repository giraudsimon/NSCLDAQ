/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Jeromy Tompkins 
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  main.cpp
 *  @brief: Program entry
 */
#include <cstdlib>
#include "DescriptionParser.h"
#include "DSPFileReader.h"
#include <fstream>
#include <stdexcept>

static void Usage()
{
    std::cerr << "Usage:\n";
    std::cerr << "  dumpSetFile set-file var-file\n";
    std::cerr << "Where:\n";
    std::cerr << "    set-file   - path to a .set file from e.g. nscope\n";
    std::cerr << "    var-file   - path to DSP variable definition file\n";
}

static void printModule(const void* pSettings, const std::vector <DescriptionParser::DSPParameter>& d)
{
    const std::uint8_t* p = reinterpret_cast<const std::uint8_t*>(pSettings);
    
    std::cout << "------------------------------------------------------------\n";
    for (int i =0; i < d.size(); i++) {
        DescriptionParser::DSPParameter descrip = d[i];
        const std::uint32_t* pParam = reinterpret_cast<const std::uint32_t*>(&(p[descrip.s_offset]));
        std::cout << descrip.s_name << ":\t";
        std::string leader="";
        for (int o = 0; o < descrip.s_qty; o++) {
            std::cout << leader << pParam[o] << " 0x" 
                << std::hex << pParam[o] << std::dec << std::endl;
            leader = "\t\t";
        }
    }
}

int main (int argc, char** argv)
{
    // Require a .set file and  a .var file:
    
    if (argc != 3) {
        Usage();
        std::exit(EXIT_FAILURE);
    }
    DescriptionParser varFile;
    DSPFileReader     setFile;
    
    try {
        setFile.readSetFile(std::string(argv[1]));
        if (setFile.getModuleCount() > 0) {
            std::ifstream vf(argv[2]);
            if (!vf) {
                throw std::runtime_error("Failed to open .var file");
            }
            varFile.parseFile(vf);
            vf.close();
            std::vector<DescriptionParser::DSPParameter> d = varFile.getDescription();
            
            // Iterate over the modules:
            
            for (int i = 0; i < setFile.getModuleCount(); i++) {
                const void* pVars = setFile.getModuleParameters(i);
                printModule(pVars, d);
            }
            
            
        }
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        std::exit(EXIT_FAILURE);
    }
    catch (std::string& msg) {
        std::cerr << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
    
    
    return EXIT_SUCCESS;
}