// Template for a test suite.


#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/Asserter.h>
#include "Asserts.h"
#include <XXUSBConfigurableObject.h>
#include <algorithm>

using namespace std;



class configtest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(configtest);
  CPPUNIT_TEST(construct);
  CPPUNIT_TEST(addstring);
  CPPUNIT_TEST(config);
  CPPUNIT_TEST(cget);
  CPPUNIT_TEST(configseveral);
  CPPUNIT_TEST(intparam);
  CPPUNIT_TEST(boolparam);
  CPPUNIT_TEST(listparam);
  CPPUNIT_TEST(boollistparam);
  CPPUNIT_TEST(intlistparam);
  CPPUNIT_TEST(stringlist);
  CPPUNIT_TEST(fpvalid);
  CPPUNIT_TEST_SUITE_END();


private:
  XXUSB::CConfigurableObject* m_pObject;
public:
  void setUp() {
    m_pObject = new XXUSB::CConfigurableObject("testObject");
  }
  void tearDown() {
    delete m_pObject;
  }
protected:
  void construct();
  void addstring();
  void config();
  void cget();
  void configseveral();
  void intparam();
  void boolparam();
  void listparam();
  void boollistparam();
  void intlistparam();
  void stringlist();
  void fpvalid();
};

CPPUNIT_TEST_SUITE_REGISTRATION(configtest);

void configtest::construct() {
  EQ(string("testObject"), m_pObject->getName());
}

void configtest::addstring() {
  m_pObject->addParameter("item", NULL, NULL, "initialValue");
  EQ(string("initialValue"), m_pObject->cget("item"));
}

void configtest::config() {
  m_pObject->addParameter("item", NULL, NULL);
  m_pObject->configure("item", "testString");
  EQ(string("testString"), m_pObject->cget("item"));

}

void configtest::cget() 
{
  m_pObject->addParameter("item1", NULL,NULL, "item1");
  m_pObject->addParameter("item2", NULL, NULL, "item2");

  string result = m_pObject->cget("item1");
  EQ(string("item1"), result);
  
  result = m_pObject->cget("item2");
  EQ(string("item2"), result);

  bool thrown = false;
  try {
    m_pObject->cget("No such item");
  }
  catch (string msg) {
    thrown = true;
  }
  ASSERT(thrown);
}

// really tsting the array returning version of cget.


void configtest::configseveral() {
  m_pObject->addParameter("one", NULL, NULL, "one");
  m_pObject->addParameter("two", NULL, NULL, "two");
  m_pObject->addParameter("three", NULL, NULL);
  m_pObject->configure("three", "three");

  XXUSB::CConfigurableObject::ConfigurationArray info = m_pObject->cget();

  // Each of these items should be findable, and have the right values.
  //
  pair<string,string> one("one", "one");
  pair<string,string> two("two", "two");
  pair<string,string> three("three", "three");

  EQMSG("one",true, find(info.begin(), info.end(), one) != info.end());
  EQMSG("two", true, find(info.begin(), info.end(), two) != info.end());
  EQMSG("three", true, find(info.begin(), info.end(), three) != info.end());

}
// Integer parameters are  parameters that have XXUSB::CConfigurableObject::isInteger
// as the validator. There are a couple of cases
// with and without range checking.
//
void configtest::intparam()
{
  // No range checking.
  m_pObject->addParameter("int", XXUSB::CConfigurableObject::isInteger, NULL, "0");
  XXUSB::CConfigurableObject::Limits range(XXUSB::CConfigurableObject::limit(0),
				    XXUSB::CConfigurableObject::limit(10));
  m_pObject->addParameter("zerototen", XXUSB::CConfigurableObject::isInteger, &range, "0");

  // Legal configuratiuon of int:

  bool thrown(false);
  try {
    m_pObject->configure("int", "1234");
  }
  catch(string msg) {
    thrown = true;
  }
  EQMSG("legal", false, thrown);

  // Illegal configuration of unconstrained int:

  try {
    m_pObject->configure("int", "justwrong");
  } 
  catch (string msg) {
    thrown = true;
  }
  EQMSG("Illegal", true, thrown);
  thrown = false;

  // Legal sets of constrained int:

  try {
    m_pObject->configure("zerototen", "0");
    m_pObject->configure("zerototen", "10");
    m_pObject->configure("zerototen", "5");
  }
  catch (string msg) {
    thrown = true;
  }
  EQMSG("Legally constrained", false, thrown);

  // Illegal value for constrained int:

  try {
    m_pObject->configure("zerototen", "11"); // it  doesn't go to eleven ;-)
  }
  catch (string msg) {
    thrown = true;
  }
  EQMSG("Illegal range", true, thrown);

}

