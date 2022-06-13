/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** 
 * @file COnLateDataCommand.h
 * @brief Class definition for COnLateDataCommand - invoke user script on data late.
 */

#ifndef CONLATEDATACOMMAND_H
#define CONLATEDATACOMMAND_H

#include <TCLObjectProcessor.h>	// base class

// Forward definitions.

class CTCLInterpreter;
class CTCLObject;
class CLateScriptObserver;


/**
 * @class COnLateDataCommand
 * @brief Tcl scripted traces on late fragments.
 *
 *  This class suplies the onDataLate command which allows you to extablish
 *  Tcl script handlers for data from data sources which arrives late with respect
 *  to the output time window.  The forms of the command are:
 *
 * \verbatim
 *  onDataLate newScript
 *  onDataLate
 *  onDataLate ""
 * \endverbatim
 *
 * # The first form supplies a new script for the data late handler.
 *   The return value is the previous script prefix.
 * # The second form only returns the current script prefix.
 * # The final form removes all script prefixes.
 *
 */
class COnLateDataCommand : public CTCLObjectProcessor
{
  // private data:

private:
  CLateScriptObserver* m_pDispatcher; // The observer actually dispatches the script.

  // Canonical methods:

public:
  COnLateDataCommand(CTCLInterpreter& interp, std::string command);
  virtual ~COnLateDataCommand();
 
  // Illegal canonical methods:

private:
  COnLateDataCommand(const COnLateDataCommand&);
  COnLateDataCommand& operator=(const COnLateDataCommand&);
  int operator==(const COnLateDataCommand&) const;
  int operator!=(const COnLateDataCommand&) const;

  // Virtual override (public):

public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>&objv);
};

#endif
