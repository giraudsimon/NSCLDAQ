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

#ifndef __CMODULEFACTORYT_H
#define __CMODULEFACTORYT_H

/**
 * @file CModuleFactoryT.h
 * @brief Defines a factory for control modules in the Tcl server.
 * @author Ron Fox <fox@nscl.msu.edu>
 */

#ifndef __STL_STRING
#include <string>
#ifndef __STL_STRING
#define __STL_STRING
#endif
#endif

#ifndef __STL_MAP
#include <map>
#ifndef __STL_MAP
#define __STL_MAP
#endif
#endif

#include <memory>

template<class Ctlr> class CControlHardwareT;
template<class Ctlr> class CModuleCreatorT;

/**
 * @class CModuleFactory
 *    Singleton extensible factory.   The factory creates CControlModules.
 *  
 */
template<class Ctlr>
class CModuleFactoryT {

  private:

    static CModuleFactoryT* m_pInstance; //!< sole instance

    // the mapping of module names to creators
    std::map<std::string, 
             std::unique_ptr<CModuleCreatorT<Ctlr>> > m_Creators;

    /**! \brief Private constructor */
    CModuleFactoryT();

    /**! \brief Private desstructor */
    ~CModuleFactoryT();

  public:

    /**! \brief Mechanism to get the singleton pointer:
     *
     * If the instance does not yet exist, it is constructed.
     * 
     * \returns pointer to the sole instance
     */
    static CModuleFactoryT* instance();


  public:
    /**! \brief Insert a new creator type 
     *
     * This passes ownership of a new creator into the factory
     * for use.
     *
     *  \param type       name of type 
     *  \param pCreator   a creator instance 
     */ 
    void addCreator(std::string type, 
        std::unique_ptr<CModuleCreatorT<Ctlr>> pCreator);

    /**! \brief Factory method
     *
     * \param type  the type of control hardware desired
     *
     * \returns instance of hardware associated with type
     */
    CControlHardwareT<Ctlr>* create(std::string type);


};

#include <CModuleFactoryT.hpp>

#endif
