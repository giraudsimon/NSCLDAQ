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
# @file   CMarkerCreator.h
# @brief  Creational class for CMarkerObject objects.
# @author <fox@nscl.msu.edu>
*/

#ifndef _CMARKERCREATOR_H
#define _CMARKERCREATOR_H

#include "CModuleCreator.h"
#include "CControlHardware.h"

/**
 * @class CMarkerCreator
 *     Creates CMarkerObject items.
 */
class CMarkerCreator : public CModuleCreator
{
public:
  virtual CControlHardware* operator()();
};


#endif
