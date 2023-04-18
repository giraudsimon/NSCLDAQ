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

/** @file:  SetFile.cpp
 *  @brief: Implement methods in the SetFile class.
 */

#include "SetFile.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


namespace DDAS {

/**
 * readVarFile
 *    Reads a Var file and provides descriptors for the variables it defines
 *    relative to the start of either the DSP var memory in the boards
 *    or relative to the file itself.
 *
 * @param  filename - Pointer to the filename to open.
 * @return VarOffsetArray - Resulting description of all the variables.
 * @note the array is in order of the definitions in the var file.
 */
VarOffsetArray SetFile::readVarFile(const char* filename)
{
    // Open the file and whine if that fails:
    
    std::ifstream  f(filename);
    if (!f) {
        // Unfortunately iostreams is pretty opaque about why this failed.
        
        std::stringstream msg;
        msg << "readVarFile unable to open " << filename;
        throw std::invalid_argument(msg.str());
    }
    
    // The file consists of lines that contain hex offsets from DSP_BASE
    // followed by a word describing the data  there.
    
    VarOffsetArray result;
    f >> std::hex;                       // NO decimal data here.
    
    while (!f.eof()) {
        VarDescriptor item;
        uint32_t    offset;
        std::string name;
        
        f >> offset >> name;
        if (!f.fail()) {                    // iostreams parse worked.
            offset -= DSP_BASE;
            item.s_name    = name;
            item.s_longoff = offset;
            item.s_nLongs  = 1;             // Default:
            result.push_back(item);
            
            // If there are at least two items we can see if the next to last
            // needs adjustment.  We've got no choice but to guess about
            // the last one:
            
            if (result.size() >= 2) {
                VarDescriptor& prior = result[result.size() - 2];
                VarDescriptor& last  = result.back();
                prior.s_nLongs = last.s_longoff - prior.s_longoff;
            }

        }
    }
    f.close();
    return result;
}

/**
 * createVaroffsetMap
 *    Create a map organized by offset values from the array of descriptors
 *    created by readVarFile.
 *
 *  @param offsets - the array of offset descriptors.
 *  @return VarMapByOffset - map of those descriptors organized by offset:
 */
VarMapByOffset
SetFile::createVarOffsetMap(const VarOffsetArray& offsets)
{
    VarMapByOffset result;
    for (int i =0; i < offsets.size(); i++) {
        result[offsets[i].s_longoff] = offsets[i];
    }
    
    return result;
}
/**
 * createVarNameMap
 *    Takes an array of offset descriptors from a var file and tossses them up
 *    into  a map that's indexed by variable name.
 *
 *  @param offsets - the offset array descriptor.
 *  @return VarMapByName - the map.
 */
VarMapByName
SetFile::createVarNameMap(const VarOffsetArray& offsets)
{
    VarMapByName result;
    for(int i =0; i < offsets.size(); i++) {
        result[offsets[i].s_name]  = offsets[i];
    }
    
    return result;
}
/**
 * readSetFile
 *    Reads the set file in to a soup of storage.
 *    - Figures out how many bytes are in the file
 *    - allocates  a buffer for that.
 *    - opens the file and suckes it all into the buffer.
 *
 * @param filename - path to the .set file.
 * @return std::pair<unsigned, uint32_t*> - first is number of longs, second pointer
 *                    to the file contents.
 */
std::pair<unsigned, uint32_t*>
SetFile::readSetFile(const char* filename)
{
    struct stat info;
    int status = stat(filename, &info);
    if (status == -1) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to stat : " << filename << " : " << strerror(e);
        throw std::invalid_argument(msg.str());
    }
    size_t fileLen = info.st_size;
    uint32_t* pData = static_cast<uint32_t*>(malloc(fileLen));
    
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to open " << filename << " For read : " << strerror(e);
        throw std::invalid_argument(msg.str());
        
    }
    
    ssize_t nRead = read(fd, pData, fileLen);
    if (nRead < 0) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to read from : " << filename << " : " << strerror(e);
        throw std::invalid_argument(msg.str());
    }
    close(fd);               // Not really sure what to do on failure.
    
    if (nRead < fileLen) {
        std::stringstream msg;
        msg << "Only able to partially read the file : " << filename << " "
            << nRead << "/" << fileLen << " bytes read";
        throw std::invalid_argument(msg.str());
    }
    
    return {fileLen/sizeof(uint32_t), pData};
    
}
/**
 * freeSetFile
 *    Free the setfile data gotten by readSetFile
 *
 * p  - pointer to data returned from readSetFile
 */