// bool parameters have the isBool validator.  This takes noparameters but
// accepts as true:
//    true, yes, 1 on, enabled
// and as false:
//    false, no, 0, off, disabled
//
// A white box note:  bools are implemented as enums so 
// isEnum is therefore tested by this 
//
void configtest::boolparam()
{

  // Good values;


  vector<string> good;
  good.push_back("true");  // True values:
  good.push_back("yes");
  good.push_back("1");
  good.push_back("on");
  good.push_back("enabled");

  good.push_back("false"); // False values.
  good.push_back("no");
  good.push_back("0");
  good.push_back("off");
  good.push_back("disabled");

  m_pObject->addParameter("bool", XXUSB::CConfigurableObject::isBool, NULL, "1");

  // legal values:

  string errormessage("Did not throw");
  bool thrown(false);
  try {
    for (int i=0; i < good.size(); i++) {
      m_pObject->configure("bool", good[i]);
    }
  }
  catch (string msg) {
    thrown = true;
    errormessage = msg;
  }
  EQMSG(errormessage, false, thrown);

  try {
    m_pObject->configure("bool", "junk");
  }
  catch (string msg) {
    thrown = true;
  }
  ASSERT(thrown);
}



// 
// List type... we will not check the type checkers directly, but let
// the isBoolList, isIntList isStringList etc. check this indirectly.
// We will check
//   - Legal lists with no length restrictions
//   - Illegal lists with no lenght restrictions.
//   - Lists with length restrictions within the allowed lengths.
//   - Lists that with length restrictions that are not in the allowed lengths.
//
void configtest::listparam()
{
  XXUSB::CConfigurableObject::limit none;
  XXUSB::CConfigurableObject::ListSizeConstraint constraint  = {none, none};
  XXUSB::CConfigurableObject::TypeCheckInfo      notype(NULL, NULL);

  XXUSB::CConfigurableObject::isListParameter nochecks(constraint, notype);

  m_pObject->addParameter("list", XXUSB::CConfigurableObject::isList,
			  &nochecks, "");
  // Legal list

  bool thrown(false);
  string message;
  try {
    m_pObject->configure("list", "a b c d");
  }
  catch (string msg) {
    message = msg;
    thrown = true;
  }
  EQMSG(message, false, thrown);

  // Illegal list (unbalanced {}'s).

  try {
    m_pObject->configure("list", "a b {c d e");
  }
  catch (string msg) {
    message = msg;
    thrown  = true;
  }
  EQMSG(message, true, thrown);
  thrown = false;

  // Now a param that has at least 1 and at most 5 elements

  XXUSB::CConfigurableObject::limit low(1);
  XXUSB::CConfigurableObject::limit high(5);
  XXUSB::CConfigurableObject::ListSizeConstraint constrained = {low, high};
  XXUSB::CConfigurableObject::isListParameter sizeChecked(constrained, notype);

  m_pObject->addParameter("constrained", XXUSB::CConfigurableObject::isList,
			  &sizeChecked, "two");

  try {
    m_pObject->configure("constrained", "one");
    m_pObject->configure("constrained", "1 2 3 4 5");
  }
  catch (string msg) {
    message = msg;
    thrown = true;
  }
  EQMSG(message, false, thrown);
  
  // Bad length:

  try {
    m_pObject->configure("constrained", "");
  }
  catch (string msg) {
    message = msg;
    thrown = true;
  }
  EQMSG(message, true, thrown);

  thrown = false;
  try {
    m_pObject->configure("constrained", "1 2 3 4 5 6");
  }
  catch (string msg) {
    message = msg;
    thrown = true;
  }
  EQMSG(message, true, thrown);
}
// Bool lists. just need to check for bad value detection as the 
// size stuff has already been done.

