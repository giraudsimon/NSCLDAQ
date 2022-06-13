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
 * @file CConfigure.h
 * @brief Define class for evb configuration management.
 */

#ifndef CCONFIGURE_H
#define CCONFIGURE_H

#include <TCLObjectProcessor.h>


// Forward class/type definitions:

class CTCLInterpreter;
class CTCLObject;

/**
 * Cconfigure
 *
 *  This class implements a command ensemble that manages the event orderer
 *  configurable parameters.
 *  
 *  To set a configuration parameter one does:
 * \code {.tcl}
 *   EVB::config set name value
 * \endcode
 *

 * To retrieve a configuration parameter:

 * \code {.tcl}
 * EVB::Configure get name
 * \endcode
 *
 *  *  Where
 *    - name is the name of a configuration parameter.
 *    - value is the proposed value for the parameter.
 */
class CConfigure : public CTCLObjectProcessor
{

  // Canonicals:
public:

  CConfigure(CTCLInterpreter& interp, std::string commandName);
  virtual ~CConfigure();

  // Disallowed canonicals:

  CConfigure(const CConfigure&);
  CConfigure& operator=(const CConfigure&);
  int operator==(const CConfigure&) const;
  int operator!=(const CConfigure&) const;

 // Public entries:

public:
  int operator()(CTCLInterpreter& interp, std::vector<CTCLObject>& objv);

  // Utilities:

private:
  int Set(CTCLInterpreter& interp, std::string name, CTCLObject& value);
  int Get(CTCLInterpreter& interp, std::string name);

};

#endif
