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

/** @file:  
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "CConfiguration.h"
#include <TCLInterpreter.h>
#include <tcl.h>
#include <tclUtil.h>
#include <CMockVMUSB.h>

class ctlexporttest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(ctlexporttest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:

public:
    void setUp() {
        
    }
    void tearDown() {
        
    }
protected:
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(ctlexporttest);

void ctlexporttest::test_1()
{
    CConfiguration config;
    CMockVMUSB     controller;
    config.exportController(&controller);
    
    // figure out what ::Globals::aController SHould be:
    
    std::string sb = tclUtil::swigPointer(&controller, "CVMUSB");
    
    // Get the variable value:
    
    Tcl_Interp* pRaw = config.getInterpreter()->getInterpreter();
    const char* value = Tcl_GetVar(pRaw, "::Globals::aController", 0);
    
    ASSERT(value);
    EQ(sb, std::string(value));
}