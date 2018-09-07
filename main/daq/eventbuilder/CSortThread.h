/**

#    This software is Copyright by the Board of Trustees of Michigan
#    State University (c) Copyright 2013-2018
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
# @file   CSortThread.h
# @brief  Thread to sort together extents of fragment queues
# @author <fox@nscl.msu.edu>
*/
#ifndef CSORTTHREAD_H
#define CSORTTHREAD_H

#include <Thread.h>
#include <CFragmentHandler.h>
#include <list>
#include <vector>
#include <CBufferQueue.h>
#include "fragment.h"

/**
 * @param CSortThread
 *    This thread takes vectors of pointer to list of event times/fragment
 *    pairs and merges the fragments by timestamp.  The resulting
 *    merged list is passed on to the output thread for further processing.
 *
 *    Normally, this is run by the fragment handler thread.
 */

class CSortThread : public Thread
{
    // Local data types and member data
    
public:
    typedef std::list<std::pair<time_t, EVB::pFragment> > FragmentList;
    typedef std::vector<FragmentList*> Fragments;
private:
    CBufferQueue<Fragments*> m_fragmentQueue;
    CFragmentHandler*       m_pHandler;
public:
    CSortThread();
    virtual ~CSortThread();
    
    virtual void run();
    
    void queueFragments(Fragments& frags);
private:
    Fragments* dequeueFragments();
    void merge(FragmentList& result, Fragments& lists);
    void clearBufferQueue();
    void releaseFragments(Fragments& frags);
    void releaseFragmentList(FragmentList& frags);
    
};
#endif
