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

/** @file:  xmlsetwritertests.cpp
 *  @brief: Tests the DDAS::XMLSettingsWriter class
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "XMLSettingsWriter.h"
#include "XMLSettingsReader.h" // how else you gonna check?

#include <string>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>


class xsetwrtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(xsetwrtest);
    CPPUNIT_TEST(write_1);
    CPPUNIT_TEST(write_2);
    CPPUNIT_TEST(write_3);
    CPPUNIT_TEST(write_4);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_tempfile;
    DDAS::ModuleSettings m_settings;
public:
    void setUp() {
        char ftemplate[100];
        strcpy(ftemplate, "xmlsetwriteXXXXXX");
        int fd = mkstemp(ftemplate);
        if (fd < 0) {
            int e = errno;
            std::stringstream msg;
            msg << "Failed to open temp file with template name "
                << ftemplate << " : " << strerror(e);
            throw std::logic_error(msg.str());
        }
        close(fd);
        m_tempfile = ftemplate;
        
        fillInSettings();
    }
    void tearDown() {
        unlink(m_tempfile.c_str());
    }
protected:
    void write_1();
    void write_2();
    void write_3();
    void write_4();
private:
    void fillInSettings();
};

// Just put some stuff in the settings; values don't actually
// haved to make sense.

void
xsetwrtest::fillInSettings()
{
    memset(&m_settings, 0, sizeof(m_settings)); // mostly zeroes.
    
    m_settings.s_csra = 0xaaaaaaaa;
    m_settings.s_csrb = 0x55555555;
    m_settings.s_SlowFilterRange = 3;
    m_settings.s_FastFilterRange = 4;
    m_settings.s_FastTrgBackPlaneEnables = 0x11111111;
    
    for (int i = 0; i < 16; i++) {
        m_settings.s_triggerRiseTime[i] = 0.4;
        m_settings.s_triggerFlattop[i]  = 0.04;
        m_settings.s_triggerThreshold[i] = 100;
        m_settings.s_energyRiseTime[i]   = 0.5;
        m_settings.s_energyFlattop[i]   = 1.0;
        
        // Rest will be zero.
    }
    
}

CPPUNIT_TEST_SUITE_REGISTRATION(xsetwrtest);

void xsetwrtest::write_1()
{
    // Exception is thrown for bad filename:
    
    DDAS::XMLSettingsWriter w("/this/is/a/bad/filename");
    
    CPPUNIT_ASSERT_THROW(
        w.write(m_settings),
        std::runtime_error
    );
}
void xsetwrtest::write_2()
{
    // No exception for good filename e.g.
    
    DDAS::XMLSettingsWriter w(m_tempfile.c_str());
    CPPUNIT_ASSERT_NO_THROW(
        w.write(m_settings);
    );
}
void xsetwrtest::write_3()
{
    // The module global settings should have been written
    // correctly see fillInSettings for what they should be
    
    DDAS::XMLSettingsWriter w(m_tempfile.c_str());
    w.write(m_settings);
    
    DDAS::XMLSettingsReader r(m_tempfile.c_str());
    auto s = r.get();
    
     
    EQ(unsigned(0xaaaaaaaa), s.s_csra);
    EQ(unsigned(0x55555555), s.s_csrb);
    EQ(unsigned(3), s.s_SlowFilterRange);
    EQ(unsigned(4), s.s_FastFilterRange);
    EQ(unsigned(0x11111111), s.s_FastTrgBackPlaneEnables);
    
}
void xsetwrtest::write_4()
{
    // Spot check module level values.
    
    DDAS::XMLSettingsWriter w(m_tempfile.c_str());
    w.write(m_settings);
    
    DDAS::XMLSettingsReader r(m_tempfile.c_str());
    auto s = r.get();
    
    for (int i =0; i < 16; i++) {
        EQ(double(0.4), s.s_triggerRiseTime[i]);
        EQ(double(0.04), s.s_triggerFlattop[i]);
        EQ(double(100.0), s.s_triggerThreshold[i]);
        EQ(double(0.5), s.s_energyRiseTime[i]);
        EQ(double(1.0), s.s_energyFlattop[i]);
    }
}