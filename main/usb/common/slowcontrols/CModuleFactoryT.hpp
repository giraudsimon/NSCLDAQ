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

/**
 * addCreator
 *
 *  Adds a new creator to the factory.
 *
 * @param type - The type of the creator.
 * @param pCreator - Pointer to a creator (best if dynamically allocated).
 *
 * @note duplicate type - last one wins for now. - TODO - detect and object
 */
template <class Ctlr>
void
CModuleFactoryT<Ctlr>::addCreator(std::string type, 
                                  std::unique_ptr<CModuleCreatorT<Ctlr>> pCreator)
{
  m_Creators[type] = std::move(pCreator);
}
/**
 * create
 *
 *   Asks a module creator to create a control module.
 *
 * @param type - The type of control module to create.
 * @param name - Name of the module
 *
 * @return CControlHardware* - Pointer to the created module.
 * @retval NULL - no such type.
 */
template <class Ctlr>
CControlHardwareT<Ctlr>*
CModuleFactoryT<Ctlr>::create(std::string type)
{
  auto p = m_Creators.find(type);
  if (p != m_Creators.end()) {
    auto& creator = p->second;
    return creator->operator()();
  } else {
    return nullptr;
  }
}
