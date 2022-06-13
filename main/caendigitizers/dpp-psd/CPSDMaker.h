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
* @file     CPSDMaker.h
* @brief    Header for class that makes all DPP-PSD readout modules
*           given a config file.
* @author   Ron Fox
*
*/
#ifndef CPSDMAKER_H
#define CPSDMAKER_H
#include <string>

class CPsdCompoundEventSegment;
class CPsdTrigger;


/**
 * @class  CPSDMaker
 *    Given a compass configuation file, this class
 *    creates all of the stuff needed to readout all of the DPP-PSD
 *    modules described in the file.
 *
 *    Source ids are assigned from the module serial number in the
 *    configuration file.  That ensures a constant binding regardless
 *    of the ordering of modules in the config file.
 */
class CPSDMaker {
private:
  CPsdCompoundEventSegment*  m_pReadout;
  CPsdTrigger*               m_pTrigger;
  std::string                m_configFile;
public:
    CPSDMaker(const char* filename);
    void generate();
    CPsdCompoundEventSegment* getEventSegment();
    CPsdTrigger*      getTrigger();
    
};


#endif