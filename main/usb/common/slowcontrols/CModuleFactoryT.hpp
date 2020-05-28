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
 * @file CModuleFactoryT.cpp
 * @brief Implementation of the control module factory.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

#include <CModuleCreatorT.h>




/**
 *  The instance pointer (static)
 */
template <class Ctlr>
CModuleFactoryT<Ctlr>* CModuleFactoryT<Ctlr>::m_pInstance(0);

/**
 * Constructor is null for now but must be defined to
 * make it private.
 */
template <class Ctlr>
CModuleFactoryT<Ctlr>::CModuleFactoryT() {}

/**
 *  similarly for the destructor
 */
template <class Ctlr>
CModuleFactoryT<Ctlr>::~CModuleFactoryT() {}


/**
 * instance (static)
 *    @return CModuleFactoryT*  The pointer to the singleton instance.
 */
template <class Ctlr>
CModuleFactoryT<Ctlr>*
CModuleFactoryT<Ctlr>::instance()
{
  if (!m_pInstance) {
    m_pInstance = new CModuleFactoryT<Ctlr>;
  }
  return m_pInstance;
}
