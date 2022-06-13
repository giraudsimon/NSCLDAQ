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
 * @file CDropSourcesCommand.h
 * @brief Define the class that implements the dropSources command.
 */

#ifndef CDROPSOURCESCOMMAND_H
#define CDROPSOURCESCOMMAND_H

#include <TCLObjectProcessor.h>



class CTCLInterpreter;
class CTCOObject;

/**
 * CDropSourcesCommand
 *
 *  Implements a command to drop all knowledge of the current set of sources.
 *  Normally this is registered as EVB::dropSources.
 */
class CDropSourceCommand : public CTCLObjectProcessor
{
  // Impelemented canonicals:

public:
  CDropSourceCommand(CTCLInterpreter& interp, std::string command);
  virtual ~CDropSourceCommand();
  
  // Unimplemented canonicals:

private:
  CDropSourceCommand(const CDropSourceCommand&);
  CDropSourceCommand& operator=(const CDropSourceCommand&);
  int operator==(const CDropSourceCommand&) const;
  int operator!=(const CDropSourceCommand&) const;

  // Public interfaces:

  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

};

#endif
