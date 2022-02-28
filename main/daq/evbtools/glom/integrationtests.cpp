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

/** @file:  integrationtests.cpp
 *  @brief: Tests for glom as a whole.
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <DataFormat.h>
#include <time.h>
#include <string.h>
#include <fragment.h>

static  const char* argv[] = {
    "./glom", "--dt=100", "--timestamp-policy=earliest", "--sourceid=10",
    "--maxfragments=100", nullptr
};
static const char* glom="./glom";

class integrationtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(integrationtest);
    CPPUNIT_TEST(nothing_1);
    CPPUNIT_TEST(begin_1);
    CPPUNIT_TEST(scaler_1);
    CPPUNIT_TEST_SUITE_END();

protected:
    void nothing_1();
    void begin_1();
    void scaler_1();
    
private:
    pid_t m_glomPid;
    int m_stdinpipe[2];         // glom reads 0, we write 1
    int m_stdoutpipe[2];        //glom writes 1 we read 0.
public:
    void setUp() {
        m_glomPid = startGlom();
    }
    void tearDown() {
        close(m_stdinpipe[1]);         // This EOF will klll off glom.
        close(m_stdoutpipe[0]);
        if (m_glomPid > 0) {
            int exitStatus;
            EQ(waitpid(m_glomPid, &exitStatus, 0), m_glomPid);
        }
            
    }
private:
    pid_t startGlom();
    ssize_t writeGlom(const void* pBuffer, size_t nBytes);
    ssize_t readGlom(void* pBuffer, size_t nBytes);
    
};
// Write glom - must succeed or fail on one shot:

ssize_t
integrationtest::writeGlom(const void* pBuffer, size_t nBytes)
{
    return write(m_stdinpipe[1], pBuffer, nBytes);
}
ssize_t
integrationtest::readGlom(void* pBuffer, size_t nBytes)
{
    return read(m_stdoutpipe[0], pBuffer, nBytes);
}


/**
 * startGlom
 *    Starts the glom program off with the arguments in argv:
 * @return pid_t - the PID of glom.
 */
pid_t
integrationtest::startGlom()
{
    ASSERT(pipe2(m_stdinpipe, O_DIRECT) == 0);
    ASSERT(pipe2(m_stdoutpipe, 0) == 0);
    pid_t pid = fork();
    ASSERT(pid >= 0);
    if (pid > 0) {                  // parent
        close(m_stdinpipe[0]);
        close(m_stdoutpipe[1]);
        return pid;
    } else {                       // child
        // m_stdinpipe[0] is our stdin.

        
        close(STDIN_FILENO);
        close(m_stdinpipe[1]);
        int status = dup2(m_stdinpipe[0], STDIN_FILENO);
        if (status == -1) {
            perror("Glom dup of pipe[0] -> stdin failed");
            exit(EXIT_FAILURE);
        }
        // stdoutpipe[1] becomes stdout:
        
        close(STDOUT_FILENO);
        close(m_stdoutpipe[0]);
        status = dup2(m_stdoutpipe[1], STDOUT_FILENO);
        if (status == -1) {
            perror("Glom dup of pipe[1] -> stdout failed");
            exit(EXIT_FAILURE);
        }
        // Exec glom.
        
        status = execv(glom, const_cast<char**>(argv));
        if (status == -1) {
            perror("Could not exec glom");
        }
    }
    return pid;
}

CPPUNIT_TEST_SUITE_REGISTRATION(integrationtest);

// closing Gloms input makes glom exit sending nothing.
void integrationtest::nothing_1()
{
    close(m_stdinpipe[1]);   // EOF to glom's stin.
    uint8_t buffer[1024];
    ssize_t n = read(m_stdoutpipe[0], buffer, sizeof(buffer));
    if (n < 0) {
        perror("Failed read");
    }
    EQ(sizeof(DataFormat), size_t(n));
    pDataFormat p = reinterpret_cast<pDataFormat>(buffer);
    EQ(uint32_t(sizeof(DataFormat)),p->s_header.s_size);
    EQ(RING_FORMAT, p->s_header.s_type);
    EQ(FORMAT_MAJOR, p->s_majorVersion);
    EQ(FORMAT_MINOR, p->s_minorVersion);
    
    int status;
    EQ(m_glomPid, waitpid(m_glomPid, &status, 0));
    
    m_glomPid = 0;
}
// Sending a begin run at glom should give a format and begin run back.
// We've already confirmed the format comes

