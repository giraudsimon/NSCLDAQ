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

/** @file:  fragcmdtests.cpp
 *  @brief: Test the CFragmentHandlerCommand.cpp
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "fragcmdstubs.h"
#include "CFragmentHandlerCommand.h"
#include <tcl.h>
#include "TCLInterpreter.h"

class fragcmdtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(fragcmdtest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    CTCLInterpreter*         m_pInterp;
    CFragmentHandlerCommand* m_pCmd;
    CFragmentHandler*        m_pStubs;
public:
    void setUp() {
        m_pInterp = new CTCLInterpreter;
        m_pCmd    = new CFragmentHandlerCommand(*m_pInterp,"handle");
        m_pStubs  = CFragmentHandler::getInstance();
    }
    void tearDown() {
        delete m_pStubs;
        CFragmentHandler::m_pInstance = nullptr;
        delete m_pCmd;
        delete m_pInterp;
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(fragcmdtest);

void fragcmdtest::test_1()
{
}

void* gpTCLApplication(nullptr);