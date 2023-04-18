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

/** @file:  setfilewritertests.cpp
 *  @brief: Tests for DDAS::SetFileWriter
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <string>

#include "SetFileWriter.h"
#include "SetFileEditor.h"
#include "SetFileReader.h"
#include "XMLSettingsReader.h"
#include "ModuleSettings.h"
#include "CrateManager.h"

#include <config.h>
#include <config_pixie16api.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdexcept>
#include <stdio.h>
#include <errno.h>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <math.h>

const std::string TEST_FILEDIR=SOURCE_DIR;
const std::string XML_INPUT="bimdev_Module_00.xml";
const std::string VAR_FILE="test.var";
const std::string SET_FILE("crate_1.set");

class setfwrtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(setfwrtest);
    CPPUNIT_TEST(error_1);
    CPPUNIT_TEST(wrok_1);
    CPPUNIT_TEST(wrok_2);
    CPPUNIT_TEST(wrok_3);
    CPPUNIT_TEST_SUITE_END();
    
private:
    std::string m_setFileName;
    std::string m_varFile;
    DDAS::SetFileEditor*   m_Editor;
    DDAS::ModuleSettings  m_Settings;    
public:
    void setUp() {
        m_setFileName = copySetFile();
        m_varFile     = makePath(VAR_FILE);
        m_Editor      = new DDAS::SetFileEditor(m_setFileName.c_str());
        readXMLSettings();
    }
    void tearDown() {
        delete m_Editor;
        unlink(m_setFileName.c_str());
    }
protected:
    void error_1();
    
    void wrok_1();
    void wrok_2();
    void wrok_3();
private:
    std::string mkTempFile();
    std::string copySetFile();
    std::string makePath(const std::string& file);
    void readXMLSettings();
};

CPPUNIT_TEST_SUITE_REGISTRATION(setfwrtest);

// Private utilities:

/**
 * mkTempFile
 *    Make a temp file, close the associated fd and
 *    return the filename.
 * @return std::string
 */
std::string
setfwrtest::mkTempFile()
{
    static const char* fileTemplate  = "setfwrXXXXXX";
    char  fileName[200];
    strcpy(fileName, fileTemplate);
    int fd = mkstemp(fileName);
    if (fd < 0) {
        int e = errno;
        std::stringstream emsg;
        emsg << "Unable to create tempfile from: " << fileTemplate
            << " : " << strerror(e);
        throw std::logic_error(emsg.str());
    }
    close(fd);
    return std::string(fileName);
}
/**
 * copySetFile
 *   Copies the set file into a temp file which we use
 *   as output for our writer tests.
 * @return std::string - set file name we can use.
 */
std::string
setfwrtest::copySetFile()
{
    std::string infile = makePath(SET_FILE);
    std::string ofile  = mkTempFile();
    int fdi = open(infile.c_str(), O_RDONLY);
    if (fdi < 0) {
        int e = errno;
        std::stringstream emsg;
        emsg << "Unable to open setfile: " << infile
            << " : " << strerror(e);
        throw std::logic_error(emsg.str());
    }
    int fdo = creat(ofile.c_str(),  S_IRWXU);
    if (fdo < 0) {
        int e = errno;
        std::stringstream emsg;
        emsg << "Unable to create setfile copy: " << ofile
            << " : " << strerror(e);
        throw std::logic_error(emsg.str());
    }
    // Copy:
    ssize_t n;
    uint8_t buffer[8192];
    while ((n = read(fdi, buffer, sizeof(buffer))) > 0) {
        ssize_t stat = write(fdo, buffer, n);
        if (stat < 0) {
            int e = errno;
            std::stringstream emsg;
            emsg << "Write of setfile copy failed: " << strerror(e);
            throw std::logic_error(emsg.str());
        }
        EQMSG("Not all bytes copied in set file copy", n, stat);
    }
    if (n < 0) {
        int e = errno;
        std::stringstream emsg;
        emsg << "Set file copy read failed: " << strerror(e);
        throw std::logic_error(emsg.str());
    }
    close(fdi);
    close(fdo);
    
    return ofile;
}

