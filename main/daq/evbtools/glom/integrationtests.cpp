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

static  const char* argv[] = {
    "./glom", "--dt=100", "--timestamp-policy=first", "--sourceid=10",
    "--maxfragments=100", nullptr
};
static const char* glom="./glom";

class integrationtest : public CppUnit::TestFixture {
    CPPUNIT_TEST_SUITE(integrationtest);
    CPPUNIT_TEST(test_1);
    CPPUNIT_TEST_SUITE_END();
    
private:
    pid_t m_glomPid;
    int m_pipes[2];         // m_pipes[0] - data to glom glom m_pipes[1] output from glom.
public:
    void setUp() {
        m_glomPid = startGlom();
    }
    void tearDown() {
        close(m_pipes[0]);         // This EOF will klll off glom.
        close(m_pipes[1]);
        if (m_glomPid > 0) {
            int exitStatus;
            EQ(waitpid(m_glomPid, &exitStatus, 0), m_glomPid);
        }
            
    }
protected:
    void test_1();
private:
    pid_t startGlom();
};

/**
 * startGlom
 *    Starts the glom program off with the arguments in argv:
 * @return pid_t - the PID of glom.
 */
pid_t
integrationtest::startGlom()
{
    ASSERT(pipe2(m_pipes, O_DIRECT) == 0);
    pid_t pid = fork();
    ASSERT(pid >= 0);
    if (pid > 0) {                  // parent
        return pid;
    } else {                       // child
        // m_pipes[0] becomes stdin and
        // m_pipes[1] becomes stdout:
        
        int status = dup2(m_pipes[0], STDIN_FILENO);
        if (status == -1) {
            perror("Glom dup of pipe[0] -> stdin failed");
            exit(EXIT_FAILURE);
        }
        status = dup2(m_pipes[1], STDOUT_FILENO);
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

void integrationtest::test_1()
{
}