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

/** @file:  
 *  @brief: 
 */
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <sstream>
#include <errno.h>
#include <stdexcept>
#include <sys/types.h>
#include <signal.h>
#include <wait.h>
#include <time.h>
#include <fragment.h>
#include <sys/uio.h>
#include <io.h>
#include <stdint.h>
#include <DataFormat.h>


class barriertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(barriertest);
    CPPUNIT_TEST(begin_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int  m_glomStdin;
    int  m_glomInput;
    
    int  m_glomOutput;
    int  m_glomStdout;
    int  m_glomPid;
public:
    void setUp() {
        int pipefds[2];
        if (pipe(pipefds)) {
            int e = errno;
            std::stringstream msg;
            msg << "Could not create pipes for glom: "
                << strerror(e) << std::endl;
            throw std::logic_error(msg.str());
        }
        m_glomStdin  = pipefds[0];
        m_glomInput  = pipefds[1];
        
        if (pipe(pipefds)) {
            int e = errno;
            std::stringstream msg;
            msg << "Could not create pipes for glom: "
                << strerror(e) << std::endl;
            throw std::logic_error(msg.str());
        }
        m_glomOutput = pipefds[0];
        m_glomStdout = pipefds[1];
        
        // Create the glom command. We do this here in case
        // there are errors so we can report them and never fork:
        
        std::string glomcmd;
        const char* glomdir = getenv("GLOMDIR");
        if (!glomdir) {
            throw std::logic_error("Could not read env variable GLOMDIR");
        }
        glomcmd = glomdir;
        glomcmd += "/";
        glomcmd += "glom";
        
        m_glomPid    = fork();
        if (m_glomPid == 0) {
            close(m_glomInput);
            close(m_glomOutput);
            // Child process...
            // set up our stdin to be glomInput
            // and stdout to be glomOutput then
            // start glom .
            
            dup2(m_glomStdin, STDIN_FILENO);
            dup2(m_glomStdout, STDOUT_FILENO);
            execl(glomcmd.c_str(), glomcmd.c_str(), "--dt=100", nullptr);
            exit(EXIT_SUCCESS);
        } else {
            close(m_glomStdin);
            close(m_glomStdout);
        }
    }
    void tearDown() {
        close(m_glomInput);
        close(m_glomOutput);
        
        // The above should have made glom exit -but- just to make
        // "assurance double-sure":
        
        kill(m_glomPid, SIGKILL);  // Don't check error -- could alraeady be gone.
        int status;
        waitpid(m_glomPid, &status, 0);
    }
protected:
    void begin_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(barriertest);

void barriertest::begin_1()
{
    // A pair of begin run records results in a pair of begin
    // run records with only the first one having the barrier set.
    // Note - using a barrier type of 5 is a trick to prevent the
    // glom parameters record from being output.
    
    time_t now = time(nullptr);
    pStateChangeItem pItem = formatTimestampedStateChange(
        0x1234567890, 1, 5, now, 0, 124,  1,  "Testing", BEGIN_RUN
    );
    EVB::FragmentHeader frag;
    frag.s_timestamp = 0x1234567890;
    frag.s_sourceId  = 1;
    frag.s_size      = pItem->s_header.s_size;
    frag.s_barrier   = 1;
    
    // Submit two of these to glom.
    
    iovec vec[2];
    vec[0].iov_len = sizeof(frag);
    vec[0].iov_base = &frag;
    
    vec[1].iov_len = frag.s_size;
    vec[1].iov_base = pItem;
    
    io::writeDataV(m_glomInput, vec, 2);
    io::writeDataV(m_glomInput, vec, 2);
    free(pItem);
    
    // Read data back from glom.. Should be two state change
    // items.  The first will have a barrier in the body header.
    // the second should not.
    
    uint8_t inbound[frag.s_size];
    pStateChangeItem p = reinterpret_cast<pStateChangeItem>(inbound);
    
    // First a data format record:
    
    DataFormat fmt;
    size_t fsize = io::readData(m_glomOutput, &fmt, sizeof(fmt));
    EQ(fsize, sizeof(fmt));
    EQ(FORMAT_MAJOR, fmt.s_majorVersion);
    EQ(FORMAT_MINOR, fmt.s_minorVersion);
    
    // First begin run:
    
    size_t n = io::readData(m_glomOutput, inbound, frag.s_size);
    EQ(size_t(frag.s_size), n);
    EQ(uint32_t(5), p->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
    
    // Second begin run:
    
    n = io::readData(m_glomOutput, inbound, frag.s_size);
    EQ(size_t(frag.s_size), n);
    EQ(uint32_t(0), p->s_body.u_hasBodyHeader.s_bodyHeader.s_barrier);
    
    
    
}