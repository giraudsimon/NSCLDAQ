/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

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
 * @file CPH7106Creator.h
 * @brief creational class for CPH7106 discriminator objects.
/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2013.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef  _CPH7106CREATOR_H
#define  _CPH7106CREATOR_H

#ifndef _CMODULECREATOR_H
#include "CModuleCreator.h"
#endif

#include <memory>
#include <CControlHardware.h>

/**
 * @file CPH7106Creator.h
 * @brief creational class for CPH7106 discriminator objects.
 * @author Ron Fox <fox@nscl.msu.edu>
 */


/**
 * @class CPH7106Creator
 *
 *   Extensible factories rely on registered creators to match an object type
 *   and generate an instance of that class.  This class is such a creational
 *   for the CModuleFactory which generates a CPH7106 object to control a 
 *   Phillips 7106 CAMAC discriminator.
 */
class CPH7106Creator : public CModuleCreator
{
public:
  virtual CControlHardware* operator()();
};


#endif