void configtest::boollistparam()
{
  XXUSB::CConfigurableObject::limit none;
  XXUSB::CConfigurableObject::ListSizeConstraint sizes={none, none};

  m_pObject->addParameter("boollist", XXUSB::CConfigurableObject::isBoolList,
			  &sizes, "");
  
  // Legal bools:

  bool thrown(false);
  string message;
  try {
    m_pObject->configure("boollist", "on off true false 0 1 enabled");
  } 
  catch (string msg) {
    message = msg;
    thrown  = true;
  }
  EQMSG(message, false, thrown);

  // Illegal bool... add some commas.. typical error:

  try {
    m_pObject->configure("boollist", "on, off, true, false, 0, 1, enabled");
  }
  catch (string msg) {
    thrown = true;
  }
  EQMSG("should have thrown", true, thrown);

}
// int lists - we already know that int range checking works so we'll just
// check that the checker gets called...using format validity.
//
void configtest::intlistparam()
{
  XXUSB::CConfigurableObject::limit none;
  XXUSB::CConfigurableObject::ListSizeConstraint s = {none, none};

  m_pObject->addParameter("intlist", XXUSB::CConfigurableObject::isIntList, &s, "");

  // legal list:

  string message;
  bool   thrown(false);

  try {
    m_pObject->configure("intlist", "1 2 3 4");
  }
  catch(string msg) {
    message = msg;
    thrown  = true;
  }
  EQMSG(message, false, thrown);

  // Illegal list

  try {
    m_pObject->configure("intlist", "1 b c");
  }
  catch (string msg) {
    thrown = true;
  }
  EQMSG("Should have thrown", true, thrown);
}
// String lists:

void configtest::stringlist()
{
  XXUSB::CConfigurableObject::limit none;
  XXUSB::CConfigurableObject::ListSizeConstraint s = {none, none};
  m_pObject->addParameter("stringlist", XXUSB::CConfigurableObject::isStringList, &s);
  

  bool thrown(false);
  string message;
  try {
    m_pObject->configure("stringlist", "a b c d e f g");
  }
  catch (string msg) {
    message =msg;
    thrown = true;
  }
  EQMSG(message, false, thrown);
}

// Check the floating point validity checker given valid values:

void configtest::fpvalid()
{

  // limit contituents.

  XXUSB::CConfigurableObject::flimit low(1.0), high(2.0);
  XXUSB::CConfigurableObject::flimit nolow, nohigh;

  // none       --  Fully open limits.
  // lowOpen    --  Limits open on the low end.
  // highOpen   --  Limits open onthe high end.
  // fullClosed --  Fully closed limit set.

  XXUSB::CConfigurableObject::FloatingLimits none(nolow, nohigh);
  XXUSB::CConfigurableObject::FloatingLimits lowOpen(nolow, high);
  XXUSB::CConfigurableObject::FloatingLimits highOpen(low, nohigh);
  XXUSB::CConfigurableObject::FloatingLimits fullClosed(low, high);

  // add parameters to check against, one for each of the limits above,
  // and one with no checker provided.

  m_pObject->addParameter("norange", XXUSB::CConfigurableObject::isFloat, (void*)NULL);
  m_pObject->addParameter("none", XXUSB::CConfigurableObject::isFloat, &none);
  m_pObject->addParameter("lowopen", XXUSB::CConfigurableObject::isFloat, &lowOpen);
  m_pObject->addParameter("highopen", XXUSB::CConfigurableObject::isFloat, &highOpen);
  m_pObject->addParameter("closed",   XXUSB::CConfigurableObject::isFloat, &fullClosed);
  
  
  
}
