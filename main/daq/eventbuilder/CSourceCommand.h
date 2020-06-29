/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2005.

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
 * @file CSourceCommand.h
 * @brief provide command to add data source queues.
 */

#ifndef CSOURCECOMMAND_H
#define CSOURCECOMMAND_H

#include <TCLObjectProcessor.h>



// Forward definitions:

class CTCLInterpreter;
class CTCLObject;

/**
 * @class CSourceCommand
 *
 *  The EVB::source command allows TCL Scripts to crete event source queues
 *  in the fragment handler singleton.
 *
 * Command syntax:
 *
 * \verbatim
 *   EVB::source ?socket? ?id1? ?id2? ?id3?
 * \endverbatim
 *
 *  Without any socket/source ids, the list of known source ids is provided.
 */

class CSourceCommand : public CTCLObjectProcessor
{
  // Supported canonicals:

public:
  CSourceCommand(CTCLInterpreter& interp, std::string command);
  virtual ~CSourceCommand();

  // Illegal canonicals:

private:
  CSourceCommand(const CSourceCommand&);
  CSourceCommand& operator=(const CSourceCommand&);
  int operator==(CSourceCommand&) const;
  int operator!=(CSourceCommand&) const;

  // Public interface.
public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  // Utilities:
private:
  int listSources(CTCLInterpreter& interp);
  int addSources(std::vector<CTCLObject>& objv);
};

#endif
