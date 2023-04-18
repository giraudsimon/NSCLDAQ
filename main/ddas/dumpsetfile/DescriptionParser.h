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

/** @file:  DescriptionParser.h
 *  @brief: Parses a description file (Pixie16DSP.var) for the DSP vars.
 */

#ifndef DESCRIPTIONPARSER_H
#define DESCRIPTIONPARSER_H

#include <iostream>
#include <cstdint>
#include <vector>
#include <string>

class DescriptionParser
{
    // Public types:
    
public:
    // Contains information about a single DSP Parameter:
    // Note
    typedef struct _DSPParameter {
        std::uint32_t s_offset;            // Offset in data block to parameter.
        unsigned      s_qty;               // Number of elements making up parameter.
        std::string  s_name;              // Name of element.
    } DSPParameter, *pDSPParameter;
    
    // Attributes:
    
private:
    std::vector<DSPParameter>   m_vars;
    
    // Methods:
    
public:
    void parseFile(std::istream& f);            // Parse the description from a source.
    std::vector<DSPParameter> getDescription();
};

#endif                                    // multi include guard.