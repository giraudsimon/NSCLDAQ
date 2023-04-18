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

/** @file:  xmlcwritertests.cpp
 *  @brief: Test of DDAS::XMLCrateWriter
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <XMLCrateWriter.h>
#include <XMLCrateReader.h>
#include <ModuleSettings.h>

#include <string>
#include <stdexcept>
#include <sstream>

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

class xmlcwrtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(xmlcwrtest);
    CPPUNIT_TEST(construct_1);
    
    CPPUNIT_TEST(write_1);
    CPPUNIT_TEST(write_2);
    CPPUNIT_TEST(write_3);
    CPPUNIT_TEST(write_4);
    CPPUNIT_TEST(write_5);
    CPPUNIT_TEST_SUITE_END();

protected:
    void construct_1();
    
    void write_1();
    void write_2();
    void write_3();
    void write_4();
    void write_5();
private:
    std::string m_crateFile;
    std::string m_slotFile;
    DDAS::Crate m_settings;
public: 
    void setUp() {
        // Need a temp file name for the crate and
        // another for the slot in the crate:
        
        char nameTemplate[100];
        strcpy(nameTemplate, "crateXXXXXX");
        m_crateFile = tempFile(nameTemplate);
        
        strcpy(nameTemplate,"slotXXXXXX");
        m_slotFile = tempFile(nameTemplate);
    }
    void tearDown() {
        unlink(m_crateFile.c_str());
        unlink(m_slotFile.c_str());
    }

private:
    std::string tempFile(char* t);
    DDAS::ModuleSettings makeSettings();
};


// Give a filename name template for mkstemp, provides
// crates a tempfile and returns its name.

std::string
xmlcwrtest::tempFile(char* t)
{
    int fd = mkstemp(t);
    if (fd < 0) {
        int e = errno;
        std::stringstream msg;
        msg << "Failed to make a temp file for template: "
            << t << " : " << strerror(e);
        throw std::logic_error(msg.str());
    }
    close(fd);
    return std::string(t);
}

DDAS::ModuleSettings
xmlcwrtest::makeSettings()
{
    // Make some module settings:
    
    DDAS::ModuleSettings result;
    memset(&result, 0, sizeof(result));
    
    // Module globals:
    
    result.s_csra = 0x55555555;
    result.s_csrb = 0xaaaaaaaa;
    result.s_SlowFilterRange = 5;
    result.s_FastFilterRange = 3;
    
    for(int i =0; i < 16; i++) {
        result.s_triggerRiseTime[i] = 0.3;
        result.s_triggerFlattop[i]  = 0.03;
        result.s_triggerThreshold[i] = 10.0;
        result.s_Xdt[i] = 0.5;
        result.s_CFDDelay[i] = 1.5;
        result.s_CFDScale[i] = 2;
        result.s_CFDThreshold[i] = 5;
    }
    return result;
}
CPPUNIT_TEST_SUITE_REGISTRATION(xmlcwrtest);

void xmlcwrtest::construct_1()
{
    // Metadata for the crate must have the same number
    // of slots as the Crate.  We'll use 0 slots in the Crate
    // but one metadata record.
    
    DDAS::XMLCrateWriter::ModuleInformation m;
    std::vector<DDAS::XMLCrateWriter::ModuleInformation> info={m};
    DDAS::Crate c;
    
    CPPUNIT_ASSERT_THROW(
        DDAS::XMLCrateWriter w(m_crateFile.c_str(), c, info),
        std::invalid_argument
    );
}

void xmlcwrtest::write_1()
{
    // Bad crate name should fail the write... before doing anything
    
    
    std::vector<DDAS::XMLCrateWriter::ModuleInformation> info;
    DDAS::Crate c;         // Both slots empty.
    c.s_crateId= 1;        // Should not get used.
    
    DDAS::XMLCrateWriter w("/bad/crate/file/name", c, info);
    CPPUNIT_ASSERT_THROW(w.write(), std::invalid_argument);
}
void xmlcwrtest::write_2()
{
    // Bad slot name...we'll get an error for that as well
    // though from the settings writer.
    
    DDAS::XMLCrateWriter::ModuleInformation m;
    m.s_moduleSettingsFile = "/bad/slot/file/name";
    std::vector<DDAS::XMLCrateWriter::ModuleInformation> info = {m};
    
    m_settings.s_crateId = 0;
    DDAS::Slot s;
    s.s_slotNum = 2;
    m_settings.s_slots.push_back(s);
    
    // Construction should be ok but write fails:
    DDAS::XMLCrateWriter w(m_crateFile, m_settings, info);
    CPPUNIT_ASSERT_THROW(w.write(), std::runtime_error);
    
}
void xmlcwrtest::write_3()
{
    // Crate and slot number information are written correctly:
    
        // Bad slot name...we'll get an error for that as well
    // though from the settings writer.
    
    DDAS::XMLCrateWriter::ModuleInformation m;
    m.s_moduleSettingsFile = m_slotFile;
    std::vector<DDAS::XMLCrateWriter::ModuleInformation> info = {m};
    
    m_settings.s_crateId = 0;
    DDAS::Slot s;
    s.s_slotNum = 2;
    m_settings.s_slots.push_back(s);
    
    // Construction should be ok but write fails:
    DDAS::XMLCrateWriter w(m_crateFile, m_settings, info);
    CPPUNIT_ASSERT_NO_THROW(w.write());
    
    // The way to know if this worked is to read it back.
    // we've already written tests that ensure the reader works.
    
    DDAS::XMLCrateReader r(m_crateFile.c_str());
    auto c = r.readCrate();
    
    EQ(unsigned(0), c.s_crateId);
    EQ(size_t(1), c.s_slots.size());
    EQ(unsigned(2), c.s_slots[0].s_slotNum);
}
void xmlcwrtest::write_4()
{
    // Slot global data is correct:
    
    DDAS::XMLCrateWriter::ModuleInformation m ;
    m.s_moduleSettingsFile = m_slotFile;
    std::vector<DDAS::XMLCrateWriter::ModuleInformation> info = {m};
    
    m_settings.s_crateId = 0;
    DDAS::Slot s;
    s.s_slotNum = 2;
    s.s_settings = makeSettings();
    m_settings.s_slots.push_back(s);
    
    // Construction should be ok but write fails:
    DDAS::XMLCrateWriter w(m_crateFile, m_settings, info);
    CPPUNIT_ASSERT_NO_THROW(w.write());
    
    // The way to know if this worked is to read it back.
    // we've already written tests that ensure the reader works.
    
    DDAS::XMLCrateReader r(m_crateFile.c_str());
    auto c = r.readCrate();
    
    auto settings = c.s_slots[0].s_settings;
    EQ(unsigned(0x55555555), settings.s_csra);
    EQ(unsigned(0xaaaaaaaa), settings.s_csrb);
    EQ(unsigned(5), settings.s_SlowFilterRange);
    EQ(unsigned(3), settings.s_FastFilterRange);
}
void xmlcwrtest::write_5()
{
    // Spot check per channel settings
    
    DDAS::XMLCrateWriter::ModuleInformation m ;
    m.s_moduleSettingsFile = m_slotFile;
    std::vector<DDAS::XMLCrateWriter::ModuleInformation> info = {m};
    
    m_settings.s_crateId = 0;
    DDAS::Slot s;
    s.s_slotNum = 2;
    s.s_settings = makeSettings();
    m_settings.s_slots.push_back(s);
    
    // Construction should be ok but write fails:
    DDAS::XMLCrateWriter w(m_crateFile, m_settings, info);
    CPPUNIT_ASSERT_NO_THROW(w.write());
    
    // The way to know if this worked is to read it back.
    // we've already written tests that ensure the reader works.
    
    DDAS::XMLCrateReader r(m_crateFile.c_str());
    auto c = r.readCrate();
    
    auto settings = c.s_slots[0].s_settings;
    for(int i =0; i < 16; i++) {
        EQ(double(0.3), settings.s_triggerRiseTime[i]);
        EQ(double(0.03), settings.s_triggerFlattop[i]);
        EQ(double(10.0), settings.s_triggerThreshold[i]);
        EQ(double(0.5), settings.s_Xdt[i]);
        EQ(double(1.5), settings.s_CFDDelay[i]);
        EQ(double(2), settings.s_CFDScale[i]);
        EQ(double(5), settings.s_CFDThreshold[i]);

    }
}