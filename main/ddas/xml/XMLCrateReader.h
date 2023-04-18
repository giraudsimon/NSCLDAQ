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

/** @file:  XMLCrateReader.h
 *  @brief: Define an XML Crate reader
 */
#ifndef XMLCRATEREADER_H
#define XMLCRATEREADER_H

#include "CrateReader.h"

#include <stdint.h>
#include <map>
#include <string>
#include <stdio.h>
#include <tinyxml2.h>
#include "tinyxmlutil.h"

namespace DDAS {

/**
 * @class XMLCrateReader
 *    This class specializes the CrateReader class to work with a
 *    set of XML specifications of the crate, modules and their settings.
 *    The top level is a crate xml file which is of the form
 * \verbatim
 *  <crate id="1">
 *      <slot number="2" ... configfile="/user/fox/configs/Ge.xml" />
 *      <slot number="5" ... configfile="/user/fox/configs/Ge-short-trace.xml />
 *  </crate>
 *
 * \endverbatim
 *    I've only shown the <slot> attributes we care/check for.  The other
 *    attributes at this time are:
 *
 *    -   evtlen - Number of longwords expected for a hit from this
 *        module.
 *    -   fifo_threshold - Minimum # of longs in the module fifo before
 *            declaring an trigger.
 *    -   infinity_clock - does not re-synchrnoize at the start of each
 *            run.
 *    -    timestamp_scale -  Scale factor to apply to the timestamp,
 *             whatever its source, to compute the event builder timestamp.
 *    -    external_clock - Indicates the timestamp source is the external
 *             clock.
 * What we care about in <slot tags>:
 *     -  number - the slot number.
 *     -  configfile - Where the module settings xml  file is.
 * We have extra methods to make all this other stuff available.
 */
class XMLCrateReader : public CrateReader {
public:
    
    // For each slot our document processor fills the following struct:
    
    struct SlotInformation {
        unsigned       s_slot;           // Required.
        std::string    s_configFile;     // required.
        unsigned       s_evtlen;         // Required
        unsigned       s_fifothreshold;  // Defaults to 10*EXTFIFO_READ_THRESH
        bool           s_infinityclock;  // Defaults to false.
        double         s_timestampscale; // Defaults to 1.0
        bool           s_externalclock;   // Defaults to false.
    };
private:    
    // Map from slots to SlotInformation:
    
    std::map<unsigned short, SlotInformation> m_slotInfo;
public:
    XMLCrateReader(const char* configFile);
    virtual ~XMLCrateReader();
    
    // Required to implement the interface:
    
    virtual SettingsReader* createReader(unsigned short slot);
    
    
    // Info getters the CrateReader doesn't need but others do:
    
    unsigned short getCrateId() const {return m_crateId;}
    unsigned getEvtLen(unsigned short slot);
    unsigned getFifoThreshold(unsigned short slot);
    bool     isInfinityClock(unsigned short slot);
    double   getTimestampScale(unsigned short slot);
    bool     isExternalClock(unsigned short slot);
    const std::map<unsigned short, SlotInformation>&
        getSlotInformation() const {return m_slotInfo;}
private:
    const SlotInformation& getSlotInfo(unsigned short slot);
    void                   processCrateFile(FILE* pFile);
    void                   processCrateInfo(tinyxml2::XMLElement& root);
    SlotInformation        processSlotInfo(tinyxml2::XMLElement& slot);
    void                   checkDuplicateSlot(const SlotInformation& slot);
   
};


}                                       // DDAS Namespace.
#endif
