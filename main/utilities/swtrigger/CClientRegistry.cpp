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

/** @file:  CClientRegistry.cpp
 *  @brief: Implement the client registry.
 */

#include "CClientRegistry.h"
#include <stdexcept>

/**
 * add
 *   @param newId - add a new client to the registry.
 *   @throw std::logic_error    if the Id  is already registered.
 */
void
CClientRegistry::add(uint64_t newId)
{
    if (hasClient(newId)) {
        throw std::logic_error("CCLientRegistry::add - duplicate id");
    }
    m_clients.insert(newId);
}
/**
 * remove
 *    @param existingId - id to remove from the registry.
 *    @throw std::logic_error  - if the id is not in the registry.
 */
void
CClientRegistry::remove(uint64_t existingId)
{
    if(!hasClient(existingId)) {
        throw std::logic_error("CClientRegistry::remove - no such id");
    }
    m_clients.erase(existingId);
}
/**
 *  hasClient
 *    @param id - the id to look for.
 *    @return bool - true if the id is in the registry, false otherwise.
 */
bool
CClientRegistry::hasClient(uint64_t id)
{
    return (m_clients.count(id) != 0);
}
/**
 * empty
 *    @bool - true if there are no more clients.
 */
bool
CClientRegistry::empty()
{
    return m_clients.empty();
}