/**
 * makePath
 *    Given a filename, make a path to it in the
 *    source directory.
 * @param file - the filename part of the path.
 * @return std::string.
 */
std::string
setfwrtest::makePath(const std::string& file)
{
    std::string result(TEST_FILEDIR);
    result += "/";
    result += file;
    return result;
}
/**
 * readXMLSettings
 *    Read the settings from the XML file. These are the
 *    things that go to the setfile.
 * @note the settings go to m_Settings.
 */
void
setfwrtest::readXMLSettings()
{
    std::string xmlFile = makePath(XML_INPUT);
    DDAS::XMLSettingsReader r(xmlFile.c_str());
    m_Settings = r.get();
}

////////////////////////////////////////////////////////////////
// tests:
void setfwrtest::error_1()
{
    // Invalid MHz
    
    DDAS::SetFileWriter w(*m_Editor, 2, 125);
    CPPUNIT_ASSERT_THROW(
       w.write(m_Settings),
       std::invalid_argument
    );
}
void setfwrtest::wrok_1()
{
    // We can write the file ok if we give a good MHz:
    
    DDAS::SetFileWriter w(*m_Editor, 2, 250);
    CPPUNIT_ASSERT_NO_THROW(w.write(m_Settings));
}
void setfwrtest::wrok_2()
{
    // Write and read -- the module level stuff should match:
    
    DDAS::SetFileWriter w(*m_Editor, 2, 250);
    w.write(m_Settings);
    
    std::string vfile = DDAS::CrateManager::getVarFile(250);
    DDAS::SetFileReader r(m_setFileName.c_str(), vfile.c_str(), 250, 2);
    DDAS::ModuleSettings rd = r.get();
    
    EQMSG("csra", m_Settings.s_csra, rd.s_csra);
    EQMSG("csrb", m_Settings.s_csrb, rd.s_csrb);
    EQMSG("format", m_Settings.s_format, rd.s_format);
    EQMSG("maxevents", m_Settings.s_maxEvents, rd.s_maxEvents);
    EQMSG("synchWait", m_Settings.s_synchWait, rd.s_synchWait);
    EQMSG("insynch", m_Settings.s_inSynch, rd.s_inSynch);
    EQMSG("slow filter range", m_Settings.s_SlowFilterRange, rd.s_SlowFilterRange);
    EQMSG("fast filter range", m_Settings.s_FastFilterRange, rd.s_FastFilterRange);
    EQMSG("fastrg bkpln ena", m_Settings.s_FastTrgBackPlaneEnables, rd.s_FastTrgBackPlaneEnables);
    EQMSG("trgcfg0", m_Settings.s_trigConfig0, rd.s_trigConfig0);
    EQMSG("trgcfg1", m_Settings.s_trigConfig1, rd.s_trigConfig1);
    EQMSG("trgcfg2", m_Settings.s_trigConfig2, rd.s_trigConfig2);
    EQMSG("trgcfg3", m_Settings.s_trigConfig3, rd.s_trigConfig3);
    EQMSG("Host Rtprst", m_Settings.s_HostRtPreset, rd.s_HostRtPreset);
    
       
}
void setfwrtest::wrok_3()
{
    // Write and read the channel level stuff should match.

    // Write and read -- the module level stuff should match:
    
    DDAS::SetFileWriter w(*m_Editor, 2, 250);
    w.write(m_Settings);
    
    std::string vfile = DDAS::CrateManager::getVarFile(250);
    DDAS::SetFileReader r(m_setFileName.c_str(), vfile.c_str(), 250, 2);
    DDAS::ModuleSettings rd = r.get();
    
    for (int i  = 0; i < 16; i++) {
        EQMSG("trrise", m_Settings.s_triggerRiseTime[i], rd.s_triggerRiseTime[i]);
        EQMSG("trflat", m_Settings.s_triggerFlattop[i], rd.s_triggerFlattop[i]);
        EQMSG("trthresh", m_Settings.s_triggerThreshold[i], rd.s_triggerThreshold[i]);
        EQMSG("erise", m_Settings.s_energyRiseTime[i], rd.s_energyRiseTime[i]);
        EQMSG("eflat", m_Settings.s_energyFlattop[i], rd.s_energyFlattop[i]);
        EQMSG("tau", m_Settings.s_tau[i], rd.s_tau[i]);
        EQMSG("tracel", m_Settings.s_traceLength[i], rd.s_traceLength[i]);
        EQMSG("traced", m_Settings.s_traceDelay[i], rd.s_traceDelay[i]);
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "Voffset", fabs(m_Settings.s_vOffset[i]), fabs(rd.s_vOffset[i]),
            DAC_VOLTAGE_RANGE/(65535.0)
        );
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE(
            "xdt", m_Settings.s_Xdt[i], rd.s_Xdt[i], 0.2); // Diff in granularity.
        EQMSG("BlPercent", m_Settings.s_BaselinePct[i], rd.s_BaselinePct[i]);
        EQMSG("emin", m_Settings.s_Emin[i], rd.s_Emin[i]);
        EQMSG("binfactor", m_Settings.s_binFactor[i], rd.s_binFactor[i]);
        EQMSG("blavg", m_Settings.s_baselineAverage[i], rd.s_baselineAverage[i]);
        EQMSG("csra", m_Settings.s_chanCsra[i], rd.s_chanCsra[i]);
        EQMSG("csrb", m_Settings.s_chanCsrb[i], rd.s_chanCsrb[i]);
        EQMSG("blcut", m_Settings.s_blCut[i], rd.s_blCut[i]);
        EQMSG("ftrbacklen", m_Settings.s_fastTrigBackLen[i], rd.s_fastTrigBackLen[i]);
        EQMSG("cfddel", m_Settings.s_CFDDelay[i], rd.s_CFDDelay[i]);
        EQMSG("cfdsc", m_Settings.s_CFDScale[i], rd.s_CFDScale[i]);
        EQMSG("cfdthr", m_Settings.s_CFDThreshold[i], rd.s_CFDThreshold[i]);
        EQMSG("qlen0", m_Settings.s_QDCLen0[i], rd.s_QDCLen0[i]);
        EQMSG("qlen1", m_Settings.s_QDCLen1[i], rd.s_QDCLen1[i]);
        EQMSG("qlen2", m_Settings.s_QDCLen2[i], rd.s_QDCLen2[i]);
        EQMSG("qlen3", m_Settings.s_QDCLen3[i], rd.s_QDCLen3[i]);
        EQMSG("qlen4", m_Settings.s_QDCLen4[i], rd.s_QDCLen4[i]);
        EQMSG("qlen5", m_Settings.s_QDCLen5[i], rd.s_QDCLen5[i]);
        EQMSG("qlen6", m_Settings.s_QDCLen6[i], rd.s_QDCLen6[i]);
        EQMSG("qlen7", m_Settings.s_QDCLen7[i], rd.s_QDCLen7[i]);
        EQMSG("trgstretch", m_Settings.s_extTrigStretch[i], rd.s_extTrigStretch[i]);
        EQMSG("vstretch", m_Settings.s_vetoStretch[i], rd.s_vetoStretch[i]);
        EQMSG("mmaskL", m_Settings.s_multiplicityMaskL[i], rd.s_multiplicityMaskL[i]);
        EQMSG("mmaskH", m_Settings.s_multiplicityMaskH[i], rd.s_multiplicityMaskH[i]);
        EQMSG("extdellen", m_Settings.s_externDelayLen[i], rd.s_externDelayLen[i]);
        EQMSG("ftrgdelout", m_Settings.s_FTrigoutDelay[i], rd.s_FTrigoutDelay[i]);
        EQMSG("chtrigstr", m_Settings.s_chanTriggerStretch[i], rd.s_chanTriggerStretch[i]);
    }
}
