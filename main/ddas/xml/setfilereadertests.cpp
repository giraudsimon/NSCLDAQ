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
	     East Lansing, MI 48824-1321a
*/

/** @file:  setfiletests.cpp
 *  @brief: unittests for the SetFile class methods.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "SetFileReader.h"             // Under test.
#include "Asserts.h"
#include <string>
#include <stdexcept>

// Note that we depend on the following file to live in @srcdir@

static const std::string dir(SRCDIR);
static const std::string setfile("crate_1.set");
static const std::string varfile("test.var");

class sfreadertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sfreadertest);
    CPPUNIT_TEST(get_1);
    CPPUNIT_TEST(get_2);
    CPPUNIT_TEST(get_3);
    CPPUNIT_TEST(get_4);
    CPPUNIT_TEST(get_5);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_setpath;
    std::string m_varpath;
    
public:
    void setUp() {
        m_setpath = dir;
        m_setpath += "/";      // the fs::path starts in C++17.
        m_setpath += setfile;
        
        m_varpath = dir;
        m_varpath += "/";
        m_varpath += varfile;
        
    }
    void teardDown() {
        
    }
protected:
    void get_1();
    void get_2();
    void get_3();
    void get_4();
    void get_5();
    
};

CPPUNIT_TEST_SUITE_REGISTRATION(sfreadertest);

void sfreadertest::get_1()
{
    // Constructing with a bad setfile is an exception.
    
    DDAS::SetFileReader x("/no/such/setfile", m_varpath.c_str(), 100,2);
    CPPUNIT_ASSERT_THROW(
        x.get(),
        std::invalid_argument
    );
}
void sfreadertest::get_2()
{
    // constructing with a bad var file throws.
    
    DDAS::SetFileReader x(m_setpath.c_str(), "/no/such/varfile", 100,2);
    CPPUNIT_ASSERT_THROW(
        x.get(),
        std::invalid_argument
    );
    
}
void sfreadertest::get_3()
{
    // Getting bad slot throws.
    
    DDAS::SetFileReader x(m_setpath.c_str(), m_varpath.c_str(), 100, 1);
    CPPUNIT_ASSERT_THROW(
        x.get(),
        std::invalid_argument
    );
}
void sfreadertest::get_4()
{
    // Good get
    
    DDAS::SetFileReader x(m_setpath.c_str(), m_varpath.c_str(), 100, 2);
    CPPUNIT_ASSERT_NO_THROW(x.get());
}
void sfreadertest::get_5()
{
    // Spot check the parameter values:
    
    DDAS::SetFileReader x(m_setpath.c_str(), m_varpath.c_str(), 100, 2);
    auto settings = x.get();
    
    EQ((unsigned int)(1), settings.s_csra);
    EQ((unsigned int)(0x41), settings.s_csrb);
    for(int i =0; i < 16; i++) {
        EQ(uint32_t(0x24), settings.s_chanCsra[i]);
    }
}