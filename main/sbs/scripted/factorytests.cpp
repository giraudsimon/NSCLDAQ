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

#define private public
#include "CModuleCommand.h"
#undef private

#include "CDigitizerDictionary.h"
#include "TCLInterpreter.h"
#include "CCAENV775Creator.h"
#include "CCAENV785Creator.h"
#include "CCAENV792Creator.h"
#include "CCAENV830Creator.h"
#include "CSIS3300Creator.h"
#include "CV1x90Creator.h"

class factorytest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(factorytest);
    CPPUNIT_TEST(initial_1);
    
    CPPUNIT_TEST(v775_1);
    CPPUNIT_TEST(v775_2);
    CPPUNIT_TEST(v775_3);
    
    CPPUNIT_TEST(v785_1);
    CPPUNIT_TEST(v785_2);
    CPPUNIT_TEST(v785_3);
    
    CPPUNIT_TEST(v792_1);
    CPPUNIT_TEST(v792_2);
    CPPUNIT_TEST(v792_3);
    
    CPPUNIT_TEST(v830_1);
    CPPUNIT_TEST(v830_2);
    CPPUNIT_TEST(v830_3);
    
    CPPUNIT_TEST(sis3300_1);
    CPPUNIT_TEST(sis3300_2);
    CPPUNIT_TEST(sis3300_3);
    
    CPPUNIT_TEST(v1x90_1);
    CPPUNIT_TEST(v1x90_2);
    CPPUNIT_TEST(v1x90_3);
    
    
    CPPUNIT_TEST_SUITE_END();

protected:
    void initial_1();
    
    void v775_1();
    void v775_2();
    void v775_3();
    
    void v785_1();
    void v785_2();
    void v785_3();
    
    void v792_1();
    void v792_2();
    void v792_3();
    
    void v830_1();
    void v830_2();
    void v830_3();

    void sis3300_1();
    void sis3300_2();
    void sis3300_3();
    
    void v1x90_1();
    void v1x90_2();
    void v1x90_3();
private:
    CTCLInterpreter*      m_pInterp;
    CDigitizerDictionary* m_pDict;
    CModuleCommand*       m_pCommand;
    
