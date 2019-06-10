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

#ifndef __COUTPUTSTATSCOMMAND_H
#define __COUTPUTSTATSCOMMAND_H

#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif


// Forward definitions:

class CTCLInterpreter;
class CTCLObject;
class COutputStatsObserver;

/**
 * @class COutputStatsCommand
 *
 *  Provides the outputstats command.  This is a command ensemble
 *  with subcommands:
 *   - get - retrieves the current statistics.
 *   - clear - clears the current statistics.
 *
 *  The class operates by registering an output statistics observer on
 *  the fragment handler singleton and manipulating it.
 *
 */
class COutputStatsCommand : public CTCLObjectProcessor
{
  // local member data:

private:
  COutputStatsObserver* m_pObserver;

  // Canonical methods:
public:
  COutputStatsCommand(CTCLInterpreter& intrerp, std::string command);
  virtual ~COutputStatsCommand();

  // illegal canonicals:

private:
  COutputStatsCommand(const COutputStatsCommand&);
  COutputStatsCommand& operator=(const COutputStatsCommand&) const;
  int operator==(const COutputStatsCommand&) const;
  int operator!=(const COutputStatsCommand&) const;;

  // Public entries (interface):

public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  // command execution methods:

private:
  int clear(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
  int get(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

};

#endif
