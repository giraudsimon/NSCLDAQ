/*
    This software is Copyright by the Board of Trustees of Michigan
    State University (c) Copyright 2017.

    You may use this software under the terms of the GNU public license
    (GPL).  The terms of this license are described at:

     http://www.gnu.org/licenses/gpl.txt

     Authors:
             Ron Fox
             Giordano Cerriza
	     NSCL
	     Michigan State University
	     East Lansing, MI 48824-1321
*/

/** @file:  CDataSinkElement.cpp
 *  @brief: Implement the data sink element.
 *  
 */
#include "CDataSinkElement.h"
#include "CSender.h"
#include "CReceiver.h"

#include <stdlib.h>

/**
 * constructor
 *    @param src - the data source
 *    @param sink - the data sink.
 */
CDataSinkElement::CDataSinkElement(CReceiver& src, CSender& sink) :
    m_pSource(&src), m_pSink(&sink)
{}

/**
 * destructor
 *    The data transfer objects are assumed to have been
 *    dynamically allocated.
 */
CDataSinkElement::~CDataSinkElement()
{
    delete m_pSource;
    delete m_pSink;
}

/**
 * operator()
 */
void
CDataSinkElement::operator()()
{
    void* pData;
    size_t nBytes;
    do {
        m_pSource->getMessage(&pData, nBytes);
        process(pData, nBytes);
        free(pData);
    } while(nBytes > 0);
    m_pSink->end();
}
/**
 * process
 *    Send data to the sink.
 *
 *  @param pData - pointer to the data.
 *  @param nBytes - number of bytes of data.
 */
void
CDataSinkElement::process(void* pData, size_t nBytes)
{
    m_pSink->sendMessage(pData, nBytes);
}