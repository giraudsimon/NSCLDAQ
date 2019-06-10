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
 * @file CReviveSocketCommand.h
 * @brief Defines the class that implements the reviveSocket command
 */
#ifndef __CREVIVESOCKETCOMMAND_H
#define __CREVIVESOCKETCOMMAND_H

#include <TCLObjectProcessor.h>

// forware definitions

class CTCLInterpreter;
class CTCLObject;


/**
 * @class CReviveSocketCommand
 *
 *  The reviveSocket command interfaces with the fragment handler singleton to 
 *  indicate that the sources associated with a socket that was believed to be stalled or
 *  dead are actually now alive.  This really should probably never happen'
 *  but is possible so we must consider it
 *  The format of the command is:
 *
 * \verbatim
 *   reviveSocket socketname
 *
 * \endverbatim
 *  
 * socketname - The Tcl name of the socket.
 */
class CReviveSocketCommand : public CTCLObjectProcessor
{
  // Implemented canonicals:

 public:
  CReviveSocketCommand(CTCLInterpreter& interp, std::string name);
  virtual ~CReviveSocketCommand();

  // canonicals that are forbidden:

private:
  CReviveSocketCommand(const  CReviveSocketCommand&);
  CReviveSocketCommand& operator=(const  CReviveSocketCommand&);
  int operator==(const  CReviveSocketCommand&) const;
  int operator!=(const  CReviveSocketCommand&) const;

  // public methods (command implementation):

public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);
};
#endif