public:
    void setUp() {
        m_pInterp = new CTCLInterpreter;
        m_pDict   = new CDigitizerDictionary;
        m_pCommand = new CModuleCommand(m_pInterp, m_pDict);
    }
    void tearDown() {
        delete m_pCommand;
        delete m_pDict;
        delete m_pInterp;
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(factorytest);

void factorytest::initial_1()
{
    // Initially there are no modules in the dictionary.
    // and the factory has no creators:
    
    EQ(0, m_pDict->DigitizerSize());
    std::vector<std::string> ds =
        m_pCommand->m_factory.getDescriptions();
    EQ(size_t(0), ds.size());
}
void factorytest::v775_1()
{
    // We can add a v775creator
    
    CCAENV775Creator pc;
    m_pCommand->AddCreator("v775", &pc);
    
    EQ(size_t(1), m_pCommand->m_factory.getDescriptions().size());
    
}
void factorytest::v775_2()
{
    // We can create a v775 module and it's added to the
    // commands as well as creating a new module:
    
    CCAENV775Creator pc;
    m_pCommand->AddCreator("v775", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude v775")
    );
    EQ(1, m_pDict->DigitizerSize());    // We created a module.
    
    // The command also exists:
    
    std::string result = m_pInterp->GlobalEval("info command dude");
    EQ(std::string("dude"), result);
    
}
void factorytest::v775_3()
{
    // We can add and configure a v775 module in one fell swoop.
    
    // We can create a v775 module and it's added to the
    // commands as well as creating a new module:
    
    CCAENV775Creator pc;
    m_pCommand->AddCreator("v775", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude v775 base 0x12340000")
    );
}

void factorytest::v785_1()
{
    // We can add a v785creator
    
    CCAENV785Creator pc;
    m_pCommand->AddCreator("v785", &pc);
    
    EQ(size_t(1), m_pCommand->m_factory.getDescriptions().size());
    
}
void factorytest::v785_2()
{
    // We can create a v785 module and it's added to the
    // commands as well as creating a new module:
    
    CCAENV785Creator pc;
    m_pCommand->AddCreator("v785", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude v785")
    );
    EQ(1, m_pDict->DigitizerSize());    // We created a module.
    
    // The command also exists:
    
    std::string result = m_pInterp->GlobalEval("info command dude");
    EQ(std::string("dude"), result);
    
}
void factorytest::v785_3()
{
    // We can add and configure a v785 module in one fell swoop.
    
    // We can create a v785 module and it's added to the
    // commands as well as creating a new module:
    
    CCAENV785Creator pc;
    m_pCommand->AddCreator("v785", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude v785 base 0x12340000")
    );
    
}

void factorytest::v792_1()
{
    // We can add a v792creator
    
    CCAENV792Creator pc;
    m_pCommand->AddCreator("v792", &pc);
    
    EQ(size_t(1), m_pCommand->m_factory.getDescriptions().size());
    
}
void factorytest::v792_2()
{
    // We can create a v792 module and it's added to the
    // commands as well as creating a new module:
    
    CCAENV792Creator pc;
    m_pCommand->AddCreator("v792", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude v792")
    );
    EQ(1, m_pDict->DigitizerSize());    // We created a module.
    
    // The command also exists:
    
    std::string result = m_pInterp->GlobalEval("info command dude");
    EQ(std::string("dude"), result);
    
}
void factorytest::v792_3()
{
    // We can add and configure a v792 module in one fell swoop.
    
    // We can create a v792 module and it's added to the
    // commands as well as creating a new module:
    
    CCAENV792Creator pc;
    m_pCommand->AddCreator("v792", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude v792 base 0x12340000")
    );
}   



void factorytest::v830_1()
{
    // We can add a v830creator
    
    CCAENV830Creator pc;
    m_pCommand->AddCreator("v830", &pc);
    
    EQ(size_t(1), m_pCommand->m_factory.getDescriptions().size());
    
}
void factorytest::v830_2()
{
    // We can create a v830 module and it's added to the
    // commands as well as creating a new module:
    
    CCAENV830Creator pc;
    m_pCommand->AddCreator("v830", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude v830")
    );
    EQ(1, m_pDict->DigitizerSize());    // We created a module.
    
    // The command also exists:
    
    std::string result = m_pInterp->GlobalEval("info command dude");
    EQ(std::string("dude"), result);
    
}
void factorytest::v830_3()
{
    // We can add and configure a v830 module in one fell swoop.
    
    // We can create a v830 module and it's added to the
    // commands as well as creating a new module:
    
    CCAENV830Creator pc;
    m_pCommand->AddCreator("v830", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude v830 base 0x12340000")
    );
    
}   


void factorytest::sis3300_1()
{
    // We can add a sis3300creator
    
    CSIS3300Creator pc;
    m_pCommand->AddCreator("sis3300", &pc);
    
    EQ(size_t(1), m_pCommand->m_factory.getDescriptions().size());
    
}
void factorytest::sis3300_2()
{
    // We can create a sis3300 module and it's added to the
    // commands as well as creating a new module:
    
    CSIS3300Creator pc;
    m_pCommand->AddCreator("sis3300", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude sis3300")
    );
    EQ(1, m_pDict->DigitizerSize());    // We created a module.
    
    // The command also exists:
    
    std::string result = m_pInterp->GlobalEval("info command dude");
    EQ(std::string("dude"), result);
    
}

void factorytest::sis3300_3()
{
    // We can add and configure a sis3300 module in one fell swoop.
    
    // We can create a sis3300 module and it's added to the
    // commands as well as creating a new module:
    
    CSIS3300Creator pc;
    m_pCommand->AddCreator("sis3300", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude sis3300 base 0x12340000")
    );
    
}   



void factorytest::v1x90_1()
{
    // We can add a v1x90creator
    
    CV1x90Creator pc;
    m_pCommand->AddCreator("v1x90", &pc);
    
    EQ(size_t(1), m_pCommand->m_factory.getDescriptions().size());
    
}
void factorytest::v1x90_2()
{
    // We can create a v1x90 module and it's added to the
    // commands as well as creating a new module:
    
    CV1x90Creator pc;
    m_pCommand->AddCreator("v1x90", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude v1x90")
    );
    EQ(1, m_pDict->DigitizerSize());    // We created a module.
    
    // The command also exists:
    
    std::string result = m_pInterp->GlobalEval("info command dude");
    EQ(std::string("dude"), result);
    
}
void factorytest::v1x90_3()
{
    // We can add and configure a v1x90 module in one fell swoop.
    
    // We can create a v1x90 module and it's added to the
    // commands as well as creating a new module:
    
    CV1x90Creator pc;
    m_pCommand->AddCreator("v1x90", &pc);
    CPPUNIT_ASSERT_NO_THROW(
        m_pInterp->GlobalEval("module dude v1x90 base 0x12340000")
    );
    
}   
