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

/** @file:  CClientRegistry.h
 *  @brief: Provide a registry of clients for transports that need it.
 */
#ifndef CCLIENTREGISTRY_H
#define CCLIENTREGISTRY_H
#include <stdint.h>
#include <stddef.h>
#include <set>
/**
 * @class CClientRegistry
 *    Some (particularly fan out) transports require that clients register
 *    their presence so that appropriate end of data messages can be sent to
 *    all of them when there's no more data to process.  This class provides
 *    a registry of uint64_t client ids.
 *
 *    1. No policy is prescribed by this class for the assignment of client ids.
 *       Id's only have to be unique across the registry.
 *    2. If I were ambitious I could make this a template class allowing
 *       the client id to be any data type with the ability to index a set.
 *       -- I'm not that ambitous.
 */
class CClientRegistry {
private:
    std::set<uint64_t>   m_clients;
public:
    void add(uint64_t newId);
    void remove(uint64_t existingId);
    bool hasClient(uint64_t id);
    bool empty();
    size_t size() { return m_clients.size(); }
};

#endif
