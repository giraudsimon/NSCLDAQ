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
# @brief  Implements COutputThread - see the header for description/rationale.
# @author <fox@nscl.msu.edu>
*/

#include "COutputThread.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

/**
 * Constructor
 *   For now this is null, just setting the thread name in the base class
 *   constructor.
 */
COutputThread::COutputThread() :
Thread(std::string("OutputThread")),
m_nInflightCount(0)
{
    
}

/**
 * destructor
 *   Null just giving the base class a chance to run:
 */
COutputThread::~COutputThread() {}


/*---------------------------------------------------------------------------
 * Thread entry point.
 */
void
COutputThread::run()
{
    while (1) {
        std::vector<EVB::pFragment>* pFrags = getFragments();
        m_nInflightCount -= (pFrags->size());
        {
            CriticalSection c(m_observerGuard);
            for (auto p = m_observers.begin(); p != m_observers.end(); p++) {
                CFragmentHandler::Observer* pO = *p;
                (*pO)(*pFrags);
            }
        }
        freeFragments(pFrags);
    }
}
/*---------------------------------------------------------------------------
 * Methods called by other threads:
 */

/**
 *  addObserver
 *     Add a new observer to the end of the list
 *
 *  @param o - pointer to an objecte derived from CFragmentHandler::Observer.
 */
void
COutputThread::addObserver(CFragmentHandler::Observer* o)
{
    CriticalSection c(m_observerGuard);
    m_observers.push_back(o);
    
}
/**
 *  removeObserver
 *    Remove exsting observer from the list:
 *
 *  @param o - observer to remove.,
 *  @throw std::invalid_argument if o is not an observer.
 */
void
COutputThread::removeObserver(CFragmentHandler::Observer* o)
{
    CriticalSection c(m_observerGuard);
    std::list<CFragmentHandler::Observer*>::iterator p =
        std::find(m_observers.begin(), m_observers.end(), o);
    if (p == m_observers.end()) {
        throw std::invalid_argument("COutputThread::removeObserber - observer does not exist");
    }
    m_observers.erase(p);
}

/**
 * queueFragments
 *    Queue a vector of fragments to the thread.  The run() method will notice
 *    these eventually, invoke the observers on them and clean up.
 *
 *  @param pFrags - pointer to a vector of fragment pointers we queue for
 *                  processing.
 */
void
COutputThread::queueFragments(std::list<std::pair<time_t, EVB::pFragment> >* pFrags)
{
    m_nInflightCount += pFrags->size();
    m_inputQueue.queue(pFrags);
}
/**
 * getInflightCount
 *    Return the number of fragments in the queue.
 *
 * @return size_t
 */
size_t
COutputThread::getInflightCount() const
{
    return m_nInflightCount;
}

/*----------------------------------------------------------------------------
 * private utilities
 */

/**
 * getFragments
 *    Return the next set of fragments that were queued for processing by our
 *    observers.
 * @return std::vector<EVB::pFragment>*
 */
std::vector<EVB::pFragment>*
COutputThread::getFragments()
{
    std::list<std::pair<time_t, EVB::pFragment> >* pFragmentList =
        m_inputQueue.get();
    
    std::vector<EVB::pFragment>& result(*(new std::vector<EVB::pFragment>));

    for(auto f = pFragmentList->begin(); f != pFragmentList->end(); f++) {
        result.push_back(f->second);
    }
    delete pFragmentList;  // Only the list, not what the fragment pointers point to.
    return &result;        // result is a ref dynamically created so this is ok.
    
    
}
/**
 * freeFragments
 *    Releases storage associated with a set of fragments
 *
 * @param frags - pointer to a fragment vector
 * @note  There are two strong assumptions made by this method:
 *        *  The fragments are all dynamically allocated.
 *        *  The vector itself was dynamically allocated:
 */
void
COutputThread::freeFragments(std::vector<EVB::pFragment>* frags)
{
    std::vector<EVB::pFragment>& fs(*frags);
    // free storage associated with each fragment.
    
    for (int i = 0; i < fs.size(); i++) {
        freeFragment(fs[i]);
    }
    // and now the vector itself:
    
    delete frags;
}