void integrationtest::begin_1()
{
    StateChangeItem begin;
    DataFormat      format;
    
    // Should be able to get the data format right away:
    
    ssize_t n = readGlom(&format, sizeof(format));
    EQ(sizeof(format), size_t(n));
    
    // Now format and produce the begin run item....we're going to not
    // say this is a barrier to avoid  glom parameters record:
    
    
    uint32_t itemsize =
        sizeof(RingItemHeader) + sizeof(BodyHeader) + sizeof(StateChangeItemBody);
    begin.s_header.s_type = BEGIN_RUN;
    begin.s_header.s_size = itemsize;
    begin.s_body.u_hasBodyHeader.s_bodyHeader.s_size = sizeof(BodyHeader);
    begin.s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp = 0;
    begin.s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId = 1; // gets overwritten.
    begin.s_body.u_hasBodyHeader.s_bodyHeader.s_barrier = 0;  // cheat.
    pStateChangeItemBody pBody = &begin.s_body.u_hasBodyHeader.s_body;
    pBody->s_runNumber = 12;
    pBody->s_timeOffset = 0;
    pBody->s_Timestamp = time(nullptr);
    pBody->s_offsetDivisor = 1;
    pBody->s_originalSid = 1;   // Not overwritten.
    strcpy(pBody->s_title, "A title string");
    
    // Need a fragment header for this:
    
    EVB::FragmentHeader h;
    h.s_timestamp = 0;
    h.s_sourceId = 1;
    h.s_barrier = 0;
    h.s_size = itemsize;
    
    // Write the flat fragment.
    
    writeGlom(&h, sizeof(h));
    writeGlom(&begin, itemsize);  // Includes a flush.
    
    uint8_t buffer[1000];
    memset(buffer, 0, sizeof(buffer));
    n = readGlom(buffer, sizeof(buffer));
    EQ(itemsize, uint32_t(n));
    
    // The result should be a BEGIN_RUN item with the source id rewritten:
    
    pStateChangeItem p = reinterpret_cast<pStateChangeItem>(buffer);
    
    EQ(begin.s_header.s_type, p->s_header.s_type);
    EQ( begin.s_header.s_size, p->s_header.s_size);
    EQ( begin.s_body.u_hasBodyHeader.s_bodyHeader.s_size, p->s_body.u_hasBodyHeader.s_bodyHeader.s_size );
    EQ(begin.s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp, p->s_body.u_hasBodyHeader.s_bodyHeader.s_timestamp);
    EQ(uint32_t(10), p->s_body.u_hasBodyHeader.s_bodyHeader.s_sourceId);
    EQ(begin.s_body.u_hasBodyHeader.s_bodyHeader.s_barrier, p->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
    
    auto pB = &(p->s_body.u_hasBodyHeader.s_body);
    EQ(pBody->s_runNumber, pB->s_runNumber);
    EQ(pBody->s_timeOffset, pB->s_timeOffset);
    EQ(pBody->s_Timestamp, pB->s_Timestamp);
    EQ(pBody->s_offsetDivisor, pB->s_offsetDivisor);
    EQ(pBody->s_originalSid, pB->s_originalSid);
    EQ(0, strcmp(pBody->s_title, pB->s_title));

    
}
// Sending a scaler item immediately gives me a scaler item back:
// Note that oob also sends  a sclaer request.
void integrationtest::scaler_1()
{
    // This declaration is a bit odd - it provides storage for the
    // scalers with body header as well as 32 actual scalers.
    // Doing it in this way means we don't have the variable sized
    // union that ScalerItem has.
#pragma packed(push, 1)
    struct Scaler {
        RingItemHeader  s_header;
        BodyHeader      s_bodyHeader;
        ScalerItemBody  s_body;
        uint32_t        scalers[32];
    };
#pragma packed(pop)
    Scaler s;                                // This is what we'll send:
    s.s_header.s_type = PERIODIC_SCALERS;
    s.s_header.s_size = sizeof(struct Scaler);
    s.s_bodyHeader.s_size = sizeof(BodyHeader);
    s.s_bodyHeader.s_timestamp = 0x123456789;
    s.s_bodyHeader.s_sourceId  = 1;          // Will be edited to 10.
    s.s_bodyHeader.s_barrier   = 0;
    s.s_body.s_intervalStartOffset = 0;     // first 2 seconds of the run.
    s.s_body.s_intervalEndOffset = 2;
    s.s_body.s_timestamp = time(nullptr);
    s.s_body.s_intervalDivisor = 1;
    s.s_body.s_scalerCount = 32;
    s.s_body.s_isIncremental=1;
    s.s_body.s_originalSid = s.s_bodyHeader.s_sourceId;
    for (int i = 0; i < 32; i++) {
        s.scalers[i] = i*10;
    }
    EVB::FragmentHeader h;
    h.s_timestamp = 0x123456789;
    h.s_size      = sizeof(struct Scaler);
    h.s_sourceId  = s.s_bodyHeader.s_sourceId;
    h.s_barrier   = 0;
    
    // send glom the fragment header and the scaler item:
    
    writeGlom(&h, sizeof(h));
    writeGlom(&s, sizeof(s));
    
    // Read the result, which should be a format item and a scaler item:
    
    DataFormat      format;
    ssize_t n = readGlom(&format, sizeof(format));
    EQ(sizeof(format), size_t(n));
    
    //
    
    uint8_t buffer[1000];
    memset(buffer, 0, sizeof(buffer));    // No coinidences
    n = readGlom(buffer, sizeof(buffer));
    EQ(sizeof(Scaler), size_t(n));
    
    // The only things that should differ are the sourceid in the body header:
    
    Scaler* p = reinterpret_cast<Scaler*>(buffer);
    EQ(0, memcmp(&(s.s_header), &(p->s_header), sizeof(RingItemHeader)));
    
    EQ(s.s_bodyHeader.s_size, p->s_bodyHeader.s_size);
    EQ(uint32_t(10), p->s_bodyHeader.s_sourceId);
    EQ(s.s_bodyHeader.s_timestamp, p->s_bodyHeader.s_timestamp);
    EQ(s.s_bodyHeader.s_barrier, p->s_bodyHeader.s_barrier);
    
    EQ(0, memcmp(&s.s_body, &p->s_body, sizeof(ScalerItemBody)));
    EQ(0, memcmp(s.scalers, p->scalers, 32*sizeof(uint32_t)));
}
