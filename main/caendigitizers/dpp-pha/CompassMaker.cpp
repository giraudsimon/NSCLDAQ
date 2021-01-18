/*
*-------------------------------------------------------------
 
 CAEN SpA 
 Via Vetraia, 11 - 55049 - Viareggio ITALY
 +390594388398 - www.caen.it

------------------------------------------------------------

**************************************************************************
* @note TERMS OF USE:
* This program is free software; you can redistribute it and/or modify it under
* the terms of the GNU General Public License as published by the Free Software
* Foundation. This program is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. The user relies on the 
* software, documentation and results solely at his own risk.
*
* @file     CompassMaker.cpp
* @brief    Implementation of the CompassMaker class
* @author   Ron Fox
*
*/
#include "CompassMaker.h"
#include "CompassEventSegment.h"
#include "CompassMultiModuleEventSegment.h"
#include "CompassTrigger.h"

#include <stdexcept>
#include <sstream>

static const int PHA_FIRMWARE_ID=139;

/**
 * Constructor.
 *   @param filename - name of the configuration file.
 */
CompassMaker::CompassMaker(const char* filename) :
    m_pModules(nullptr),
    m_pTrigger(nullptr),
    m_configFile(filename)
{}
/**
 * generate
 *    Generates the modules from the configuration file.
 */
void
CompassMaker::generate()
{
    if (m_pModules || m_pTrigger) {
        throw std::logic_error("CompassMaker::generate -- already performed");
    }
    pugi::xml_document doc;
    auto status = doc.load_file(m_configFile.c_str());
    
    if (status.status != pugi::status_ok) {
        std::stringstream msg;
        msg << "Error parsing " << m_configFile << " at: "
            << status.offset << " : " << status.description();
        throw std::invalid_argument(msg.str());
    }
    
    // Get the document root node .. must be present.
    
    auto config =
        getNodeByNameOrThrow(doc, "configuration", "Not a compass config file");
    
    auto boards = getAllByName(config, "board");
    
    // Ok now we have at least an empty trigger and event segment:
    
    m_pModules = new CompassMultiModuleEventSegment;
    m_pTrigger = new CompassTrigger;
    
    
    for (int i =0; i < boards.size(); i++) {
        auto info = getDigitizerInfo(boards[i]);
        if (info.s_fwMajor = PHA_FIRMWARE_ID) {
            auto pBoard = new CompassEventSegment(
                m_configFile.c_str(), info.s_serial, info.s_linkType,
                info.s_linkNum, info.s_node, info.s_base
            );
            m_pModules->addModule(pBoard);
            m_pTrigger->addModule(pBoard);
        }
    }
}
/**
 * getEventSegment
 *    Returna a pointer to the event segment we made:
 * @return CompassMultiModuleEventSegment
 */
CompassMultiModuleEventSegment*
CompassMaker::getEventSegment()
{
    if (!m_pModules) {
        throw std::logic_error(
            "CompassMaker::getEventSegment - must call generate first"
        );
    }
    return m_pModules;
}

/**
 *  getTrigger
 * @return CompassTrigger* - Pointer to our internal trigger module.
 */
CompassTrigger*
CompassMaker::getTrigger()
{
    if(!m_pTrigger) {
        throw std::logic_error(
            "CompassMaker::getTrigger - must call generate first"
        );
    }
    return m_pTrigger;
}

//////////////////////////////////////////////////////////
// Private utilities.

/**
 * getDigitizerInfo
 *    Returns information about a digitizer given it's <board> node.
 * @param board - the XML node for the <board> tag.
 * @return DigitizerInfo
 */
CompassMaker::DigitizerInfo
CompassMaker::getDigitizerInfo(pugi::xml_node& board)
{
    // get the nodes we need from the <board>'s immediate children:
    
    auto serial =
        getNodeByNameOrThrow(board, "serialNumber", "Missing <serialNumber tag");
    auto link   =
        getNodeByNameOrThrow(board, "linkNum", "Missing <linkNum> tag");
    auto ctype  =
        getNodeByNameOrThrow(board, "connectionType", "Missing <connectionType> tag");
    auto node   =
        getNodeByNameOrThrow(board, "conetNode", "Missing <conetNode> tag");
    auto base =
        getNodeByNameOrThrow(board, "address", "Missing <address> tag");
    auto amcfw =
        getNodeByNameOrThrow(board, "macFirmware", "Missing <amcFirware> tag");
    auto major =
        getNodeByNameOrThrow(board, "major", "AMCFirmware missing <major> tag");
    
    // Fill in the function result:
    
    DigitizerInfo result;
    
    result.s_serial = getUnsignedContents(serial);
    result.s_linkNum   = getUnsignedContents(link);
    result.s_node   = getUnsignedContents(node);
    result.s_base  = getUnsignedContents(base);
    result.s_fwMajor = getUnsignedContents(major);
    result.s_linkType = getLinkType(ctype);
    
    return result;
}

/**
 * getLinkType
 *   Get the contents of the <connectionType> node and
 *   turn it into a link type identifier.  The values
 *   I can expect are "USB" and "OPTICAL"
 *   which map respectively to CAEN_DGTZ_USB and
 *   CAEN_DGTZ_Optical respectively.
 *
 * @param linkType - reference to the <connectionType> tag node.
 * @return CAEN_DGTZ_ConnectionType
 */
CAEN_DGTZ_ConnectionType
CompassMaker::getLinkType(pugi::xml_node& linkType)
{
    std::string sLinkType = getStringContents(linkType);
    if (sLinkType == "USB") {
        return CAEN_DGTZ_USB;
    } else if (sLinkType == "OPTICAL") {
        return CAEN_DGTZ_OpticalLink;
    } else {
        std::string msg("Unexpected link type: " );
        msg += sLinkType;
        throw std::invalid_argument(msg);
    }
}
