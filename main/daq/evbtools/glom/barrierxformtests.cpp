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

class barriertest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(barriertest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    int  m_glomOutput;
    int  m_glomInput;
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
        m_glomOutput = pipefds[0];
        m_glomInput  = pipefds[1];
        
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
            // Child process...
            // set up our stdin to be glomInput
            // and stdout to be glomOutput then
            // start glom .
            
            dup2(m_glomInput, STDIN_FILENO);
            dup2(m_glomOutput, STDOUT_FILENO);
            execl(glomcmd.c_str(), "--dt=100", nullptr);
            exit(EXIT_SUCCESS);
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
    void test_1();
};

CPPUNIT_TEST_SUITE_REGISTRATION(barriertest);

void barriertest::test_1()
{
}