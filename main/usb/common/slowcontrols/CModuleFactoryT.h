/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2014

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

#ifndef CMODULEFACTORYT_H
#define CMODULEFACTORYT_H

/**
 * @file CModuleFactoryT.h
 * @brief Defines a factory for control modules in the Tcl server.
 * @author Ron Fox <fox@nscl.msu.edu>
 */
#include <string>
#include <map>
#include <memory>
#include <CExtensibleFactory.h>
#include "CModuleCreatorT.h"


template<class Ctlr> class CControlHardwareT;

/**
 * @class CModuleFactory
 *    Singleton extensible factory.   The factory creates CControlModules.
 *  
 */


template<class Ctlr>
class CModuleFactoryT : public CExtensibleFactory<CControlHardwareT<Ctlr>> {

private:

    static CModuleFactoryT* m_pInstance; //!< sole instance
private:
  CModuleFactoryT();
  virtual ~CModuleFactoryT();
public:
  static CModuleFactoryT<Ctlr>* instance();

};

#include <CModuleFactoryT.hpp>

#endif
