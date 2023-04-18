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

/** @file:  SettingsWriter.h
 *  @brief: Abstract base class to write DSP settings to file.
 */
 
#ifndef SETTINGSWRITER_H
#define SETTINGSWRITER_H
#include <ModuleSettings.h>

#include <vector>

namespace DDAS {
 

 /**
  * @class SettingsWriter
  *    Abstract base class for writing settings data to something.
  *    The idea is that this can be subclassed to write this data in
  *    any of a number of ways.
  *    Typically the concrete class's constructor will connect the
  *    object to some sink (e.g. file or database or maybe even crate)
  *    and the write method would be used to actually output the settings
  *    to that sink.
  */
class SettingsWriter
{
public:
    virtual ~SettingsWriter() {}
    virtual void write(const ModuleSettings& dspSettings) = 0;
};
 
}                                  // namespace DDAS
 #endif