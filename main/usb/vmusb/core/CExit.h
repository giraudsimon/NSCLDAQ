/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013.
#
#    You may use this software under the terms of the GNU public license
#    (GPL).  The terms of this license are described at:
#
#     http://www.gnu.org/licenses/gpl.txt
#
#    Author:
#            Ron Fox
#            NSCL
#            Michigan State University
#            East Lansing, MI 48824-1321

##
# @file   CExit.h
# @brief  Command to override the Tcl exit command for the main interp.
# @author <fox@nscl.msu.edu>
*/



#ifndef CEXIT_H
#define CEXIT_H

#include <TCLObjectProcessor.h>

#include <vector>
#include <string>

class CTCLInterpreter;
class CTCLInterpreterObject;

/**
 * This class provides an override for the Tcl exit
 * command.  The syntax of that command is
 * \verbatim
 *    exit ?status?
 * \endverbatim
 *
 * Just like the standard Tcl exit command.  What we do,
 * differently, however is ensure that the Tcl server
 * thread has exited prior to calling Tcl_Exit().
 * This is done to avoid the random SEGFAULT on exit that
 * sometimes happens.  For rationale see the second reply to
 * http://objectmix.com/tcl/388620-segfault-exit-activetcl-8-5-2-threaded-extension.html
 */
class CExit : public CTCLObjectProcessor
{
  // Canonicals:
public:
  CExit(CTCLInterpreter& interp);
  virtual ~CExit();
private:
  CExit(const CExit& rhs);
  CExit& operator=(const CExit& rhs);
  int operator==(const CExit& rhs) const;
  int operator!=(const CExit& rhs) const;
public:
  static void exit(int status);

  // Process the command:
protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);

};


#endif
