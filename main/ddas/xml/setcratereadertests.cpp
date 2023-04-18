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

/** @file:  setcraterdrtests.cpp
 *  @brief: Tests for the set file crate reader class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "SetFileCrateReader.h"

#include <string>

// Note that we depend on the following file to live in @srcdir@

static const std::string dir(SRCDIR);
static const std::string setfile("crate_1.set");
static const std::string varfile("test.var");

class sfcreadertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sfcreadertest);
    CPPUNIT_TEST(read_1);
    CPPUNIT_TEST(read_2);
    CPPUNIT_TEST(read_3);
    CPPUNIT_TEST(read_4);
    CPPUNIT_TEST(read_5);
    CPPUNIT_TEST(read_6);
    CPPUNIT_TEST(read_7);
    CPPUNIT_TEST(read_8);
    CPPUNIT_TEST(read_9);
    CPPUNIT_TEST(read_10);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_setpath;
    std::string m_varpath;
    std::vector<unsigned short> m_slots;
    std::vector<std::string> m_varFiles;
    std::vector<unsigned short> m_MHz;
public:
    void setUp() {
        m_setpath = dir;
        m_setpath += "/";      // the fs::path starts in C++17.
        m_setpath += setfile;
        
        m_varpath = dir;
        m_varpath += "/";
        m_varpath += varfile;
        
        m_slots.clear();
        m_varFiles.clear();
        m_MHz.clear();
        
        m_slots.push_back(2);
        m_slots.push_back(4);
        m_slots.push_back(8);
        
        m_varFiles.push_back(m_varpath);
        m_varFiles.push_back(m_varpath);
        m_varFiles.push_back(m_varpath);
        
        m_MHz.push_back(100);
        m_MHz.push_back(250);
        m_MHz.push_back(500);
    }
    void teardDown() {
        
    }
protected:
    void read_1();
    void read_2();
    void read_3();
    void read_4();
    void read_5();
    void read_6();
    void read_7();
    void read_8();
    void read_9();
    void read_10();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sfcreadertest);

void sfcreadertest::read_1()
{
    // If the set file is bad, this all fails.
    
    DDAS::SetFileCrateReader reader(
        "/this/file/does/not/exist", 1, m_slots, m_varFiles, m_MHz
    );
    CPPUNIT_ASSERT_THROW(
        reader.readCrate(),
        std::invalid_argument
    );
    
    
}
void sfcreadertest::read_2()
{
    // var file array size must match slots size:
    
    m_varFiles.pop_back();
    
    CPPUNIT_ASSERT_THROW(
        DDAS::SetFileCrateReader rdr(
            m_setpath.c_str(), 1, m_slots, m_varFiles, m_MHz
        ),
        std::invalid_argument
    );
}
void sfcreadertest::read_3()
{
    // MHz array must match slt size:
    
    m_MHz.pop_back();
    CPPUNIT_ASSERT_THROW(
        DDAS::SetFileCrateReader rdr(
            m_setpath.c_str(), 1, m_slots, m_varFiles, m_MHz
        ),
        std::invalid_argument
    );
}
void sfcreadertest::read_4()
{
    // illegal var file also throws on readCrate
    
    m_varFiles[0] += "junkly";
    DDAS::SetFileCrateReader rdr(
        m_setpath.c_str(), 1, m_slots, m_varFiles, m_MHz  
    );
    CPPUNIT_ASSERT_THROW(
        rdr.readCrate(), std::invalid_argument
    );
}
void sfcreadertest::read_5()
{
    // unmodified Setup reads with no exception throws.
    
    DDAS::SetFileCrateReader rdr(
        m_setpath.c_str(), 1, m_slots, m_varFiles, m_MHz  
    );
    CPPUNIT_ASSERT_NO_THROW(rdr.readCrate());
}
void sfcreadertest::read_6()
{
    // When we read get the crate id and number of slots right:
    
    DDAS::SetFileCrateReader rdr(
        m_setpath.c_str(), 1, m_slots, m_varFiles, m_MHz  
    );
    auto settings = rdr.readCrate();
    EQ(unsigned(1), settings.s_crateId);
    EQ(size_t(3), settings.s_slots.size());
}
void sfcreadertest::read_7()
{
    // When we read, the slots have the right slot numbers and
    
    DDAS::SetFileCrateReader rdr(
        m_setpath.c_str(), 1, m_slots, m_varFiles, m_MHz  
    );
    auto settings = rdr.readCrate();
    EQ(unsigned(2), settings.s_slots[0].s_slotNum);
    EQ(unsigned(4), settings.s_slots[1].s_slotNum);
    EQ(unsigned(8), settings.s_slots[2].s_slotNum);
}
void sfcreadertest::read_8()
{
    // Spot check values in slot 2 (id 0).
    
    DDAS::SetFileCrateReader rdr(
        m_setpath.c_str(), 1, m_slots, m_varFiles, m_MHz  
    );
    auto settings = rdr.readCrate();
    auto& s(settings.s_slots[0].s_settings);
    EQ(unsigned(1), s.s_csra);
    EQ(unsigned(0x41), s.s_csrb);
    EQ(unsigned(3), s.s_SlowFilterRange);
    for (int i =0; i < 16; i++) {
        EQ(uint32_t(0x24), s.s_chanCsra[i]);
        
    }
    
}
void sfcreadertest::read_9()
{
    // Spot check values for slot 4 (id 1).
    
    DDAS::SetFileCrateReader rdr(
        m_setpath.c_str(), 1, m_slots, m_varFiles, m_MHz  
    );
    auto settings = rdr.readCrate();
    auto& s(settings.s_slots[1].s_settings);
    
    EQ(unsigned(1), s.s_csra);
    EQ(unsigned(0x1), s.s_csrb);
    EQ(unsigned(3), s.s_SlowFilterRange);
    for (int i =0; i < 16; i++) {
        EQ(uint32_t(0x24), s.s_chanCsra[i]);
        
    }
}
void sfcreadertest::read_10()
{
    // spot check values from slot 8 (id 2).
    
    DDAS::SetFileCrateReader rdr(
        m_setpath.c_str(), 1, m_slots, m_varFiles, m_MHz  
    );
    auto settings = rdr.readCrate();
    auto& s(settings.s_slots[1].s_settings);
    
    EQ(unsigned(1), s.s_csra);
    EQ(unsigned(0x1), s.s_csrb);
    EQ(unsigned(3), s.s_SlowFilterRange);
    for (int i =0; i < 16; i++) {
        EQ(uint32_t(0x24), s.s_chanCsra[i]);
        
    }
}