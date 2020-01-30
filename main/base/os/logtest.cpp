// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include <stdexcept>
#include <string>
#include <unistd.h>
#include <fstream>
#include <Asserts.h>

#include "NSCLDAQLog.h"

namespace daqlog {
extern void reset();  
}



class logtests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(logtests);
#ifdef HAVE_BOOST_LOG
  CPPUNIT_TEST(logb4file);
  CPPUNIT_TEST(error);
  CPPUNIT_TEST(info);
  CPPUNIT_TEST(Warning);
  CPPUNIT_TEST(Fatal);
  
  CPPUNIT_TEST(debugNoShow);
  CPPUNIT_TEST(debugShow);
  CPPUNIT_TEST(traceShow);
#endif
  CPPUNIT_TEST_SUITE_END();


private:
  std::string m_logFile;
public:
  void setUp() {
    char tplate[100];
    memset(tplate, 0, sizeof(tplate));
    strncpy(tplate, "logXXXXXX", sizeof(tplate)-1);
    int fd = mkstemp(tplate);
    if (fd < 0) {
      perror("setUp failed to make temp file.");
      exit(EXIT_FAILURE);
    }
    close(fd);
    m_logFile = tplate;
    daqlog::setLogFile(m_logFile.c_str());
  }
  void tearDown() {
    daqlog::reset();
    unlink(m_logFile.c_str());
  }
protected:
  void logb4file();
  void error();
  void info();
  void Warning();
  void Fatal();
  void debugNoShow();
  void debugShow();
  void traceShow();
};

static std::string
stripDate(std::string s)
{
  size_t closeParen = s.find_first_of(")");
  ASSERT(closeParen != std::string::npos);
  closeParen += 3;     // Skip ") "
  return s.substr(closeParen);
}

CPPUNIT_TEST_SUITE_REGISTRATION(logtests);

// Logging before setting the filename is a failure

void logtests::logb4file() {
  daqlog::reset();
  CPPUNIT_ASSERT_THROW(
    daqlog::error("Testing"), std::logic_error
  );
}

void logtests::error() {
  daqlog::error("Test log string");
  
  // The file should have a line that ends ") Error : Test log string"
  // Note ) is after the date string.
  
  std::ifstream logfile(m_logFile);
  std::string line;
  std::getline(logfile, line);
  
  line = stripDate(line);
  EQ(std::string("Error : Test log string"), line);
}

void logtests::info()
{
  daqlog::info("Informational message");
   
  std::ifstream logfile(m_logFile);
  std::string line;
  std::getline(logfile, line);
  
  line = stripDate(line);
  EQ(std::string("Info : Informational message"), line);
}
void logtests::Warning()
{
  daqlog::warning("A warning message");

  std::ifstream logfile(m_logFile);
  std::string line;
  std::getline(logfile, line);
  
  line = stripDate(line);
  EQ(std::string("Warning : A warning message"), line);
}
void logtests::Fatal()
{
  daqlog::fatal("A fatal error");
  
  std::ifstream logfile(m_logFile);
  std::string line;
  std::getline(logfile, line);
  
  line = stripDate(line);
  EQ(std::string("Fatal : A fatal error"), line);
}

// By default debug messages are filtered out.

void logtests::debugNoShow()
{
  daqlog::debug("Should produce no output");
  
  std::ifstream logfile(m_logFile);
  std::string line;
  std::getline(logfile, line);
  ASSERT(line.empty());
}

// But if we set the severity level to allow it we can see debugs:

void logtests::debugShow()
{
  daqlog::setLogLevel(daqlog::Trace);
  
  daqlog::debug("Should produce output");
  
  std::ifstream logfile(m_logFile);
  std::string line;
  std::getline(logfile, line);
  line = stripDate(line);
  EQ(std::string("Debug : Should produce output"),line);
}
// Trace - we also need to set the log level:

void logtests::traceShow()
{
  daqlog::setLogLevel(daqlog::Trace);
  
  daqlog::trace("Should produce output");
  
  std::ifstream logfile(m_logFile);
  std::string line;
  std::getline(logfile, line);
  line = stripDate(line);
  EQ(std::string("Trace : Should produce output"),line);
}