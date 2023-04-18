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

/** @file:  xmlcreadertests.cpp
 *  @brief: Unit tests for XMLCrateReader class
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "XMLCrateReader.h"
#include "XMLCrateWriter.h"
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <fstream>

const std::string GOODCRATE="goodcrate.xml";
const std::string BADCRATE="badcrate.xml";
const std::string SETTINGSFILE="bimdev_Module_00.xml";
const std::string DIRNAME=SRCDIR;

class xmlcreadertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(xmlcreadertest);
  CPPUNIT_TEST(createTestInput);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    CPPUNIT_TEST(construct_4);
    
    CPPUNIT_TEST(read_1);
    CPPUNIT_TEST(read_2);
    CPPUNIT_TEST(read_3);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_badcrate;
    std::string m_goodcrate;
    std::string m_settingsfile;
public:
    void setUp() {
        m_badcrate = DIRNAME;
        m_badcrate += "/";
        m_badcrate += BADCRATE;
        
        m_goodcrate = DIRNAME;
        m_goodcrate += "/";
        m_goodcrate += GOODCRATE;

	// settings file in same directory as good/badcrate.xml
	m_settingsfile = DIRNAME;
	m_settingsfile += "/";
	m_settingsfile += SETTINGSFILE;
	
    }
    void teardDown() {
        
    }
protected:
  void createTestInput();
  
    void construct_1();
    void construct_2();
    void construct_3();
    void construct_4();
    
    void read_1();
    void read_2();
    void read_3();
};

CPPUNIT_TEST_SUITE_REGISTRATION(xmlcreadertest);

void xmlcreadertest::createTestInput()
{
  // make the goodcrate.xml file pointing at the right settings
  // (manually format prior to xml write unit test)
  
  CPPUNIT_ASSERT_NO_THROW(
  	std::ofstream cratexml;
        cratexml.open(m_goodcrate.c_str(), std::ios::out);
        cratexml << "<crate id=\"1\">\n";
        cratexml << "  <slot number=\"2\" evtlen=\"4\" configfile=\"" << m_settingsfile << "\" />\n";
        cratexml << "</crate>";
        cratexml.close();
  );
}

void xmlcreadertest::construct_1()
{
    // Invalid crate filename throws on construction.
    
    CPPUNIT_ASSERT_THROW(
        new DDAS::XMLCrateReader("/no/such/xml/file/crate.xml"),
        std::invalid_argument
    );
    
}
void xmlcreadertest::construct_2()
{
    // Bad crate file throws on construction.
    
    CPPUNIT_ASSERT_THROW(
        new DDAS::XMLCrateReader(m_badcrate.c_str()),
        std::invalid_argument
    );
}
void xmlcreadertest::construct_3()
{
    // Good crate file does not throw on construction.
    
    DDAS::XMLCrateReader* pReader;
    CPPUNIT_ASSERT_NO_THROW(
        pReader = new DDAS::XMLCrateReader(m_goodcrate.c_str())
    );
    delete pReader;
}

void xmlcreadertest::read_1()
{
    // Reading the crate will give the right crate ID and
    // right number of slots:
    
    DDAS::XMLCrateReader reader(m_goodcrate.c_str());
    auto config = reader.readCrate();
    
    EQ(unsigned(1), config.s_crateId);
    EQ(size_t(1), config.s_slots.size());
}
void xmlcreadertest::read_2()
{
    // Check the module global parameters.
    
    DDAS::XMLCrateReader reader(m_goodcrate.c_str());
    auto config = reader.readCrate();
    auto& slot(config.s_slots.at(0));
    
    EQ(unsigned(2), slot.s_slotNum);
    auto& settings(slot.s_settings);
    
    EQ(unsigned(1), settings.s_csra);
    EQ(unsigned(81), settings.s_csrb);
    EQ(unsigned(3), settings.s_SlowFilterRange);
    EQ(unsigned(0), settings.s_FastFilterRange);
    EQ(unsigned(0), settings.s_FastTrgBackPlaneEnables);
    EQ(unsigned(0), settings.s_trigConfig0);
    EQ(unsigned(0), settings.s_trigConfig1);
    EQ(unsigned(0), settings.s_trigConfig2);
    EQ(unsigned(0), settings.s_trigConfig3);
    
}
void xmlcreadertest::read_3()
{
    DDAS::XMLCrateReader reader(m_goodcrate.c_str());
    auto config = reader.readCrate();
    auto& slot(config.s_slots.at(0));
    
    EQ(unsigned(2), slot.s_slotNum);
    auto& settings(slot.s_settings);
    
    // The csra dependson channel:
    std::vector<uint32_t> csra = {
        36, 132, 36, 36, 36, 36, 36, 36,
        164, 36, 36, 36, 36, 36, 36, 36
    };
    for (int i =0; i < 16; i++) {
        EQ(double(0.4), settings.s_triggerRiseTime[i]);
        EQ(double(0.08), settings.s_triggerFlattop[i]);
        EQ(double(65.0), settings.s_triggerThreshold[i]);
        EQ(double(0.06), settings.s_Xdt[i]);
        EQ(csra[i], settings.s_chanCsra[i]);
    }
}
void xmlcreadertest::construct_4()
{
    // After construction we should be able to get
    // the crate id:
    
    DDAS::XMLCrateReader r(m_goodcrate.c_str());
    EQ((unsigned short)(1), r.getCrateId());
}
