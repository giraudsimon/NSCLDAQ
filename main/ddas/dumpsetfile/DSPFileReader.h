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

/** @file:  DSPFileReader.h
 *  @brief: Read DSP files into memory blocks.
 */

#ifndef DSPFILEREADER_H
#define DSPFILEREADER_H

#include <config.h>
#include <config_pixie16api.h>
#include <vector>
#include <cstdint>
#include <string>
#include <sys/types.h>                     // For off_t (I think).

class DSPFileReader
{
private:
    typedef std::uint32_t ModuleParams[N_DSP_PAR]; // Params for a module.
    std::vector<std::uint32_t*>  m_moduleParameters;
    
public:
    void readSetFile(std::string filename);
    size_t getModuleCount() const;
    const void* getModuleParameters(size_t which);
    
private:
    int  openFile(std::string name);
    void readBlock(int fd, void* pBuffer, size_t nBytes);
    off_t getFileLength(int fd);
    void emptyVector();
};

#endif
