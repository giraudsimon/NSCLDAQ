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

/** @file:  swFilterRingBlockDataSource.cpp
 *  @brief: Implement the swFilterRingBlockDataSource class.
 */

#include "swFilterRingBlockDataSource.h"
#include <CRingBlockReader.h>
#include <stdlib.h>
#include <string.h>
#include <new>

/**
 * constructor
 *    We just store away the reader object.
 *
 *  @param reader - the reader we'll use to transfer data in.
 *  @param readSize - the size of the read that will be givento
 *          CRingBlockReader::read.
 *  @note storage management/ownership remains with the caller.
 */
swFilterRingBlockDataSource::swFilterRingBlockDataSource(
    CRingBlockReader& reader, size_t readSize
) : m_Reader(reader), m_nReadSize(readSize)
{}

/**
 * destructor
 *    currently a no-op
 */
swFilterRingBlockDataSource::~swFilterRingBlockDataSource()
{
    
}
/**
 * connect
 *    The reader object is assumed to come preconnected.
 */
void
swFilterRingBlockDataSource::connect()
{
    
}
/**
 * disconnect
 *    It's assumed the caller will disconnect as part of its storage
 *    management.
 */
void
swFilterRingBlockDataSource::disconnect()
{
    
}
/**
 *   read
 *       Read a block of data and pass back  the pair where:
 *       - size is the size of a descsriptor
 *       - pointer is a pointer to a malloc/memcpy'd copy of the
 *         descriptor so that the storage can be free'd as
 *         expected.
 *
 *  @return std::pair<std::size_t, void*> - see above.
 *  @note if a descriptor with an end file is returned, the pair
 *        returned is {0, nullptr}
 */
std::pair<std::size_t, void*>
swFilterRingBlockDataSource::read()
{
    std::pair<std::size_t, void*> result = {0, nullptr};
    CRingBlockReader::DataDescriptor d = m_Reader.read(m_nReadSize);
    if (d.s_pData != nullptr) {
        CRingBlockReader::pDataDescriptor pD =
            static_cast<CRingBlockReader::pDataDescriptor>(malloc(sizeof(d)));
        if (!pD) {
            throw std::bad_alloc();
        }
        memcpy(pD, &d, sizeof(d));
        result.first = sizeof(d);
        result.second = pD;
    }
    
    return result;
}
