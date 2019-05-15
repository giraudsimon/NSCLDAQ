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
* @file  CCommnicatorFactoryCreator.h
* @brief Extensible factory to create communicator factories.
*/
#ifndef CCOMMUNICATORFACTORYMAKER_H
#define CCOMMUNICATORFACTORYMAKER_H
#include "CCommunicatorFactory.h"
#include <CExtensibleFactory.h>

typedef CCreator<CCommunicatorFactory> CommunicatorFactoryCreator;
typedef CExtensibleFactory<CCommunicatorFactory> CommunicatorFactoryFactory;

class CCommunicatorFactoryMaker : public CommunicatorFactoryFactory
{
private:
    static CCommunicatorFactoryMaker* m_pInstance;
private:
    CCommunicatorFactoryMaker();
    ~CCommunicatorFactoryMaker() {}
public:
    static CCommunicatorFactoryMaker* getInstance();
};
#endif