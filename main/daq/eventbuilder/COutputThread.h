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
# @file   COutputThread.h
# @brief  Thread to for ordered output.
# @author <fox@nscl.msu.edu>
*/
#ifndef COUTPUTTHREAD_H
#define COUTPUTTHREAD_H
#include <Thread.h>

#include "CFragmentHandler.h"
#include <list>
#include <vector>
#include "fragment.h"
#include <CMutex.h>
#include <CBufferQueue.h>
#include <atomic>

/**
 * @class COutputThread
 *     This thread is responsible for handling the output observers for the
 *     event ordering stage of the event builder.  The thread contains
 *     a thread safe queue of pointers to vectors of ordered fragments
 *     and an ordered list of observers.  A mutex protects the list of observers.
 *     The queue has its own mutex/condition variable.
 *
 *     The action of the thread is to get vectors of fragments fromt its thread
 *     safe queue and pass them on to the ordered lists of observers.  Each observer
 *     does as it will with the fragments.   A separate thread is used for two
 *     reasons:
 *      -  Theoretically on a multicore system we can improve performance by
 *         pipelining fragment sorting with fragment outupt.
 *      -  More importantly, without this thread we observed (see Bug #6079) that
 *         flow control on the output would block user interface updates which,
 *         in turn, would mislead the user about how the event builder was
 *         performing.
 *
 */
class COutputThread : public Thread
{
    // Local data:
    
private:
    // Buffer queue between the fragment source and us:
    
    CBufferQueue<std::list<std::pair<time_t, EVB::pFragment> >* > m_inputQueue;
    
    // The mutex guard and the list of observers it guards:
    
    CMutex                                 m_observerGuard;
    std::list<CFragmentHandler::Observer*> m_observers;
    
    // Book keeping for in-flight fragments.
    
    std::atomic<size_t>   m_nInflightCount;
    
public:
    COutputThread();
    virtual ~COutputThread();
    
    // Thread entry point:
    
    virtual void run();
    
    // Methods to manipulate the observer list:
    
public:
    void addObserver(CFragmentHandler::Observer* o);
    void removeObserver(CFragmentHandler::Observer* o);

    // Make fragments available to the thread:
public:
    void queueFragments(std::list<std::pair<time_t, EVB::pFragment> >* pFrags);
    size_t getInflightCount() const;
    
    // Private utilities:
    
private:
    std::list<std::pair<time_t, EVB::pFragment> >* getFragments();
    void freeFragments(std::list<std::pair<time_t, EVB::pFragment> >* frags);
    
};

#endif

