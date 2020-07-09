// Template for a test suite.

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#define private public
#include <CSinkFactory.h>
#undef private
#include <CFileSinkCreator.h>
#include <CSink.h>

#include <string.h>
#include <unistd.h>

using namespace std;

static const string FileType("file");
static const string Filename("test.log");
static const string Command("testing");

class FactoryTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FactoryTests);
  CPPUNIT_TEST(SplitTest);
  CPPUNIT_TEST(NullCreateTest);
  CPPUNIT_TEST(CreateTest);
  CPPUNIT_TEST_SUITE_END();


private:

public:
  void setUp() {
  }
  void tearDown() {
    // Force re-creation of the factory.
    
    delete CSinkFactory::m_pInstance;
    CSinkFactory::m_pInstance = nullptr;
  }
protected:
  void SplitTest();
  void NullCreateTest();
  void CreateTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(FactoryTests);

/*
Invoke SplitName on a file:name combination
*/
void FactoryTests::SplitTest() {
  char *(parts[2]);

  string descriptor(FileType);
  descriptor += ':';
  descriptor += Filename;
  
  char* pName = strdup(descriptor.c_str());
  CSinkFactory::SplitName(pName, parts);

  EQMSG("File type comparison", FileType, string(parts[0]));
  EQMSG("Filename comparison",  Filename, string(parts[1]));


  
  
}
/*
 Invoke Create with "file" "name"
*/
void
FactoryTests::NullCreateTest()
{
  CSink* pSink = CSinkFactory::getInstance()->Create(FileType, 
				      Command, Filename);
  EQMSG("Null sink", (CSink*)NULL, pSink);
}

/*
AddCreator  for a CFileSinkCreator
Create "file", "name"
1. Should get non null creator.
Create "zzzzz", "name"
2. should get null creator.
*/
void
FactoryTests::CreateTest()
{
  CSinkFactory::getInstance()->AddCreator(FileType, new CFileSinkCreator);

  CSink* pSink = CSinkFactory::getInstance()->Create(FileType,
				      Command, Filename);
  ASSERT(pSink);
  delete pSink;
  unlink(Filename.c_str());

  pSink = CSinkFactory::getInstance()->Create(string("zzzzz"), Command,
			       Filename);
  ASSERT(! pSink);
}
