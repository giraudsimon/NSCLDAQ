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

/** @file:  setfcwritetests.cpp
 *  @brief: Test suite for the SetFileCrateWriter class.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "SetFileCrateWriter.h"
#include "XMLCrateReader.h"
#include "ModuleSettings.h"
#include "SetFileCrateReader.h"
#include "CrateManager.h"

#include <string>
#include <stdexcept>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <vector>
#include <stdint.h>

const char* srcdir = SOURCE_DIR;
const char* crateFilename = "goodcrate.xml";


class setfcwritetest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(setfcwritetest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    
    CPPUNIT_TEST(write_1);
    CPPUNIT_TEST(write_2);
    CPPUNIT_TEST(write_3);
    CPPUNIT_TEST_SUITE_END();
    
private:
    DDAS::XMLCrateReader* m_pReader;
    DDAS::Crate           m_crate;
    std::string           m_setfile;
    std::vector<std::pair<uint16_t, uint16_t>> m_slotspecs;
    std::vector<uint16_t> m_slots;
    std::vector<uint16_t> m_speeds;
    std::vector<std::string> m_varfiles;
public:
    void setUp() {
        std::string crateFile = srcdir;
        crateFile += "/";
        crateFile += crateFilename;
        m_pReader = new DDAS::XMLCrateReader(crateFile.c_str());
        m_crate   = m_pReader->readCrate();
        
        m_setfile = makeTempName();
        
        // Assume all slots are 250s.
        
        for (int i =0; i < m_crate.s_slots.size(); i++) {
            uint16_t s = m_crate.s_slots[i].s_slotNum;
            std::pair<uint16_t, uint16_t> spec =
                {s, uint16_t(250)};
            m_slotspecs.push_back(spec);
            m_slots.push_back(s);
            m_speeds.push_back(250);
            m_varfiles.push_back(DDAS::CrateManager::getVarFile(250));
        }
    }
    void tearDown() {
        delete m_pReader;
        unlink(m_setfile.c_str());
        m_slotspecs.clear();
        m_slots.clear();
        m_speeds.clear();
        m_varfiles.clear();
    }
protected:
    void construct_1();
    void construct_2();
    
    void write_1();
    void write_2();
    void write_3();
private:
    static std::string makeTempName();
};
/**
 * makeTempName
 *   We want to make a nice temporary filename without
 *   actually keeping the file around.
 */
std::string
setfcwritetest::makeTempName()
{
    const char* nameTemplate = "setfcwrittestXXXXXX";
    char name[200];
    strcpy(name, nameTemplate);
    int stat = mkstemp(name);
    if (stat < 0) {
        int e = errno;
        std::stringstream msg;
        msg << "setfcwrittest - unable to create a tempfile: "
            << strerror(e);
        throw std::logic_error(msg.str());
    }
    close(stat);               // It's an fd.
    unlink(name);
    return std::string(name);
}
CPPUNIT_TEST_SUITE_REGISTRATION(setfcwritetest);

void setfcwritetest::construct_1()
{
    // Constructing the set file crate writer with a bad set file
    // throws.
    
    CPPUNIT_ASSERT_THROW(
        DDAS::SetFileCrateWriter("/this/cannot/exist", m_crate, m_slotspecs),
        std::exception
    );
}
void setfcwritetest::construct_2()
{
    // Construction for a good destination is ok.
    
    DDAS::SetFileCrateWriter* pWriter(nullptr);
    CPPUNIT_ASSERT_NO_THROW(
        pWriter = new DDAS::SetFileCrateWriter(
            m_setfile.c_str(), m_crate, m_slotspecs
        )
    );
    delete pWriter;
}
void setfcwritetest::write_1()
{
    // Should be able to write and read with no exceptions.
    
    DDAS::SetFileCrateWriter Writer(m_setfile.c_str(), m_crate, m_slotspecs);
    CPPUNIT_ASSERT_NO_THROW(Writer.write());
    
    // Now a crate reader:
    
    
    DDAS::SetFileCrateReader r(
        m_setfile.c_str(), 1, m_slots, m_varfiles, m_speeds);
    
    CPPUNIT_ASSERT_NO_THROW(r.readCrate());
}

void setfcwritetest::write_2()
{
    // Crate id and slot ok in read stuff.
    
    DDAS::SetFileCrateWriter Writer(m_setfile.c_str(), m_crate, m_slotspecs);
    Writer.write();
    DDAS::SetFileCrateReader r(
        m_setfile.c_str(), 1, m_slots, m_varfiles, m_speeds);
    DDAS::Crate c = r.readCrate();
    
    EQ(unsigned(1), c.s_crateId);
    EQ(size_t(1), c.s_slots.size());
    EQ(unsigned(2), c.s_slots[0].s_slotNum);
}
void setfcwritetest::write_3()
{
    // There's a match for the module global parameters
    // If that works we'll assume all else works because of our
    // tests for SetFileWriter

    DDAS::SetFileCrateWriter Writer(m_setfile.c_str(), m_crate, m_slotspecs);
    Writer.write();
    DDAS::SetFileCrateReader r(
        m_setfile.c_str(), 1, m_slots, m_varfiles, m_speeds);
    DDAS::Crate c = r.readCrate();
    
    DDAS::ModuleSettings&  is(c.s_slots[0].s_settings);
    DDAS::ModuleSettings&  sb(m_crate.s_slots[0].s_settings);
    EQMSG("csra", sb.s_csra, is.s_csra);
    EQMSG("csrb", sb.s_csrb, is.s_csrb);
    EQMSG("format", sb.s_format, is.s_format);
    EQMSG("maxevents",sb.s_maxEvents, is.s_maxEvents);
    EQMSG("synchwait", sb.s_synchWait, is.s_synchWait);
    EQMSG("insynch", sb.s_inSynch, is.s_inSynch);
    EQMSG("slowfilterrange", sb.s_SlowFilterRange, is.s_SlowFilterRange);
    EQMSG("fastfilterrange", sb.s_FastFilterRange, is.s_FastFilterRange);
    EQMSG(
        "Fast trigger backplane enables",
        sb.s_FastTrgBackPlaneEnables, is.s_FastTrgBackPlaneEnables
    );
    EQMSG("trigger config0", sb.s_trigConfig0, is.s_trigConfig0);
    EQMSG("trigger config1", sb.s_trigConfig1, is.s_trigConfig1);
    EQMSG("trigger config2", sb.s_trigConfig2, is.s_trigConfig2);
    EQMSG("trigger config3", sb.s_trigConfig3, is.s_trigConfig3);
    EQMSG("host rt preset", sb.s_HostRtPreset, is.s_HostRtPreset);
    
}