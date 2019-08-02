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

/** @file:  CTestExtender.cpp
 *  @brief: Example of a ring item extender.
 */

// The code in this file can be turned into a shared object that can be
// dynamically loaded into Transformer.

#include <CBuiltRingItemExtender.h>
#include <stdint.h>

/**
 * @class CTestExtender
 *    This is a class that tests the built ring item extender class.
 *    It produces a three word extension that looks like:
 *    +---------+
 *    | a5a5    |
 *    +---------+
 *    | sum     |  Modulo 65535 sum of the ring item uint16's. without this extension.
 *    +---------+
 *    | 5a5a    |
 *    +---------+
 *
 *  @note  It's tempting to have static storage for the extension,
 *         that will not work as there needs to be a distinct extension for each
 *         of the fragments in the built event.  Therefore we're going
 *         to new each extension into existence and use our free method to
 *         kill them off.
 */
class CTestExtender : public CBuiltRingItemExtender::CRingItemExtender
{
private:
    struct Extension {
        uint16_t  s_header;
        uint16_t  s_checksum;
        uint16_t s_trailer;
        
        Extension() {
            s_header = 0xa5a5;
            s_checksum = 0;
            s_trailer = 0x5a5a;
        }
    };
    
public:
    iovec operator()(pRingItem item);
    void free(iovec& e);
};

/**
 * operator()
 *    Create the extension, the iovec and compute the checksum field:
 *
 *  @param item - pointer to  a ring item.
 */
iovec
CTestExtender::operator()(pRingItem item)
{
    iovec result;
    result.iov_len = sizeof(Extension);
    Extension* p   = new Extension;
    result.iov_base= p;
    
    // Compute the checksum:
    
    uint16_t* pItem = reinterpret_cast<uint16_t*>(item);
    for (int i =0; i < item->s_header.s_size/sizeof(uint16_t); i++) {
        p->s_checksum+= *pItem++;
    }
    
    return result;
}

/**
 * free
 *    Given one of our Iovecs - frees the extension associated with it.
 * @param extension - reference to the iovec describing our extension.
 */
void
CTestExtender::free(iovec& extension)
{
    Extension* pExt = static_cast<Extension*>(extension.iov_base);
    delete pExt;
}


/**
 *  This next bit provides the transformation framework a mechanism
 *  to create our extender.  It must have a C binding so that it can be
 *  found without compiler specific name mangling.  The name of the
 *  function must be exactly "createExtender".
 */
extern "C" {                       // Force C (un-name-mangled) bindings.
    CTestExtender* createExtender() {
        return new CTestExtender;
    }
    
}