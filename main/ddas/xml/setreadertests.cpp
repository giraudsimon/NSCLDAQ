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

/** @file:  setreadertests.cpp
 *  @brief: Tests for the setfiler reader.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include "SetFile.h"
#include <string>
#include <stdexcept>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

// Note that we depend on the following file to live in @srcdir@

static const std::string dir(SRCDIR);
static const std::string setfile("crate_1.set");
static const std::string varfile("test.var");

class setfiletest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(setfiletest);
    CPPUNIT_TEST(openvar_1);
    CPPUNIT_TEST(openvar_2);
    
    CPPUNIT_TEST(varread_1);
    CPPUNIT_TEST(varread_2);
    
    CPPUNIT_TEST(vomap_1);
    CPPUNIT_TEST(vomap_2);
    
    CPPUNIT_TEST(vnmap_1);
    CPPUNIT_TEST(vnmap_2);
    
    CPPUNIT_TEST(openset_1);
    CPPUNIT_TEST(openset_2);
    CPPUNIT_TEST(setread_1);
    
    CPPUNIT_TEST(setwrite_1);
    CPPUNIT_TEST(setwrite_2);
    CPPUNIT_TEST(setwrite_3);
    
    CPPUNIT_TEST(array_1);
    CPPUNIT_TEST(array_2);
    
    CPPUNIT_TEST(sfmap_1);
    CPPUNIT_TEST(sfmap_2);
    CPPUNIT_TEST(sfmap_3);
    CPPUNIT_TEST(sfmap_4);
    CPPUNIT_TEST(sfmap_5);
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
    void openvar_1();
    void openvar_2();
    
    void varread_1();
    void varread_2();
    
    void vomap_1();
    void vomap_2();
    
    void vnmap_1();
    void vnmap_2();
    
    void openset_1();
    void openset_2();
    void setread_1();
    
    void setwrite_1();
    void setwrite_2();
    void setwrite_3();
    
    void array_1();
    void array_2();
    
    void sfmap_1();
    void sfmap_2();
    void sfmap_3();
    void sfmap_4();
    void sfmap_5();
};

CPPUNIT_TEST_SUITE_REGISTRATION(setfiletest);

void setfiletest::openvar_1()
{
    // With invalid filename, throws std::invalid_argument.
    
    CPPUNIT_ASSERT_THROW(
        DDAS::SetFile::readVarFile("/file/does/not/exist"),
        std::invalid_argument
    );
}
void setfiletest::openvar_2()
{
    // Open a valid file is good.
    
    CPPUNIT_ASSERT_NO_THROW(
        DDAS::SetFile::readVarFile(m_varpath.c_str())
    );
}
void setfiletest::varread_1()
{
    // Check the first few entries:
    
    auto vars = DDAS::SetFile::readVarFile(m_varpath.c_str());
    EQ(std::string("ModNum"), vars[0].s_name);
    EQ(unsigned(0x00), vars[0].s_longoff);
    EQ(unsigned(1), vars[0].s_nLongs);
    
    EQ(std::string("HostIO"), vars[0xf].s_name);   // First per channel item.
    EQ(unsigned(0xf), vars[0xf].s_longoff);
    EQ(unsigned(16), vars[0xf].s_nLongs);
}
void setfiletest::varread_2()
{
    // Check some of the last records.
    
    auto vars = DDAS::SetFile::readVarFile(m_varpath.c_str());
    auto p    = vars.end();
    --p;                             // 'points' to the last one.
    
    auto& vl(*p);
    EQ(std::string("U30"), vl.s_name);
    EQ(unsigned(0x44f), vl.s_longoff);
    EQ(unsigned(1), vl.s_nLongs);
    
    --p;
    auto vn(*p);                   // Next to last.
    EQ(std::string("AutoTau"), vn.s_name);
    EQ(unsigned(0x43f), vn.s_longoff);
    EQ(unsigned(16), vn.s_nLongs);
}

void setfiletest::vomap_1()
{
    // each array element maps to a distinct address map element.
    
    auto vars   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    auto varmap = DDAS::SetFile::createVarOffsetMap(vars);
    
    EQ(vars.size(), varmap.size());
}
void setfiletest::vomap_2()
{
    // spot check a few offsets.
    
    auto vars   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    auto varmap = DDAS::SetFile::createVarOffsetMap(vars);
    
    auto& modNum(varmap[0]);
    auto& slowGap(varmap[0xa0]);
    auto& rta(varmap[0x340]);
    
    // ModNum:
    
    EQ(std::string("ModNum"), modNum.s_name);
    EQ(unsigned(0), modNum.s_longoff);
    EQ(unsigned(1), modNum.s_nLongs);
    
    // slowGap:
    
    EQ(std::string("SlowGap"), slowGap.s_name);
    EQ(unsigned(0xa0), slowGap.s_longoff);
    EQ(unsigned(16), slowGap.s_nLongs);
    
    // RealTimeA
    
    EQ(std::string("RealTimeA"), rta.s_name);
    EQ(unsigned(0x340), rta.s_longoff);
    EQ(unsigned(1), rta.s_nLongs);
}
void setfiletest::vnmap_1()
{
    // name map should have as many entries as array:
    
    auto vars   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    auto map    = DDAS::SetFile::createVarNameMap(vars);
    
    EQ(vars.size(), map.size());
}
void setfiletest::vnmap_2()
{
    // Spot check some map entries:
    
    auto vars   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    auto map    = DDAS::SetFile::createVarNameMap(vars);
    
    auto& mcsra(map.at("ModCSRA"));
    auto& sfr(map.at("SlowFilterRange"));
    auto& cfdthr(map.at("CFDThresh"));
    
    EQ(std::string("ModCSRA"), mcsra.s_name);
    EQ(unsigned(1), mcsra.s_longoff);
    EQ(unsigned(1), mcsra.s_nLongs);
    
    EQ(std::string("SlowFilterRange"), sfr.s_name);
    EQ(unsigned(0xc), sfr.s_longoff);
    EQ(unsigned(1), sfr.s_nLongs);
    
    EQ(std::string("CFDThresh"), cfdthr.s_name);
    EQ(unsigned(0xf0), cfdthr.s_longoff);
    EQ(unsigned(16), cfdthr.s_nLongs);
    
}
void setfiletest::openset_1()
{
    // Open fails are thrown
    
    CPPUNIT_ASSERT_THROW(
        DDAS::SetFile::readSetFile("/no/such/file"),
        std::invalid_argument
    );
}
void setfiletest::openset_2()
{
    // Ok file works:
    std::pair<unsigned, uint32_t*> contents;
    CPPUNIT_ASSERT_NO_THROW(
        contents = DDAS::SetFile::readSetFile(m_setpath.c_str())
    );
    DDAS::SetFile::freeSetFile(contents.second);
}
void setfiletest::setread_1()
{
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    EQ(uint32_t(0), contents.second[0]);
    EQ(uint32_t(1), contents.second[1]);
    EQ(uint32_t(0x41), contents.second[2]);
    
    DDAS::SetFile::freeSetFile(contents.second);
    
}
void setfiletest::setwrite_1()
{
    // can't write open file -- exception.
    
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    CPPUNIT_ASSERT_THROW(
        DDAS::SetFile::writeSetFile(
            "/cannot/write/here", contents.first, contents.second
        ), std::invalid_argument
    );
    DDAS::SetFile::freeSetFile(contents.second);
}
void setfiletest::setwrite_2()
{
    // Can write to a good file:
    
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    char fname[100];
    strcpy(fname, "setfileXXXXXX");
    int fd = mkstemp(fname);
    ASSERT(fd >= 0);
    close(fd);
    
    CPPUNIT_ASSERT_NO_THROW(
        DDAS::SetFile::writeSetFile(
            fname, contents.first, contents.second
        )
    );
    unlink(fname);
    DDAS::SetFile::freeSetFile(contents.second);
}
void setfiletest::setwrite_3()
{
    // What we write is faithful:
    
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    
    char fname[100];
    strcpy(fname, "setfileXXXXXX");
    int fd = mkstemp(fname);
    ASSERT(fd >= 0);
    close(fd);
    DDAS::SetFile::writeSetFile(fname, contents.first, contents.second);
    
    auto copy = DDAS::SetFile::readSetFile(fname);
    
    EQ(contents.first, copy.first);
    EQ(0, memcmp(contents.second, copy.second, contents.first));
    DDAS::SetFile::freeSetFile(contents.second);
    DDAS::SetFile::freeSetFile(copy.second);
    unlink(fname);
}

void setfiletest::array_1()
{
    // populateSetFileArray makes a properly sized array.
    
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    auto varmap   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    
    auto arr      =
        DDAS::SetFile::populateSetFileArray(
            contents.first, contents.second, varmap
        );
        
    EQ(varmap.size(), arr.size());
    DDAS::SetFile::freeSetFile(contents.second);

}
void setfiletest::array_2()
{
    // Spot check contents of unpacked array:
    
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    auto varmap   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    
    auto arr      =
        DDAS::SetFile::populateSetFileArray(
            contents.first, contents.second, varmap
        );

    // Some spot check entries:
    
    auto& mcsra(arr[1]);
    auto& hostio(arr[0xf]);

    auto p = arr.end();
    --p;                          //U30.
    --p;                          // AutoTau.
    auto& autotau(*p);
    
    // Ok check them out.. names, sizes and values:
    
    EQ(std::string("ModCSRA"), mcsra.s_desc.s_name);
    EQ(unsigned(1), mcsra.s_desc.s_nLongs);
    EQ(uint32_t(1), mcsra.s_value);
    
    // HotIO - a vector:
    
    EQ(std::string("HostIO"), hostio.s_desc.s_name);
    EQ(unsigned(16), hostio.s_desc.s_nLongs);
    for(int i =0; i < 16; i++) {
        EQ(uint32_t(0), hostio.s_values[i]);
    }
    
    // autotau is also per channel
    //
    EQ(std::string("AutoTau"), autotau.s_desc.s_name);
    EQ(unsigned(16), hostio.s_desc.s_nLongs);
    for (int i = 0; i < 16; i++) {
        EQ(uint32_t(0), autotau.s_values[i]);
    }
    DDAS::SetFile::freeSetFile(contents.second);
    
    
}
void setfiletest::sfmap_1()
{
    // Set file map should have same # elements as the varfile.
    
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    auto varmap   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    auto setfile  =
        DDAS::SetFile::populateSetFileMap(
            contents.first, contents.second, varmap
        );
    EQ(varmap.size(), setfile.size());
    DDAS::SetFile::freeSetFile(contents.second);
    
}
void setfiletest::sfmap_2()
{
    // We should be able to find every var file entry in the map:
    
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    auto varmap   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    auto setfile  =
        DDAS::SetFile::populateSetFileMap(
            contents.first, contents.second, varmap
        );
    for (int i = 0; i < varmap.size(); i++) {
        DDAS::Variable v;
        CPPUNIT_ASSERT_NO_THROW(v = setfile.at(varmap[i].s_name));
        EQ(varmap[i].s_name , v.s_desc.s_name);
        EQ(varmap[i].s_longoff, v.s_desc.s_longoff);
        EQ(varmap[i].s_nLongs, v.s_desc.s_nLongs);
    }
    DDAS::SetFile::freeSetFile(contents.second);
    
}
void setfiletest::sfmap_3()
{
    // Spot check some map values:
    
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    auto varmap   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    auto setfile  =
        DDAS::SetFile::populateSetFileMap(
            contents.first, contents.second, varmap
        );
    
    auto& mcsrb(setfile.at("ModCSRB"));
    auto& ccsra(setfile.at("ChanCSRa"));
    auto& gainDAC(setfile.at("GainDAC"));
    
    // We know the descriptors are all ok from sfmap_2
    
    EQ(uint32_t(0x41), mcsrb.s_value);
    for (int i =0; i < 16; i++) {
        EQ(uint32_t(0x24), ccsra.s_values[i]);
        EQ(uint32_t(0x69ff), gainDAC.s_values[i]);
    }


    DDAS::SetFile::freeSetFile(contents.second);
}
void setfiletest::sfmap_4()
{
    // setfile map from setfile array -- should be same size as array:
    
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    auto varmap   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    auto setarr   =
        DDAS::SetFile::populateSetFileArray(
            contents.first, contents.second, varmap
        );
    auto setmap = DDAS::SetFile::populateSetFileMap(setarr);
    
    EQ(setarr.size(), setmap.size());
    
    
    DDAS::SetFile::freeSetFile(contents.second);
}
void setfiletest::sfmap_5()
{
    // setfile map from setfile array -- should be same size as array:
    
    auto contents = DDAS::SetFile::readSetFile(m_setpath.c_str());
    auto varmap   = DDAS::SetFile::readVarFile(m_varpath.c_str());
    auto setarr   =
        DDAS::SetFile::populateSetFileArray(
            contents.first, contents.second, varmap
        );
    auto setmap = DDAS::SetFile::populateSetFileMap(setarr);
    
    // Every setarr element should have a corresponding setmap
    // element and the contents should be equal:
    
    for (int i =0; i < setarr.size(); i++) {
        auto& s(setarr[i]);
        DDAS::Variable v;
        CPPUNIT_ASSERT_NO_THROW(
            v = setmap.at(s.s_desc.s_name)
        );                                 // Exists.
        // Equal:
        
        EQ(s.s_desc.s_name, v.s_desc.s_name);
        EQ(s.s_desc.s_longoff, v.s_desc.s_longoff);
        EQ(s.s_desc.s_nLongs, v.s_desc.s_nLongs);
        if (s.s_desc.s_nLongs == 1) {
            EQ(s.s_value, v.s_value);
        } else {
            for (int i =0; i < s.s_desc.s_nLongs; i++) {
                EQ(s.s_values[i], v.s_values[i]);
            }
        }
    }
    
    DDAS::SetFile::freeSetFile(contents.second);
}