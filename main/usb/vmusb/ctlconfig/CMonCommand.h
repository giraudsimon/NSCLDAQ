/*
     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

#ifndef __CMONCOMMAND_H
#define __CMONCOMMAND_H

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


class CCtlConfiguration;
class CTCLObject;
class CTCLInterpreter;
/**
  Implements the mon command.  'mon' is used to
  retrieve any periodically monitored data from
  device driver instance.  mon operates without
  synchronizing with the data acquisition as the
  driver instances are assumed to have contributed to
  a periodic monitor list which is dispatched periodically.
  Device driver instances are presumed to be caching
  the results of these lists and returning the values
  of those cached values rather than interacting with the
  VME bus.   See as an example the CV6553 module which
  supports the CAEN V6553 high voltage bias supply.
*/
class CMonCommand : public CTCLObjectProcessor
{
private:
  CCtlConfiguration&   m_configuration;	// Tcl server that is running us.

public:
  CMonCommand(CTCLInterpreter&   interp,
	            CCtlConfiguration& configuration);
  virtual ~CMonCommand();
private:
  CMonCommand(const CMonCommand& rhs);
  CMonCommand& operator=(const CMonCommand& rhs);
  int operator==(const CMonCommand& rhs) const;
  int operator!=(const CMonCommand& rhs) const;
public:

  // Command entry point:

protected:
  int operator()(CTCLInterpreter& interp,
		 std::vector<CTCLObject>& objv);


};

#endif
