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

#define protected public
#include "CFragmentHandlerCommand.h"
#undef protected
#include <tcl.h>
#include "TCLInterpreter.h"
#include "TCLObject.h"
#include <vector>

class fragcmdtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(fragcmdtest);
    CPPUNIT_TEST(frags_1);
    CPPUNIT_TEST(frags_2);
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
    void frags_1();
    void frags_2();
};

CPPUNIT_TEST_SUITE_REGISTRATION(fragcmdtest);



void fragcmdtest::frags_1()
{
    // Zero length fragments.
    
    CTCLObject name; name.Bind(*m_pInterp);
    name = "command";
    CTCLObject sock; sock.Bind(*m_pInterp);
    sock = "stdin";
    Tcl_Obj* pFrags = Tcl_NewByteArrayObj(nullptr, 0);
    CTCLObject frags(pFrags);
    
    std::vector<CTCLObject> command = {name, sock, frags};
    int status = (*m_pCmd)(*m_pInterp, command);
    EQ(TCL_OK, status);
    EQ(size_t(0), m_pStubs->m_nLastSize);
    
    int n;
    EQ((const EVB::FlatFragment*)(Tcl_GetByteArrayFromObj(pFrags, &n)), m_pStubs->m_pLastFrags);
}

void fragcmdtest::frags_2()
{

    CTCLObject name; name.Bind(*m_pInterp);
    name = "command";
    CTCLObject sock; sock.Bind(*m_pInterp);
    sock = "stdin";
    uint8_t bytes[100];
    for (int i =0; i < 100; i++) bytes[i] = i;
    
    Tcl_Obj* pFrags = Tcl_NewByteArrayObj(bytes, 100);
    CTCLObject frags(pFrags);
    std::vector<CTCLObject> command = {name, sock, frags};
    int status = (*m_pCmd)(*m_pInterp, command);
    EQ(TCL_OK, status);
    EQ(size_t(100), m_pStubs->m_nLastSize);
    
    const uint8_t* is = reinterpret_cast<const uint8_t*>(m_pStubs->m_pLastFrags);
    for (int i =0; i < 100; i++) {
        EQ(bytes[i], is[i]);
    }
}

void* gpTCLApplication(nullptr);

