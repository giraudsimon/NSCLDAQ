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

#ifndef CVMUSBCOMMAND_H
#define CVMUSBCOMMAND_H


#include <TCLObjectProcessor.h>
#include <vector>
#include <string>

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