void
SetFile::freeSetFile(uint32_t* pVar)
{
    free(pVar);
}
/**
 * writeSetFile
 *    Writes a set file back out to disk.
 * @param filename - name of the file to write to.
 * @param nLongs   - Number of longs in the set file.
 * @param pVars    - Pointer to the setfile data in memory.
 *
 */
void
SetFile::writeSetFile(
    const char* filename, unsigned nLongs, uint32_t* pVars 
)
{
    size_t fd = creat(filename, S_IRUSR | S_IWUSR | S_IRGRP);
    if (fd < 0) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to open file: " << filename << " for write: "
            << strerror(e);
        throw std::invalid_argument(msg.str());
    }
    size_t nBytes = nLongs*sizeof(uint32_t);
    ssize_t nWritten = write(fd, pVars, nBytes);
    close(fd);
    
    if (nWritten < 0) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to write data to : " << filename 
            <<  " : " << strerror(e);
        throw std::invalid_argument(msg.str());
    }
    if (nWritten != nBytes) {
        std::stringstream msg;
        msg << "Only able to write " << nWritten << "/" << nBytes
            << " of data to " << filename;
        throw std::invalid_argument(msg.str());
    }
    
}
/**
 * populateSetFileArray
 *    Given the contents of a setfile and its variable offset
 *    definitions, returns a vector of set file descriptor/contents.
 *
 *  @param nLongs - number of longs in the set file data.
 *  @param pVars  - Buffer contaning the set file data.
 *  @param map    - vars definition.  Assumed sorted by offset.
 *  @return UnpackedSetFile - vector of variable value definitions.
 */
UnpackedSetFile
SetFile::populateSetFileArray(
    unsigned nLongs, const uint32_t* pVars,
    const VarOffsetArray& map
)
{
    UnpackedSetFile result;
    
    // Loop over the map
    // - Ensure the vars are big enough to hold the data.
    // - Pull each item from vars into a Variable and
    // - Push that into result.
    
    for (int i =0; i < map.size(); i++) {
        const VarDescriptor& d(map[i]);
        
        result.push_back(getVariable(nLongs, pVars, d));
    }
    
    return result;
}
/**
 *  populateSetFileMap
 *     Populates a set file map indexed by item  name.
 *
 * @param nLong  - Number of longs in the map.
 * @param pVars  - Pointer to the variables in the set file buffer.
 * @param map    - Variable mapping array.
 * @return SetFileByName
 */
SetFileByName
SetFile::populateSetFileMap(
    unsigned nLong, const uint32_t* pVars, const VarOffsetArray& map
)
{
    SetFileByName result;
    
    for (int i = 0; i < map.size(); i++) {
        Variable v =  getVariable(nLong, pVars, map[i]);
        result[v.s_desc.s_name] = v;
    }
    
    return result;
}
/**
 * populateSetFileMap
 *    Overload to populate the map given an unpacked setfile:
 *
 * @param vars - the unpacked set file.
 */
SetFileByName
SetFile::populateSetFileMap(
    const UnpackedSetFile& vars
)
{
    SetFileByName result;
    for (int i =0; i < vars.size(); i++) {
        result[vars[i].s_desc.s_name] = vars[i];
    }
    
    return result;
}

//////////////////////////////////////////////////////////////////////
// Utility methods:

/**
 * getVariable
 *   Get a single variable (module or per channel) from the setfile
 *   buffer.
 *
 * @param nLong - number of longs in the setfile buffer.
 * @param pVars - Pointer to the setfile buffer.
 * @param d     - Variable descriptor for what we're fetching.
 * @return Variable
 * @throws std::invalid_argument if the descriptor is outside the buffer.
 * 
 */
Variable
SetFile::getVariable(
    unsigned nLongs, const uint32_t* pVars, const VarDescriptor& d
)
{
    Variable v;
    v.s_desc = d;
    if (d.s_nLongs == 1) {
        if (d.s_longoff < nLongs) {
            v.s_value = pVars[d.s_longoff];
        } else {
            std::stringstream msg;
            msg << "Map element specifies an offset of"
                << d.s_longoff << " Longs but there are only "
                << nLongs << " long words in the data";
            throw std::invalid_argument(msg.str());
        }
     } else {
        // Ensure the whole array fits in the storage:
        
        if (d.s_longoff + d.s_nLongs < nLongs) {
            for (int i =0; i < d.s_nLongs; i++ ) {
                v.s_values.push_back(pVars[d.s_longoff+i]);
            }
        } else {
            std::stringstream msg;
            msg << "Map element  specifies and offset of: "
                << d.s_longoff << " and length of " << d.s_nLongs
                << " which won't fit in the " << nLongs
                << " of data";
            throw std::invalid_argument(msg.str());
        }
     }
     return v;
}

}                                      // DDAS Namespace.