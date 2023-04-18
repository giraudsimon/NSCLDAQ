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

/** @file:  XMLSettingsReader.h
 *  @brief: Class that reads settings from an XML file
 */
#ifndef XMLSETTINGSREADER_H
# define XMLSETTINGSREADER_H
#include "SettingsReader.h"
#include "ModuleSettings.h"

#include <vector>
#include <tinyxml2.h>


namespace DDAS {
 /**
 * @class XMLSettingsReader
 *    This class is a settings reader that
 *    can read XML Setings files.  Two types of settings files can be
 *    read:
 *      -  Individual module settings files - these contain settings
 *         for a specific module.  They have a <Module>/</Module> structure.
 *      -  Create settings files - These contain setings for a several modules,
 *         typically a single crate worth of modules.  These have a
 *         <Crate></Crate> structure.
 *
 *    For the structure of module files see XMLSettingsWriter.h and
 *    its description of the XMLSettingsWriter class.  The structure of
 *    Crate definition files is pretty simple:
 *
 *      <Crate>
 *         <Module id="n' fileref="path-to-module-file" />
 *         ...
 *      </Crate>
 *
 *    That is for each module, in the crate, there's a module tag
 *    that specifies the module id (numbered from 0) and a reference
 *    to the XML module file that configures that module.
 */
 class XMLSettingsReader : public SettingsReader
 {
 private:
    std::string m_filename;
 public:
     XMLSettingsReader(const char* filename);
     virtual ~XMLSettingsReader();
     
     virtual DDAS::ModuleSettings get();
private:
    
    tinyxml2::XMLDocument* loadModuleFile(const char* filename);
    
    ModuleSettings         getModule(tinyxml2::XMLDocument& doc);
    void getModuleSettings(
        ModuleSettings& settings, tinyxml2::XMLElement& root);
    void getChannelSettings(
        ModuleSettings& settings, tinyxml2::XMLElement& channel
    );
 
 };
}                                                // namespace DDAS
#endif