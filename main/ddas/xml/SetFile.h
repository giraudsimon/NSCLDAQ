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

/** @file:  SetFile.h
 *  @brief: Define structures and funcions for setfiles.
 */
#ifndef SETFILE_H
#define SETFILE_H
#include <string>
#include <map>
#include <vector>

#include <stdint.h>


namespace DDAS {
    // The VAR file actually is a DSP memory layout file for the chunk of
    // DSP memory that holds the DSP parameters.  This memory
    // starts at DSP_BASE below. File offsets are computed from VAR file numbers
    // as (varfileNumber - DSP_BASE ) * sizeof(uint32_t)
    //
    static const unsigned DSP_BASE=0x4a000;  
    /**
     *  @struct VarDescriptor
     *
     *  This structure documents a single item in the set file
     *  it provides:
     *  -  s_name    - The name of the item.
     *  -  s_longoff - The number of longwords into the file the item lives
     *  -  s_nLongs  - The number of longwords the item occupies (1 or 16 usually).
     */
    struct VarDescriptor {
        std::string s_name;
        unsigned    s_longoff;
        unsigned    s_nLongs;
        
        VarDescriptor() {}
        ~VarDescriptor() {}
        VarDescriptor(const VarDescriptor& rhs) {
            copyin(rhs);
        }
        VarDescriptor& operator=(const VarDescriptor& rhs) {
            if (this != &rhs) {
                copyin(rhs);
            }
            return *this;
        }
        void copyin(const VarDescriptor& rhs) {
            s_name    = rhs.s_name;
            s_longoff = rhs.s_longoff;
            s_nLongs  = rhs.s_nLongs;
        }
    };
    
    /**
     *  @typedef VarOffsetArray
     *     This is just a vector of VarDescriptor  It can be used to sequentially
     *     examine a set file:
     */
    
    typedef std::vector<VarDescriptor> VarOffsetArray;
    
    /**
     * @typedef VarMapByOffset
     *     Map of VarDescriptor's addressable by offsets into the DSP Var space.
     */
    typedef std::map<uint32_t, VarDescriptor> VarMapByOffset;
    
    /**
     * @typedef VarMapByName
     *   Map of VarDescriptors by name:
     */
    typedef std::map<std::string, VarDescriptor> VarMapByName;
    
    /**
     *  @union Variable
     *     Contains a single variable.  This can be either a scaler
     *     or a vector of s_nLongs:
     */
    typedef struct _Variable {
        VarDescriptor s_desc;
        uint32_t              s_value;          // Module level e.g.
        std::vector<uint32_t> s_values;         // Channel level e.g.
        _Variable() {}
        ~_Variable() {}
        _Variable(const _Variable& rhs) {
            copyin(rhs);
        }
        _Variable& operator=(const _Variable& rhs) {
            if (this != &rhs) {
                copyin(rhs);
            }
            return *this;
        }
        void copyin(const _Variable& rhs) {
            s_desc = rhs.s_desc;
            s_value = rhs.s_value;
            s_values = rhs.s_values;
        }
    } Variable;
    
    /**
     * @typedef UnpackedSetFile
     *    Just a vector of Variables:
     */
    typedef std::vector<Variable> UnpackedSetFile;
    
    /**
     * @typedef SetFileByName
     *    Set file values organized by name:
     */
    typedef std::map<std::string, Variable> SetFileByName;
    
    /**
     * @class SetFile
     *     A bunch of static methods that can manipulate set files:
     */
    class SetFile {
    public:
        // Var definition files:
        
        static VarOffsetArray readVarFile(const char* filename);
        static VarMapByOffset createVarOffsetMap(const VarOffsetArray& offsets);
        static VarMapByName   createVarNameMap(const VarOffsetArray& offsets);
         
        // set files I/O and resource management:
        
        static std::pair<unsigned, uint32_t*> readSetFile(const char* filename);
        static void writeSetFile(
            const char* filename, unsigned nLongs, uint32_t* pVars
        );
        static void freeSetFile(uint32_t* pVars);
        
       
        static UnpackedSetFile populateSetFileArray(
            unsigned nLongs, const uint32_t* pVars,
            const VarOffsetArray& map
        );
        static SetFileByName populateSetFileMap(
            unsigned nLong, const  uint32_t* pVars,
            const VarOffsetArray& map
        );
        static SetFileByName populateSetFileMap(
            const UnpackedSetFile& vars
        );
    private:
        static Variable getVariable(
            unsigned nLong, const uint32_t* pVars, const VarDescriptor& d
        );
    };
    
}                                    // DDAS Namespace.

#endif