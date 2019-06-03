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

/** @file:  TestClassifier.cpp
 *  @brief: Provide a classifier for testing the swtrigger framework.
 */

#include "CRingItemMarkingWorker.h"

/**
 *  This file contains code to test the swtrigger ring item classification
 *  class.  It classifies every other event seen by the classifier as 1 or 0.
 *  Note given the parallel nature of work, this won't ensure that events
 *  alternate between 1/0 classification...it's just a test to make sure the
 *  system works.
 *
 * It also shows how to supply a createClassifier factory function so let
 */


class TestClassifier : public CRingMarkingWorker::Classifier
{
private:
    int m_counter;
public:
    TestClassifier() : m_counter(0) {}
    virtual ~TestClassifier() {}
    virtual uint32_t operator()(CRingItem& item) {
        m_counter++;
        return (m_counter & 1);
    }
};


extern "C" {                   // Needed for dlsym lookup.
    
CRingMarkingWorker::Classifier* createClassifier() {
    return new TestClassifier;
}
}