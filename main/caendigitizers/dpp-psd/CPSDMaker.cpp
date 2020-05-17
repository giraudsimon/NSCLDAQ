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
* @file     CPSDMaker.cpp
* @brief    Implement the PSD maker.
* @author   Ron Fox
*
*/
#include "CPSDMaker.h"
#include "CPsdCompoundEventSegment.h"
#include "CPsdTrigger.h"
#include "PSDParameters.h"
#include "CDPpPsdEventSegment.h"
#include <stdexcept>

/**
 * constructor
 *    @param filename - pointer to the Compass config  file to process.
 */
CPSDMaker::CPSDMaker(const char* filename) :
    m_configFile(filename),
    m_pReadout(nullptr),
    m_pTrigger(nullptr)
{}

/**
 * generate
 *    Createa the compound event segment and stock it with
 *    all of the DPP-psd event segments listed in the
 *    configuration file.  Also creates the trigger object.
 *    These are all stored for later retrieval.
 */
void
CPSDMaker::generate()
{
    PSDParameters params;
    params.parseConfigurationFile(m_configFile.c_str());
    m_pReadout = new CPsdCompoundEventSegment;
    m_pTrigger = new CPsdTrigger;
    
    for (int i =0; i < params.s_boardParams.size(); i++) {
        PSDBoardParameters& board(params.s_boardParams[i]);
        
        // Pull the connection parameters out of the config
        // and generate an board event segment -- shove that
        // into the compound event segment and trigger:
        
        auto linkType = board.s_linkType;
        int  linkNum  = board.s_linkNum;
        int  node     = board.s_node;
        int  base     = board.s_base;
        int  serial   = board.s_serialNumber;
        
        auto module = new CDPpPsdEventSegment(
                linkType, linkNum, node, base, serial,
                m_configFile.c_str()
        );
        m_pReadout->addModule(module);
        m_pTrigger->addModule(module);
    }
}
/**
 * getEventSegment
 *   Must be called after generate; Returns the compound event
 *   segment created by generate.
 *
 *  @return CDPpPsdCompoundEventSegment*
 */
CPsdCompoundEventSegment*
CPSDMaker::getEventSegment()
{
    if (m_pReadout) return m_pReadout;
    throw std::logic_error(
        "CPSDMaker::getEventSegment called prior to CPSDMaker::generate"
    );
}
/**
 * getTrigger
 *   @return CDPpPsdTrigger* - pointer to generated trigger.
 */
CPsdTrigger*
CPSDMaker::getTrigger()
{
    if (m_pTrigger) return m_pTrigger;
    throw std::logic_error(
        "CPSDMaker::getTrigger called before CPSdMaker::generate"
    );
}
