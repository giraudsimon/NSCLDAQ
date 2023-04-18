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

/** @file:  sedittests.cpp
 *  @brief: SetFileEditor tests.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"

#include "SetFileEditor.h"
#include "SetFile.h"

#include <string>
#include <stdexcept>
#include <sstream>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>



const std::string srcdir(SOURCE_DIR);
const std::string setfile("crate_1.set");

class sedittest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(sedittest);
    CPPUNIT_TEST(construct_1);
    CPPUNIT_TEST(construct_2);
    CPPUNIT_TEST(construct_3);
    
    CPPUNIT_TEST(get_1);
    CPPUNIT_TEST(get_2);
    CPPUNIT_TEST(get_3);
    CPPUNIT_TEST(get_4);
    
    CPPUNIT_TEST(getchan_1);
    CPPUNIT_TEST(getchan_2);
    CPPUNIT_TEST(getchan_3);
    CPPUNIT_TEST(getchan_4);
    
    CPPUNIT_TEST(set_1);
    CPPUNIT_TEST(set_2);
    CPPUNIT_TEST(set_3);
    CPPUNIT_TEST(set_4);
    
    CPPUNIT_TEST(setchan_1);
    CPPUNIT_TEST(setchan_2);    
    CPPUNIT_TEST(setchan_3);    
    CPPUNIT_TEST(setchan_4);    
    
    CPPUNIT_TEST(persist_1);
    CPPUNIT_TEST_SUITE_END();
    
    
private:
    std::string m_setfile;
public:
    void setUp() {
        std::string srcfile = SOURCE_DIR;
        srcfile += "/";
        srcfile += setfile;
        
        m_setfile = "copy_";
        m_setfile += setfile;
        
        std::string copyCommand = "cp ";
        copyCommand += srcfile;
        copyCommand += " ";
        copyCommand += m_setfile;
        
        system(copyCommand.c_str());
        
    }
    void tearDown() {
        unlink(m_setfile.c_str());
    }
protected:
    void construct_1();
    void construct_2();
    void construct_3();
    
    void get_1();
    void get_2();
    void get_3();
    void get_4();
    
    void getchan_1();
    void getchan_2();
    void getchan_3();
    void getchan_4();
    
    void set_1();
    void set_2();
    void set_3();
    void set_4();
    
    void setchan_1();
    void setchan_2();
    void setchan_3();
    void setchan_4();
    
    void persist_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(sedittest);

void sedittest::construct_1()
{
    // Bad filename results in std::Invalid_argument I think
    
    CPPUNIT_ASSERT_THROW(
        new DDAS::SetFileEditor("/this/is/a/bad/file.set"),
        std::invalid_argument
    );
}
void sedittest::construct_2()
{
    // Good filename, open is successful.
    
    DDAS::SetFileEditor* pEditor;
    CPPUNIT_ASSERT_NO_THROW(
        pEditor = new DDAS::SetFileEditor(m_setfile.c_str())
    );
}
void sedittest::construct_3()
{
    // File exists but is not large enough.
    
    char tempName[100];
    strcpy(tempName, "badSetFileXXXXXX");
    int fd = mkstemp(tempName);
    if (fd < 0) {
        int e = errno;
        std::stringstream msg;
        msg << "Unable to open temporary 'bad' setfile: "
            << strerror(e);
        CPPUNIT_FAIL(msg.str());  
    }
    close(fd);              // zero length is too small.
    CPPUNIT_ASSERT_THROW(
        new DDAS::SetFileEditor(tempName),
        std::invalid_argument
    );
    int s = unlink(tempName);
    if (s < 0) {
        int e = errno;
        std::stringstream msg;
        msg << " Failed to remove temp file: " << tempName
            << " : " << strerror(e);
        CPPUNIT_FAIL(msg.str());
    }
}

void sedittest::get_1()
{
    // Get ModCSRB from each slot
    
    std::vector<uint32_t> csrbs =
        {
            65, 1, 1, 1, 1, 1, 1, 1, 1, 1,
            2, 2, 2, 2, 2, 2, 2,
            2, 2, 2, 2, 2, 2, 2
        };
    DDAS::SetFileEditor e(m_setfile.c_str());
    for (int i =0; i < csrbs.size(); i++) {
        EQ(csrbs[i], e.get(i+2, "ModCSRB"));
    }
}
void sedittest::get_2()
{
    // Using 'get' to get a per channel param fails:
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    CPPUNIT_ASSERT_THROW(
        e.get(2, "ChanCSRa"),
        std::invalid_argument
    );
}
void sedittest::get_3()
{
    // Error to get an invalid parameter
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    CPPUNIT_ASSERT_THROW(
        e.get(2, "junk"),
        std::invalid_argument
    );
}
void sedittest::get_4()
{
    // Error to use a bad slot.
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    CPPUNIT_ASSERT_THROW(
        e.get(100, "ModCSRA"),
        std::range_error
    );
}
void sedittest::getchan_1()
{
    // Getting a module param using getChanPar fails:
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    CPPUNIT_ASSERT_THROW(
        e.getChanPar(2, "ModCSRB"),
        std::invalid_argument
    );
}
void sedittest::getchan_2()
{
    // Get the channel CSRa from all modules:
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    std::vector<uint32_t> csrs = {
        36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
        420, 420, 420, 420, 420, 420, 420, 420,
        420, 420, 420, 420, 420, 420
    };
    for (int s =0; s < 24; s++) {
        auto v = e.getChanPar(s+2, "ChanCSRa");
        for (int c =0; c < 16; c++) {
            EQ(csrs[s], v[c]);
        }
    }
}
void sedittest::getchan_3()
{
    // error to get an invalid parameter.
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    CPPUNIT_ASSERT_THROW(
        e.getChanPar(2, "illegal"),
        std::invalid_argument
    );
}
void sedittest::getchan_4()
{
    // Error to get nonexistent slot.
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    CPPUNIT_ASSERT_THROW(
        e.getChanPar(28, "ModCSRa"),
        std::range_error
    );
}
void sedittest::set_1()
{
    // Set the slow filter range on every even channel to 2
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    for (int i = 2; i < 26; i +=2) {
        e.set(i, "SlowFilterRange", 2);
    }
    
    for (int i = 2; i < 26;  i += 2) {
        uint32_t sfr = e.get(i, "SlowFilterRange");
        uint32_t v   = ((i % 2) == 0) ? 2 : 3;
        EQ(v, sfr);
    }
}
void sedittest::set_2()
{
    // Error to set a per channel:
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    CPPUNIT_ASSERT_THROW(
        e.set(2, "ChanCSRa", 1),
        std::invalid_argument
    );
}
void sedittest::set_3()
{
    // Error to use a nonexistent name.
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    CPPUNIT_ASSERT_THROW(
        e.set(2, "nonesuch", 1),
        std::invalid_argument
    );
}
void sedittest::set_4()
{
    // Error to use a bad slot.
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    CPPUNIT_ASSERT_THROW(
        e.set(30, "ModCSRA", 0),
        std::range_error
    );
}

void sedittest::setchan_1()
{
    // Set the gain dacs to increments of 0x10 for running for
    // each chan.
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    
    uint32_t gdac = 0;
    for (int s = 2; s < 26; s++) {
        uint32_t dacs[16];
        for (int i =0; i < 16; i++) {
            dacs[i] = gdac;
            gdac += 0x10;
        }
        e.setChanPar(s, "GainDAC", dacs);
    }
    // Check:
    
    gdac = 0;
    for (int s = 2; s < 26; s++ ) {
        auto v = e.getChanPar(s, "GainDAC");
        for (int i = 0; i < 16; i++) {
            EQ(gdac, v[i]);
            gdac += 0x10;
        }
    }
}
void sedittest::setchan_2()
{
    // Can't use a module parameter.
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    uint32_t* p(nullptr);
    CPPUNIT_ASSERT_THROW(
        e.setChanPar(2, "ModCSRA", p),
        std::invalid_argument
    );
}
void sedittest::setchan_3()
{
    // Can't use an unknown name.
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    uint32_t* p(nullptr);
    CPPUNIT_ASSERT_THROW(
        e.setChanPar(2, "junk-test", p),
        std::invalid_argument
    );
}
void sedittest::setchan_4()
{
    // Can't use a bad slot number
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    uint32_t* p(nullptr);
    CPPUNIT_ASSERT_THROW(
        e.setChanPar(27, "ChanCSRa", p),
        std::range_error
    );
}
void sedittest::persist_1()
{
    // Changes persist after destruction:

    uint32_t v[16];
    for (int i = 0; i < 16; i++) {
        v[i] = i;
    }
    
    {
        DDAS::SetFileEditor e(m_setfile.c_str());
        e.set(2, "ModCSRA", 0xaaaaaaaa);
        
        e.setChanPar(25, "ChanCSRa", v);
    }                           // Unmap and flus
    
    DDAS::SetFileEditor e(m_setfile.c_str());
    EQ(uint32_t(0xaaaaaaaa), e.get(2, "ModCSRA"));
    
    auto c = e.getChanPar(25, "ChanCSRa");
    for (int i =0; i < c.size(); i++) {
        EQ(v[i], c.at(i));
    }
}