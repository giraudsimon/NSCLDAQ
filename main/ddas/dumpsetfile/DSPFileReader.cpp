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

/** @file:  DSPFileReader.cpp
 *  @brief: Implement DSPFileReader class (see header).
 *
 */

#include "DSPFileReader.h"
#include <sys/types.h>                 //  + This set of includes so we can use
#include <sys/stat.h>                  //  | the low level I/O functions to 
#include <fcntl.h>                     //  | Read the file.
#include <unistd.h>                    //<-+

#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <iostream>


void
DSPFileReader::readSetFile(std::string filename)
{
    int fd = openFile(filename);
    try {
        size_t bytesPerModule = N_DSP_PAR * sizeof(std::uint32_t);
        off_t bytesInFile = getFileLength(fd);
        size_t nModules = bytesInFile /(bytesPerModule);
        if(bytesInFile % (bytesPerModule)) {
            std::cerr <<  "The file ends with only a part of a module\n";
            std::cerr <<  "We'll continueaand ignore the stuff at the end but \n";
            std::cerr <<  "be sure this is actually a .set file\n";
        }
        for (int module = 0; module < nModules; module++) {
            std::uint32_t* params = new ModuleParams;
            m_moduleParameters.push_back(params);   // so emtpyVector kills it.
            readBlock(fd, params, bytesPerModule);
            
        }
    }
    catch (...) {
        close(fd);
        emptyVector();
        throw;
    }
    close(fd);
}
size_t
DSPFileReader::getModuleCount() const
{
    return m_moduleParameters.size();
}
const void*
DSPFileReader::getModuleParameters(size_t which)
{
    if (which < m_moduleParameters.size()) {
        return m_moduleParameters[which];
    } else {
        throw std::range_error("Selecting a module in getModuleParameters");
    }
}

/*------------------------------------------------------------------------------
 *   Private utility methods.
 */

/**
 * openFile
 *    Opens a file descriptor on the specified file for read.
 *    errors will throw an std::runtime_error.
 *
 *  @param name - name of the file to open.
 *  @return int - file descriptor on success.
 *  @throw  std::runtime_error on failure.
 */
int
DSPFileReader::openFile(std::string name)
{
    int fd = open(name.c_str(), O_RDONLY);
    if (fd < 0) {
        std::string reason = strerror(errno);
        std::string msg = "Failed to open file: ";
        msg            += name;
        msg            += " ";
        msg            += reason;
        throw std::runtime_error(msg);
    }
    return fd;
}
/**
 * readBlock
 *    Reads a block of data from the file.  If necessary, the primitive read
 *    is retried until all requested bytes are read. Retries are needed if:
 *    -  amount of data read is non zero but less than the remaining byts.
 *    -  The read failed (returned -1) but errno is one of EINTR, or EAGAIN
 *       The file descriptor is assumed to be in blocking mode so we don't
 *       handle EWOULDBLOCK.
 *
 * Note that hitting an end file prior to reading all of the data is considered
 * a sin.
 *
 * @param fd      - File descriptor on which to read.
 * @param pBuffer - Pointer to the buffer into which to read data.
 * @param nBytes  - Total number of bytes of data to read.
 * @throw std::runtime_error - in the event we're not able to satisfy the reads.
 */
void
DSPFileReader::readBlock(int fd, void* pBuffer, size_t nBytes)
{
    std::uint8_t* p = reinterpret_cast<std::uint8_t*>(pBuffer);
    
    while (nBytes) {
        ssize_t nRead = read(fd, p, nBytes);
        if (nRead < 0) {
            if ((errno != EINTR) && (errno != EAGAIN)) {
                std::string reason = strerror(errno);
                std::string msg = "Read failed to complete: ";
                msg            += reason;
                throw msg;
            }
        } else if (nRead == 0) {
            // End file prior to end read:
            
            throw std::string("Read failed to complete before encountering EOF");
        }
        // Prepare to read the next chunk
        
        nBytes -= nRead;
        p      += nRead;
    }
}
/**
 * getFileLength
 *    @return number of bytes in a file opened with open(2)
 *    @note after this method, the file will be positioned at the beginning.
 */
off_t
DSPFileReader::getFileLength(int fd)
{
    off_t result = lseek(fd,  0, SEEK_END);        // This tell us size.
    lseek(fd, SEEK_SET, 0);                       // Reset to BOF.
    if (result < 0) {
        std::string reason = strerror(errno);
        std::string msg = "Unable to determine length of the file: ";
        msg            += reason;
        throw msg;
    }
    return result;
}

/**
 * emptyVector
 *    Kill off the storage associated with m_moduleParameters.
 */
void
DSPFileReader::emptyVector()
{
    for (int i =0; i < m_moduleParameters.size(); i++) {
        delete [](m_moduleParameters[i]);
    }
    m_moduleParameters.clear();
}
