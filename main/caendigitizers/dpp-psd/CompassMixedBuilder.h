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
* @file     CompassMixedBuilder.h
* @brief    Class definition for building mixed DPP-PSD/DPP-PHA from xml
* @author   Ron Fox
*
*/
#ifndef COMPASSMIXEDBUILDER_H
#define COMPASSMIXEDBUILDER_H
class CPSDMaker;
class CompassMaker;
class COneOnlyEventSegment;
class CCompoundTrigger;

/**
 * @class CompassMixedBuilder
 *    The CompassMaker and CPSDMaker create triggers and readout
 *    objects from the appropriate modules in the COMPASS configuration
 *    file.
 *
 *    This class pulls it all together creating a COneOnlyEventSegment
 *    and CCompoundTrigger that consists of the stuff created from
 *    those two classes resulting in a readout program that
 *    can handle all of the 730's/725's in a COMPASS configuration
 *    file.
 */
class CompassMixedBuilder {
private:
    CPSDMaker*     m_pPsd;
    CCompassMaker* m_pCompass;
public:
    CompassMixedBuilder(const char* filename);
    void generate();
    COneOnlyEventSegment* getEventSegment();
    CCompoundTrigger*     getTrigger();
    
};

#endif