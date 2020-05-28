/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2009.

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
* @file  CCommunicatorFactoryMaker.cpp
* @brief Implementation of CCommunicatorFactoryMaker class.
*/

#include "CCommunicatorFactoryMaker.h"
#include "CZMQCommunicatorFactory.h"
#include <string>

CCommunicatorFactoryMaker* CCommunicatorFactoryMaker::m_pInstance(0);

class CCommunicatorFactoryCreator : public CommunicatorFactoryCreator
{
private:
    CCommunicatorFactory*  m_pFactory;
    std::string            m_description;
public:
    CCommunicatorFactoryCreator(const char* d, CCommunicatorFactory* f) :
        m_pFactory(f), m_description(d) {}
    virtual CCommunicatorFactory* operator()(void* unused) { return m_pFactory; }
    virtual std::string describe() const { return m_description; }
};

/**
 * Constructor (private)
 *   Create the instance and stock it with the pre-defined factory makers.
 */
CCommunicatorFactoryMaker::CCommunicatorFactoryMaker()
{

    addCreator(
        "ZeroMQ Communcation system",  new CCommunicatorFactoryCreator(
            "ZeroMQ Communcation system", new CZMQCommunicatorFactory()
        )
    );
}
/**
 * getInstance
 *    @return CCommunicatorFactoryMaker* - poointer to the singleton instance.
 *    @note   Not threadsafe -need to protect this with a critical section.
 */
CCommunicatorFactoryMaker*
CCommunicatorFactoryMaker::getInstance()
{
    if (!m_pInstance) {
        m_pInstance = new CCommunicatorFactoryMaker;
    }
    return m_pInstance;
}