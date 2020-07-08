/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Author:
             Ron Fox
             Jeromy Tompkins
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/
#ifndef CMODULECREATORT_H
#define CMODULECREATORT_H

#include <memory>
#include <CExtensibleFactory.h>

/**
 * @file CModuleCreatorT.h
 * @brief Defines the ABC for the module creator.
 * @author Ron Fox <fox@nscl.msu.edu>
 */


template<class Ctlr> class CControlHardwareT;

template<class Ctlr>
using CModuleCreatorT = CCreator<CControlHardwareT<Ctlr>>;


#endif
