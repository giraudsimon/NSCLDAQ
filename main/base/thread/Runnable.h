#ifndef DAQHWYAPI_RUNNABLE_H
#define DAQHWYAPI_RUNNABLE_H

/*=========================================================================*\
| Copyright (C) 2005 by the Board of Trustees of Michigan State University. |
| You may use this software under the terms of the GNU public license       |
| (GPL).  The terms of this license are described at:                       |
| http://www.gnu.org/licenses/gpl.txt                                       |
|                                                                           |
| Written by: E. Kasten                                                     |
\*=========================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>


namespace daqhwyapi {

/*=====================================================================*/
/**
* @class Runnable
* @brief Basic runnable interface.
*
* Basic runnable interface.
*
* @author  Eric Kasten
* @version 1.0.0
*/
class Runnable  {
  public:
    virtual void start() = 0;
    virtual void run() = 0;
};

} // namespace daqhwyapi

#endif
