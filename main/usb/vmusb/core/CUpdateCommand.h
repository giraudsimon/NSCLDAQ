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

#ifndef CUPDATECOMMAND_H
#define CUPDATECOMMAND_H

#include <TCLObjectProcessor.h>

#include <vector>
#include <string>


class CCtlConfiguration;
class CTCLObject;
class CTCLInterpreter;
class CVMUSB;

/*!
   CUpdateCommand implements the Update command this command
   informs a control object to update its values.
   The single form is:

   Update name

   Where:
   - name is the name of a control object
*/

class CUpdateCommand : public CTCLObjectProcessor
{
  CCtlConfiguration&   m_config;	// Tcl server that is running us.
  CVMUSB&      m_Vme;
public:
  CUpdateCommand(CTCLInterpreter&   interp,
          	     CCtlConfiguration& config,
	               CVMUSB&            vme);
  virtual ~CUpdateCommand();
private:
  CUpdateCommand(const CUpdateCommand& rhs);
  CUpdateCommand& operator=(const CUpdateCommand& rhs);
  int operator==(const CUpdateCommand& rhs) const;
  int operator!=(const CUpdateCommand& rhs) const;
public:

  // Command entry point:

protected:
  int operator()(CTCLInterpreter& interp,
		 std::vector<CTCLObject>& objv);


};



#endif
