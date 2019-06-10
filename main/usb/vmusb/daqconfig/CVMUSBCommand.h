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
 * @file CVMUSBCommand.h
 * @brief defines the command to instantiate/manipulate
 *        CVMUSBControl objects.
 * @author Ron Fox (fox@nscl.msu.edu).
 */

#ifndef __CVMUSBCOMMAND_H
#define __CVMUSBCOMMAND_H


#ifndef __TCLOBJECTPROCESSOR_H
#include <TCLObjectProcessor.h>
#endif

#ifndef __STL_VECTOR
#include <vector>
#ifndef __STL_VECTOR
#define __STL_VECTOR
#endif
#endif

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


class CTCLInterpreter;
class CTCLObject;
class CConfiguration;

/**
 * This class creates configures and queries objects of
 * the CVMUSBControl class.  That class allows you to 
 * configure the VMUSB LEDs, gate and delay generators,
 * Nim outputs and inputs as well as to read the
 * internal scalers either incrementally or cummulatively.
 *
 */
class CVMUSBCommand : public CTCLObjectProcessor
{
private:
  CConfiguration&  m_Config;

public:
public:
  CVMUSBCommand(CTCLInterpreter& interp,
	      CConfiguration&  config,
	      std::string      commandName = std::string("vmusb"));
  virtual ~CVMUSBCommand();
private:
  CVMUSBCommand(const CVMUSBCommand& rhs);
  CVMUSBCommand& operator=(const CVMUSBCommand& rhs);
  int operator==(const CVMUSBCommand& rhs) const;
  int operator!=(const CVMUSBCommand& rhs) const;
public:

  // command entry point:
protected:
  virtual int operator()(CTCLInterpreter& interp,
			 std::vector<CTCLObject>& objv);


private:
  virtual int create(CTCLInterpreter& interp, 
		     std::vector<CTCLObject>& objv);
  virtual int config(CTCLInterpreter& interp,
		     std::vector<CTCLObject>& objv);
  virtual int cget(CTCLInterpreter& interp,
		   std::vector<CTCLObject>& objv);


};


#endif
