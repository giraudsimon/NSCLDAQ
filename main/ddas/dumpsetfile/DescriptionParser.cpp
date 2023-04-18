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

/** @file:  DescriptionParser.cpp
 *  @brief: Implement DescriptionParser class
 */

#include "DescriptionParser.h"

static const std::uint32_t  BASE_ADDRESS(0x0004a000);  // Base of DSP param memory
/**
 *  parseFile
 *      Parses a file creating a new m_vars vector (old one is first cleared)
 *
 *  @param f   - std::istream open on the input file.
 */
void
DescriptionParser::parseFile(std::istream& f)
{
    m_vars.clear();                // Start with new parameters.
    while (!f.eof()) {
        std::uint32_t addr;
        std::string   name;
        
        f >> std::hex >> addr >> name;
        
        if(!f.fail()) {
            DSPParameter p;
            p.s_offset = sizeof(std::uint32_t)*(addr - BASE_ADDRESS);
            p.s_qty    = 1;
            p.s_name   = name;
            m_vars.push_back(p);
            
            // If this offset skips ahead by more than a long word from
            // the prior one, we need to adjust the qty of the last entry
            // accordingly:
            
            if (m_vars.size() > 1) {
                DSPParameter& prior = m_vars[m_vars.size()-2];
                std::uint32_t longsdiff = (p.s_offset - prior.s_offset)/sizeof(std::uint32_t);
                prior.s_qty = longsdiff;
            }
        }
    }
    
}

/**
 *  getDescription
 *
 *  @return m_vars - the most recently parsed description.
 */
std::vector<DescriptionParser::DSPParameter>
DescriptionParser::getDescription()
{
    return m_vars;
}