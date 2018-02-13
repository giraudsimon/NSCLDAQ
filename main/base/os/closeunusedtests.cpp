// Template for a test suite.
#include <Asserts.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include <io.h>


class closeUnusedTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(closeUnusedTests);
  CPPUNIT_TEST(keepOpen);
  CPPUNIT_TEST(closeAFile);
  CPPUNIT_TEST(keepTempOpen);
  CPPUNIT_TEST_SUITE_END();


private:
  std::set<int> keepFiles;
public:
  void setUp() {
    keepFiles.insert(STDIN_FILENO);
    keepFiles.insert(STDOUT_FILENO);
    keepFiles.insert(STDERR_FILENO);
  }
  void tearDown() {
    keepFiles.clear();
  }
protected:
  void keepOpen();
  void closeAFile();
  void  keepTempOpen();
  
private:
  bool isOpen(int fd);
};

/**
 * isOpen returns true if the file descriptor passed in is open.
 */
bool
closeUnusedTests::isOpen(int fd)
{
  struct stat statbuf;
  return (fstat(fd, &statbuf) == 0);
}

CPPUNIT_TEST_SUITE_REGISTRATION(closeUnusedTests);


// Requested files should be kept open.
void closeUnusedTests::keepOpen() {
  // If we ask the system to keep a file (STDIO) it should keep them open.
  
  io::closeUnusedFiles(keepFiles);
  ASSERT(isOpen(STDIN_FILENO));
  ASSERT(isOpen(STDOUT_FILENO));
  ASSERT(isOpen(STDERR_FILENO));
}


// Ensure a file not in the set is closed.

void closeUnusedTests::closeAFile()
{
  FILE* pf = tmpfile();                     // Open a temp file.
  int fd = fileno(pf);
  io::closeUnusedFiles(keepFiles);
  
  ASSERT(!isOpen(fd));
}
// Adding a new file to the set makes it stay open:

void closeUnusedTests::keepTempOpen()
{
  FILE* pf = tmpfile();                     // Open a temp file.
  int fd = fileno(pf);
  keepFiles.insert(fd);
  
  io::closeUnusedFiles(keepFiles);
  
  ASSERT(isOpen(fd));
  
  close(fd);
}