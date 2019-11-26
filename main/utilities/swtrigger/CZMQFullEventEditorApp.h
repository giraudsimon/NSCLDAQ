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

/** @file:  CZMQFullEventEditorApp.h
 *  @brief: Header for ZMQ implementation of Full Event Editor data flow.
 */
#ifndef CZMQFULLEVENTEDITORAPP_H
#define CZMQFULLEVENTEDITORAPP_H
#include "CFullEventEditorApp.h"


#include "CBuiltRingItemExtender.h"
#include <vector>

class CFullEventEditor;;

class CRingItemZMQSourceElement;
class CThreadedProcessingElement;

class CTransport;
class CSender;
class CReceiver;
class CRingItemSorter;

class CDataSinkElement;

/**
 * @class CZMQFullEventEditorApp
 *    This is the application code that's responsible for establishing
 *    ZMQ based data flow for a full event editor.  Once that dataflow is
 *    setup, the application runs all threads in the appropriate start order
 *    and waits for them to exit.
 *
 *    The CZMQTransformerApp is essentially the same thing but with a different
 *    worker object so you can see CZMQTransformerApp.h for the program
 *    structure.
 */
class CZMQFullEventEditorApp : public CFullEventEditorApp
{
private:
// Data source objects.

    CRingItemZMQSourceElement* m_pSourceElement;
    CThreadedProcessingElement* m_pSourceThread;

    // Stuff needed to support the sorter.

    CTransport*                 m_pSortServer;
    CReceiver*                  m_pSortReceiver;
    CTransport*                 m_pSortSource;
    CSender*                    m_pSortSender;
    CRingItemSorter*            m_pSortElement;
    CThreadedProcessingElement* m_pSortThread;

    // Stuff for the ultimate data sink:

    CTransport*                 m_pSortClient;
    CReceiver*                  m_pSortData;
    CTransport*                 m_pRingSink;
    CSender*                    m_pRingSender;
    CDataSinkElement*           m_pSinkElement;
    CThreadedProcessingElement* m_pSinkThread;

    std::vector<CThreadedProcessingElement*> m_workers;
public:
    CZMQFullEventEditorApp(gengetopt_args_info& args);
    virtual ~CZMQFullEventEditorApp();
    virtual int operator()();
private:
    void startWorkers();
};

#endif