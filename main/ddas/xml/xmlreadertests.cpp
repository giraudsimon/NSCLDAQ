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

/** @file:  xmlreadertests.cpp
 *  @brief: Unittests for the XML Settings reader class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "XMLSettingsReader.h"
#include <string>
#include <stdexcept>


static const std::string FILENAME("bimdev_Module_00.xml");
static const std::string DIR(SRCDIR);

class xmlrdrtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(xmlrdrtest);
    CPPUNIT_TEST(getmodule_1);
    CPPUNIT_TEST(getmodule_2);
    CPPUNIT_TEST(getmodule_3);
    CPPUNIT_TEST(getmodule_4);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_xmlfile;
public:
    void setUp() {
        m_xmlfile = DIR;
        m_xmlfile += "/";
        m_xmlfile += FILENAME;
    }
    void teardDown() {
        
    }
protected:
    void getmodule_1();
    void getmodule_2();
    void getmodule_3();
    void getmodule_4();
};

CPPUNIT_TEST_SUITE_REGISTRATION(xmlrdrtest);

void xmlrdrtest::getmodule_1()
{
    // Get from bad file fails.
    
    DDAS::XMLSettingsReader r("/no/such/xml/file.xml");
    CPPUNIT_ASSERT_THROW(
        r.get(), std::invalid_argument
    );
}
void xmlrdrtest::getmodule_2()
{
    // No exception when the file exists.
    
    DDAS::XMLSettingsReader r(m_xmlfile.c_str());
    CPPUNIT_ASSERT_NO_THROW(r.get());
}
void xmlrdrtest::getmodule_3()
{
    // Spot check module level data.
    
    DDAS::XMLSettingsReader r(m_xmlfile.c_str());
    auto s = r.get();
    
    EQ(unsigned(1), s.s_csra);
    EQ(unsigned(81), s.s_csrb);
    EQ(unsigned(3), s.s_SlowFilterRange);
    EQ(unsigned(1203982208), s.s_HostRtPreset);
    
}
void xmlrdrtest::getmodule_4()
{
    DDAS::XMLSettingsReader r(m_xmlfile.c_str());
    auto s = r.get();
    
    for(int i =0; i < 16; i++) {
        EQ(double(0.4), s.s_triggerRiseTime[i]);
        EQ(double(0.08), s.s_triggerFlattop[i]);
    }
    
}
