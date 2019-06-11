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



#ifndef __CSTRINGS_H
#define __CSTRINGS_H

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif


/*!
  This class provides static member functions that implement string utilities.
*/

class CStrings
{
public:
  static std::string EscapeString(const char* in, 
			     const char* charset, 
				  const char* how) ;
};

#endif
